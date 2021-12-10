/*
 * Copyright (c) 2019 Sergi Granell (xerpi), Joel16
 */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pspkernel.h>
#include <pspiofilemgr.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <sys/time.h>
#include <psprtc.h>
#include <sys/syslimits.h>

#include "fs.h"
#include "ftppsp.h"
#include "kernel_functions.h"
#include "mutex.h"

#define UNUSED(x) (void)(x)

#define FTP_PORT 1337
#define DEFAULT_FILE_BUF_SIZE (1 * 1024 * 1024)

#define FTP_DEFAULT_PATH   "/"

#define MAX_DEVICES 7
#define MAX_CUSTOM_COMMANDS 16

typedef struct {
    const char *cmd = nullptr;
    cmd_dispatch_func func;
} cmd_dispatch_entry;

static struct {
    char name[512] = {0};
    int valid = -1;
} device_list[MAX_DEVICES];

static struct {
    const char *cmd = nullptr;
    cmd_dispatch_func func;
    int valid = -1;
} custom_command_dispatchers[MAX_CUSTOM_COMMANDS];

static void *net_memory = nullptr;
static int ftp_initialized = 0;
static unsigned int file_buf_size = DEFAULT_FILE_BUF_SIZE;
static struct in_addr psp_addr;
static SceUID server_thid = 0;
static int server_sockfd = 0;
static int number_clients = 0;
static ftppsp_client_info_t *client_list = nullptr;
static SceUID client_list_mtx = 0;

static void (*info_log_cb)(const char *) = nullptr;
static void (*debug_log_cb)(const char *) = nullptr;

static void log_func(ftppsp_log_cb_t log_cb, const char *s, ...) {
    if (log_cb) {
        char buf[256] = {0};
        va_list argptr;
        va_start(argptr, s);
        vsnprintf(buf, sizeof(buf), s, argptr);
        va_end(argptr);
        log_cb(buf);
    }
}

#define INFO(...) log_func(info_log_cb, __VA_ARGS__)
#define DEBUG(...) log_func(debug_log_cb, __VA_ARGS__)

static int client_send_ctrl_msg(ftppsp_client_info_t *cl, const std::string &str) {
    std::printf(str.c_str());
    return sceNetInetSend(cl->ctrl_sockfd, str.c_str(), str.length(), 0);
}

// Missing prototypes in the PSPSDK
int sceNetInetGetsockname(int, struct sockaddr *, socklen_t *);
const char* sceNetInetInetNtop(int, const void *, char *, socklen_t);
int sceNetInetInetPton(int, const char *, void *);

static __inline__ unsigned int sceAllegrexWsbw(unsigned int x) {
    return (((x & 0xFF)<<24) | ((x & 0xFF00)<<8) | ((x>>8) & 0xFF00) | ((x>>24) & 0xFF));
}

static __inline__ unsigned int sceAllegrexWsbh(unsigned int x) {
    return (((x<<8) & 0xFF00FF00) | ((x>>8) & 0x00FF00FF));
}

static inline u32 SCE_HTONL(u32 hostlong) {
    return sceAllegrexWsbw(hostlong);
}

static inline u16 SCE_HTONS(u16 hostshort) {
    return sceAllegrexWsbh(hostshort);
}

static inline void client_send_data_msg(ftppsp_client_info_t *client, const std::string &str) {
    if (client->data_con_type == FTP_DATA_CONNECTION_ACTIVE)
        sceNetInetSend(client->data_sockfd, str.c_str(), str.length(), 0);
    else
        sceNetInetSend(client->pasv_sockfd, str.c_str(), str.length(), 0);
}

static inline int client_recv_data_raw(ftppsp_client_info_t *client, void *buf, unsigned int len) {
    if (client->data_con_type == FTP_DATA_CONNECTION_ACTIVE)
        return sceNetInetRecv(client->data_sockfd, buf, len, 0);
    else
        return sceNetInetRecv(client->pasv_sockfd, buf, len, 0);
}

static inline void client_send_data_raw(ftppsp_client_info_t *client, const void *buf, unsigned int len) {
    if (client->data_con_type == FTP_DATA_CONNECTION_ACTIVE)
        sceNetInetSend(client->data_sockfd, buf, len, 0);
    else
        sceNetInetSend(client->pasv_sockfd, buf, len, 0);
}

static inline const char *get_psp_path(const char *path) {
    if (std::strlen(path) > 1) /* /cache0:/foo/bar -> cache0:/foo/bar */
        return &path[1];
    else
        return nullptr;
}

static void cmd_NOOP_func(ftppsp_client_info_t *client) {
    client_send_ctrl_msg(client, "200 No operation ;)\r\n");
}

static void cmd_USER_func(ftppsp_client_info_t *client) {
    client_send_ctrl_msg(client, "331 Username OK, need password b0ss.\r\n");
}

static void cmd_PASS_func(ftppsp_client_info_t *client) {
    client_send_ctrl_msg(client, "230 User logged in!\r\n");
}

static void cmd_QUIT_func(ftppsp_client_info_t *client) {
    client_send_ctrl_msg(client, "221 Goodbye senpai :'(\r\n");
}

static void cmd_SYST_func(ftppsp_client_info_t *client) {
    client_send_ctrl_msg(client, "215 UNIX Type: L8\r\n");
}

static void cmd_PASV_func(ftppsp_client_info_t *client) {
    int ret;
    UNUSED(ret);
    
    char cmd[512] = {0};
    unsigned int namelen;
    struct sockaddr_in picked;
    
    /* Create the data socket */
    client->data_sockfd = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
    
    DEBUG("PASV data socket fd: %d\n", client->data_sockfd);
    
    /* Fill the data socket address */
    client->data_sockaddr.sin_family = AF_INET;
    client->data_sockaddr.sin_addr.s_addr = SCE_HTONL(INADDR_ANY);
    /* Let the PSP choose a port */
    client->data_sockaddr.sin_port = SCE_HTONS(0);
    
    /* Bind the data socket address to the data socket */
    ret = sceNetInetBind(client->data_sockfd, reinterpret_cast<struct sockaddr *>(&client->data_sockaddr), sizeof(client->data_sockaddr));
    DEBUG("sceNetInetBind(): 0x%08X\n", ret);
    
    /* Start listening */
    ret = sceNetInetListen(client->data_sockfd, 128);
    DEBUG("sceNetInetListen(): 0x%08X\n", ret);
    
    /* Get the port that the PSP has chosen */
    namelen = sizeof(picked);
    sceNetInetGetsockname(client->data_sockfd, reinterpret_cast<struct sockaddr *>(&picked), &namelen);
    
    DEBUG("PASV mode port: 0x%04X\n", picked.sin_port);
    
    /* Build the command */
    std::sprintf(cmd, "227 Entering Passive Mode (%hhu,%hhu,%hhu,%hhu,%hhu,%hhu)\r\n",
        (psp_addr.s_addr >> 0) & 0xFF,
        (psp_addr.s_addr >> 8) & 0xFF,
        (psp_addr.s_addr >> 16) & 0xFF,
        (psp_addr.s_addr >> 24) & 0xFF,
        (picked.sin_port >> 0) & 0xFF,
        (picked.sin_port >> 8) & 0xFF);
        
    client_send_ctrl_msg(client, cmd);
    
    /* Set the data connection type to passive! */
    client->data_con_type = FTP_DATA_CONNECTION_PASSIVE;
}

static void cmd_PORT_func(ftppsp_client_info_t *client) {
    unsigned int data_ip[4] = {0};
    unsigned int porthi, portlo;
    unsigned short data_port;
    char ip_str[16] = {0};
    struct in_addr data_addr;
    
    /* Using ints because of newlibc's u8 std::sscanf bug */
    std::sscanf(client->recv_cmd_args, "%d,%d,%d,%d,%d,%d", &data_ip[0], &data_ip[1], &data_ip[2], &data_ip[3], &porthi, &portlo);
    
    data_port = portlo + porthi*256;
    
    /* Convert to an X.X.X.X IP string */
    std::sprintf(ip_str, "%d.%d.%d.%d", data_ip[0], data_ip[1], data_ip[2], data_ip[3]);
    
    /* Convert the IP to a SceNetInAddr */
    sceNetInetInetPton(AF_INET, ip_str, &data_addr);
    
    DEBUG("PORT connection to client's IP: %s Port: %d\n", ip_str, data_port);
    
    /* Create data mode socket */
    client->data_sockfd = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
    
    DEBUG("Client %i data socket fd: %d\n", client->num, client->data_sockfd);
    
    /* Prepare socket address for the data connection */
    client->data_sockaddr.sin_family = AF_INET;
    client->data_sockaddr.sin_addr = data_addr;
    client->data_sockaddr.sin_port = SCE_HTONS(data_port);
    
    /* Set the data connection type to active! */
    client->data_con_type = FTP_DATA_CONNECTION_ACTIVE;
    
    client_send_ctrl_msg(client, "200 PORT command successful!\r\n");
}

static void client_open_data_connection(ftppsp_client_info_t *client) {
    int ret;
    UNUSED(ret);
    
    socklen_t addrlen;
    if (client->data_con_type == FTP_DATA_CONNECTION_ACTIVE) {
        /* Connect to the client using the data socket */
        ret = sceNetInetConnect(client->data_sockfd, reinterpret_cast<struct sockaddr *>(&client->data_sockaddr), sizeof(client->data_sockaddr));
        
        DEBUG("sceNetInetConnect(): 0x%08X\n", ret);
    }
    else {
        /* Listen to the client using the data socket */
        addrlen = sizeof(client->pasv_sockaddr);
        client->pasv_sockfd = sceNetInetAccept(client->data_sockfd, reinterpret_cast<struct sockaddr *>(&client->pasv_sockaddr), &addrlen);
        DEBUG("PASV client fd: 0x%08X\n", client->pasv_sockfd);
    }
}

static void client_close_data_connection(ftppsp_client_info_t *client) {
    sceNetInetClose(client->data_sockfd);
    /* In passive mode we have to close the client pasv socket too */
    if (client->data_con_type == FTP_DATA_CONNECTION_PASSIVE)
        sceNetInetClose(client->pasv_sockfd);
        
    client->data_con_type = FTP_DATA_CONNECTION_NONE;
}

static int gen_list_format(char *out, int n, int dir, const SceIoStat *stat, const char *filename) {
    static const char num_to_month[][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    
    char yt[13] = {0};
    pspTime cdt;
    sceRtcGetCurrentClockLocalTime(&cdt);
    
    if (cdt.year == stat->st_mtime.year)
        snprintf(yt, 12, "%02d:%02d", stat->st_mtime.hour, stat->st_mtime.minute);
    else
        snprintf(yt, 11, "%04d", stat->st_mtime.year);
        
    return snprintf(out, n, "%c%s 1 psp psp %llu %s %-2d %s %s\r\n", dir ? 'd' : '-', dir ? "rwxr-xr-x" : "rw-r--r--",
        stat->st_size, num_to_month[stat->st_mtime.month<=0?0:(stat->st_mtime.month-1)%12], stat->st_mtime.day,
        yt, filename);
}

static void send_LIST(ftppsp_client_info_t *client, const char *path) {
    char buffer[512] = {0};
    SceUID dir;
    SceIoDirent dirent;
    SceIoStat stat;
    char *devname;
    int send_devices = 0;
    
    /* "/" path is a special case, if we are here we have
     * to send the list of devices (aka mountpoints). */
    if (std::strcmp(path, "/") == 0)
        send_devices = 1;
        
    if (!send_devices) {
#ifdef FS_DEBUG
        dir = sceIoDopen(get_psp_path(path));
#else
        dir = pspIoOpenDir(get_psp_path(path));
#endif
        if (dir < 0) {
            client_send_ctrl_msg(client, "550 Invalid directory.\r\n");
            return;
        }
    }
    
    client_send_ctrl_msg(client, "150 Opening ASCII mode data transfer for LIST.\r\n");
    
    client_open_data_connection(client);
    
    if (send_devices) {
        for (int i = 0; i < MAX_DEVICES; i++) {
            if (device_list[i].valid) {
                devname = device_list[i].name;
                gen_list_format(buffer, sizeof(buffer), 1, &stat, devname);
                client_send_data_msg(client, buffer);
            }
        }
    }
    else {
        std::memset(&dirent, 0, sizeof(dirent));
        
#ifdef FS_DEBUG
        while (sceIoDread(dir, &dirent) > 0) {
#else
        while (pspIoReadDir(dir, &dirent) > 0) {
#endif
            gen_list_format(buffer, sizeof(buffer), FIO_S_ISDIR(dirent.d_stat.st_mode), &dirent.d_stat, dirent.d_name);
            client_send_data_msg(client, buffer);
            std::memset(&dirent, 0, sizeof(dirent));
            std::memset(buffer, 0, sizeof(buffer));
        }
        
#ifdef FS_DEBUG
        sceIoDclose(dir);
#else
        pspIoCloseDir(dir);
#endif
    }
    
    DEBUG("Done sending LIST\n");
    
    client_close_data_connection(client);
    client_send_ctrl_msg(client, "226 Transfer complete.\r\n");
}

static void cmd_LIST_func(ftppsp_client_info_t *client) {
    char list_path[512] = {0};
    int list_cur_path = 1;
    
    int n = std::sscanf(client->recv_cmd_args, "%[^\r\n\t]", list_path);
    
    if (n > 0 && FS::FileExists(list_path))
        list_cur_path = 0;
        
    if (list_cur_path)
        send_LIST(client, client->cur_path);
    else
        send_LIST(client, list_path);
}

static void cmd_PWD_func(ftppsp_client_info_t *client) {
    char msg[1059] = {0};
    snprintf(msg, 1058, "257 \"%s\" is the current directory.\r\n", client->cur_path);
    client_send_ctrl_msg(client, msg);
}

static int path_is_at_root(const char *path) {
    return std::strrchr(path, '/') == (path + std::strlen(path) - 1);
}

static void dir_up(char *path) {
    char *pch;
    size_t len_in = std::strlen(path);
    
    if (len_in == 1) {
        std::strcpy(path, "/");
        return;
    }
    
    if (path_is_at_root(path)) /* Case root of the device (/foo0:/) */
        std::strcpy(path, "/");
    else {
        pch = std::strrchr(path, '/');
        size_t s = len_in - (pch - path);
        std::memset(pch, '\0', s);
        
        /* If the path is like: /foo: add slash */
        if (std::strrchr(path, '/') == path)
            std::strcat(path, "/");
    }
}

static void cmd_CWD_func(ftppsp_client_info_t *client) {
    char cmd_path[512] = {0};
    char tmp_path[1537] = {0};
    SceUID pd;
    int n = std::sscanf(client->recv_cmd_args, "%[^\r\n\t]", cmd_path);
    
    if (n < 1)
        client_send_ctrl_msg(client, "500 Syntax error, command unrecognized.\r\n");
    else {
        if (std::strcmp(cmd_path, "/") == 0)
            std::strcpy(client->cur_path, cmd_path);
        else  if (std::strcmp(cmd_path, "..") == 0)
            dir_up(client->cur_path);
        else {
            if (cmd_path[0] == '/') /* Full path */
                std::strcpy(tmp_path, cmd_path);
            else { /* Change dir relative to current dir */
                /* If we are at the root of the device, don't add
                 * an slash to add new path */
                if (path_is_at_root(client->cur_path))
                    snprintf(tmp_path, 1535, "%s%s", client->cur_path, cmd_path);
                else
                    snprintf(tmp_path, 1536, "%s/%s", client->cur_path, cmd_path);
            }
            
            /* If the path is like: /foo: add an slash */
            if (std::strrchr(tmp_path, '/') == tmp_path)
                std::strcat(tmp_path, "/");
                
            /* If the path is not "/", check if it exists */
            if (std::strcmp(tmp_path, "/") != 0) {
                /* Check if the path exists */
#ifdef FS_DEBUG
                pd = sceIoDopen(get_psp_path(tmp_path));
#else
                pd = pspIoOpenDir(get_psp_path(tmp_path));
#endif

                if (pd < 0) {
                    client_send_ctrl_msg(client, "550 Invalid directory.\r\n");
                    return;
                }

#ifdef FS_DEBUG
                sceIoDclose(pd);
#else
                pspIoCloseDir(pd);
#endif
            }
            std::strcpy(client->cur_path, tmp_path);
        }
        client_send_ctrl_msg(client, "250 Requested file action okay, completed.\r\n");
    }
}

static void cmd_TYPE_func(ftppsp_client_info_t *client) {
    char data_type;
    char format_control[8] = {0};
    int n_args = std::sscanf(client->recv_cmd_args, "%c %s", &data_type, format_control);
    
    if (n_args > 0) {
        switch(data_type) {
            case 'A':
            case 'I':
                client_send_ctrl_msg(client, "200 Okay\r\n");
                break;
            case 'E':
            case 'L':
            default:
                client_send_ctrl_msg(client, "504 Error: bad parameters?\r\n");
                break;
        }
    }
    else
        client_send_ctrl_msg(client, "504 Error: bad parameters?\r\n");
}

static void cmd_CDUP_func(ftppsp_client_info_t *client) {
    dir_up(client->cur_path);
    client_send_ctrl_msg(client, "200 Command okay.\r\n");
}

static void send_file(ftppsp_client_info_t *client, const char *path) {
    unsigned char *buffer;
    SceUID fd;
    unsigned int bytes_read;
    
    DEBUG("Opening: %s\n", path);

#ifdef FS_DEBUG
    if ((fd = sceIoOpen(path, PSP_O_RDONLY, 0777)) >= 0) {
        sceIoLseek32(fd, client->restore_point, PSP_SEEK_SET);
#else
    if ((fd = pspIoOpenFile(path, PSP_O_RDONLY, 0777)) >= 0) {
        pspIoLseek32(fd, client->restore_point, PSP_SEEK_SET);
#endif
        
        buffer = new unsigned char[file_buf_size];
        if (buffer == nullptr) {
            client_send_ctrl_msg(client, "550 Could not allocate memory.\r\n");
            return;
        }
        
        client_open_data_connection(client);
        client_send_ctrl_msg(client, "150 Opening Image mode data transfer.\r\n");
        
#ifdef FS_DEBUG
        while ((bytes_read = sceIoRead(fd, buffer, file_buf_size)) > 0)
#else
        while ((bytes_read = pspIoReadFile(fd, buffer, file_buf_size)) > 0)
#endif
            client_send_data_raw(client, buffer, bytes_read);

#ifdef FS_DEBUG 
        sceIoClose(fd);
#else
        pspIoCloseFile(fd);
#endif
        delete[] buffer;
        client->restore_point = 0;
        client_send_ctrl_msg(client, "226 Transfer completed.\r\n");
        client_close_data_connection(client);
    }
    else
        client_send_ctrl_msg(client, "550 File not found.\r\n");
}

/* This function generates an FTP full-path with the input path (relative or absolute)
 * from RETR, STOR, DELE, RMD, MKD, RNFR and RNTO commands */
static void gen_ftp_fullpath(ftppsp_client_info_t *client, char *path, size_t path_size) {
    char cmd_path[512] = {0};
    std::sscanf(client->recv_cmd_args, "%[^\r\n\t]", cmd_path);
    
    if (cmd_path[0] == '/')
        strncpy(path, cmd_path, path_size); /* Full path */
    else {
        if (std::strlen(cmd_path) >= 5 && cmd_path[3] == ':' && cmd_path[4] == '/')
            snprintf(path, 513, "/%s", cmd_path); /* Case "ux0:/foo */
        else {
            /* The file is relative to current dir, so
             * append the file to the current path */
             snprintf(path, 1536, "%s/%s", client->cur_path, cmd_path);
        }
    }
}

static void cmd_RETR_func(ftppsp_client_info_t *client) {
    char dest_path[512] = {0};
    gen_ftp_fullpath(client, dest_path, sizeof(dest_path));
    send_file(client, get_psp_path(dest_path));
}

static void receive_file(ftppsp_client_info_t *client, const char *path) {
    unsigned char *buffer;
    SceUID fd;
    int bytes_recv;
    
    DEBUG("Opening: %s\n", path);
    
    int mode = PSP_O_CREAT | PSP_O_RDWR;
    /* if we resume broken - append missing part
    * else - overwrite file */
    if (client->restore_point)
        mode = mode | PSP_O_APPEND;
    else
        mode = mode | PSP_O_TRUNC;

#ifdef FS_DEBUG
    if ((fd = sceIoOpen(path, mode, 0777)) >= 0) {
#else
    if ((fd = pspIoOpenFile(path, mode, 0777)) >= 0) {
#endif
        buffer = new unsigned char[file_buf_size];
        if (buffer == nullptr) {
            client_send_ctrl_msg(client, "550 Could not allocate memory.\r\n");
            return;
        }
        
        client_open_data_connection(client);
        client_send_ctrl_msg(client, "150 Opening Image mode data transfer.\r\n");
        
        while ((bytes_recv = client_recv_data_raw(client, buffer, file_buf_size)) > 0)
#ifdef FS_DEBUG
            sceIoWrite(fd, buffer, bytes_recv);
#else
            pspIoWriteFile(fd, buffer, bytes_recv);
#endif

#ifdef FS_DEBUG
        sceIoClose(fd);
#else
        pspIoCloseFile(fd);
#endif
        delete[] buffer;
        client->restore_point = 0;
        if (bytes_recv == 0)
            client_send_ctrl_msg(client, "226 Transfer completed.\r\n");
        else {
#ifdef FS_DEBUG
            sceIoRemove(path);
#else
            pspIoRemoveFile(path);
#endif
            client_send_ctrl_msg(client, "426 Connection closed; transfer aborted.\r\n");
        }
        client_close_data_connection(client);   
    }
    else
        client_send_ctrl_msg(client, "550 File not found.\r\n");
}

static void cmd_STOR_func(ftppsp_client_info_t *client) {
    char dest_path[512] = {0};
    gen_ftp_fullpath(client, dest_path, sizeof(dest_path));
    receive_file(client, get_psp_path(dest_path));
}

static void delete_file(ftppsp_client_info_t *client, const char *path) {
    DEBUG("Deleting: %s\n", path);
    
#ifdef FS_DEBUG
    if (sceIoRemove(path) >= 0)
#else
    if (pspIoRemoveFile(path) >= 0)
#endif
        client_send_ctrl_msg(client, "226 File deleted.\r\n");
    else
        client_send_ctrl_msg(client, "550 Could not delete the file.\r\n");
}

static void cmd_DELE_func(ftppsp_client_info_t *client) {
    char dest_path[512] = {0};
    gen_ftp_fullpath(client, dest_path, sizeof(dest_path));
    delete_file(client, get_psp_path(dest_path));
}

static void delete_dir(ftppsp_client_info_t *client, const char *path) {
    unsigned int ret;
    DEBUG("Deleting: %s\n", path);
#ifdef FS_DEBUG
    ret = sceIoRmdir(path);
#else
    ret = pspIoRemoveDir(path);
#endif

    if (ret >= 0)
        client_send_ctrl_msg(client, "226 Directory deleted.\r\n");
    else if (ret == 0x8001005A) /* DIRECTORY_IS_NOT_EMPTY */
        client_send_ctrl_msg(client, "550 Directory is not empty.\r\n");
    else
        client_send_ctrl_msg(client, "550 Could not delete the directory.\r\n");
}

static void cmd_RMD_func(ftppsp_client_info_t *client) {
    char dest_path[512] = {0};
    gen_ftp_fullpath(client, dest_path, sizeof(dest_path));
    delete_dir(client, get_psp_path(dest_path));
}

static void create_dir(ftppsp_client_info_t *client, const char *path) {
    DEBUG("Creating: %s\n", path);
    
#ifdef FS_DEBUG
    if (sceIoMkdir(path, 0777) >= 0)
#else
    if (pspIoMakeDir(path, 0777) >= 0)
#endif
        client_send_ctrl_msg(client, "226 Directory created.\r\n");
    else
        client_send_ctrl_msg(client, "550 Could not create the directory.\r\n");
}

static void cmd_MKD_func(ftppsp_client_info_t *client) {
    char dest_path[512] = {0};
    gen_ftp_fullpath(client, dest_path, sizeof(dest_path));
    create_dir(client, get_psp_path(dest_path));
}

static void cmd_RNFR_func(ftppsp_client_info_t *client) {
    char path_src[512] = {0};
    const char *psp_path_src;
    /* Get the origin filename */
    gen_ftp_fullpath(client, path_src, sizeof(path_src));
    psp_path_src = get_psp_path(path_src);
    
    /* Check if the file exists */
    if (!FS::FileExists(psp_path_src)) {
        client_send_ctrl_msg(client, "550 The file doesn't exist.\r\n");
        return;
    }
    
    /* The file to be renamed is the received path */
    std::strcpy(client->rename_path, psp_path_src);
    client_send_ctrl_msg(client, "350 I need the destination name b0ss.\r\n");
}

static void cmd_RNTO_func(ftppsp_client_info_t *client) {
    char path_dst[512] = {0};
    const char *psp_path_dst;
    
    /* Get the destination filename */
    gen_ftp_fullpath(client, path_dst,sizeof(path_dst));
    psp_path_dst = get_psp_path(path_dst);
    
    DEBUG("Renaming: %s to %s\n", client->rename_path, psp_path_dst);
    
#ifdef FS_DEBUG
    if (sceIoRename(client->rename_path, psp_path_dst) < 0)
#else
    if (pspIoRename(client->rename_path, psp_path_dst) < 0)
#endif
        client_send_ctrl_msg(client, "550 Error renaming the file.\r\n");
        
    client_send_ctrl_msg(client, "226 Rename completed.\r\n");
}

static void cmd_SIZE_func(ftppsp_client_info_t *client) {
    SceIoStat stat;
    char path[512] = {0};
    char cmd[64] = {0};
    /* Get the filename to retrieve its size */
    gen_ftp_fullpath(client, path, sizeof(path));
    
    /* Check if the file exists */
#ifdef FS_DEBUG
    if (sceIoGetstat(get_psp_path(path), &stat) < 0) {
#else
    if (pspIoGetstat(get_psp_path(path), &stat) < 0) {
#endif
        client_send_ctrl_msg(client, "550 The file doesn't exist.\r\n");
        return;
    }
    
    /* Send the size of the file */
    std::sprintf(cmd, "213 %lld\r\n", stat.st_size);
    client_send_ctrl_msg(client, cmd);
}

static void cmd_REST_func(ftppsp_client_info_t *client) {
    char cmd[64] = {0};
    std::sscanf(client->recv_buffer, "%*[^ ] %d", &client->restore_point);
    std::sprintf(cmd, "350 Resuming at %d\r\n", client->restore_point);
    client_send_ctrl_msg(client, cmd);
}

static void cmd_FEAT_func(ftppsp_client_info_t *client) {
    /*So client would know that we support resume */
    client_send_ctrl_msg(client, "211-extensions\r\n");
    client_send_ctrl_msg(client, "REST STREAM\r\n");
    client_send_ctrl_msg(client, "211 end\r\n");
}

static void cmd_APPE_func(ftppsp_client_info_t *client) {
    /* set restore point to not 0
    restore point numeric value only matters if we RETR file from psp.
    If we STOR or APPE, it is only used to indicate that we want to resume
    a broken transfer */
    client->restore_point = -1;
    char dest_path[512] = {0};
    gen_ftp_fullpath(client, dest_path, sizeof(dest_path));
    receive_file(client, get_psp_path(dest_path));
}

#define add_entry(name) {#name, cmd_##name##_func}
static const cmd_dispatch_entry cmd_dispatch_table[] = {
    add_entry(NOOP),
    add_entry(USER),
    add_entry(PASS),
    add_entry(QUIT),
    add_entry(SYST),
    add_entry(PASV),
    add_entry(PORT),
    add_entry(LIST),
    add_entry(PWD),
    add_entry(CWD),
    add_entry(TYPE),
    add_entry(CDUP),
    add_entry(RETR),
    add_entry(STOR),
    add_entry(DELE),
    add_entry(RMD),
    add_entry(MKD),
    add_entry(RNFR),
    add_entry(RNTO),
    add_entry(SIZE),
    add_entry(REST),
    add_entry(FEAT),
    add_entry(APPE),
    {nullptr, nullptr}
};

static cmd_dispatch_func get_dispatch_func(const char *cmd) {
    for(int i = 0; cmd_dispatch_table[i].cmd && cmd_dispatch_table[i].func; i++) {
        if (std::strcmp(cmd, cmd_dispatch_table[i].cmd) == 0)
            return cmd_dispatch_table[i].func;
    }
    
    // Check for custom commands
    for(int i = 0; i < MAX_CUSTOM_COMMANDS; i++) {
        if (custom_command_dispatchers[i].valid) {
            if (std::strcmp(cmd, custom_command_dispatchers[i].cmd) == 0)
                return custom_command_dispatchers[i].func;
        }
    }
    
    return nullptr;
}

static void client_list_add(ftppsp_client_info_t *client) {
    /* Add the client at the front of the client list */
    sceKernelLockMutex(client_list_mtx, 1, nullptr);
    
    if (client_list == nullptr) { /* List is empty */
        client_list = client;
        client->prev = nullptr;
        client->next = nullptr;
    } else {
        client->next = client_list;
        client_list->prev = client;
        client->prev = nullptr;
        client_list = client;
    }
    client->restore_point = 0;
    number_clients++;
    
    sceKernelUnlockMutex(client_list_mtx, 1);
}

static void client_list_delete(ftppsp_client_info_t *client) {
    /* Remove the client from the client list */
    sceKernelLockMutex(client_list_mtx, 1, nullptr);
    
    if (client->prev)
        client->prev->next = client->next;
    if (client->next)
        client->next->prev = client->prev;
    if (client == client_list)
        client_list = client->next;
        
    number_clients--;
    
    sceKernelUnlockMutex(client_list_mtx, 1);
}

static void client_list_thread_end(void) {
    ftppsp_client_info_t *it, *next;
    SceUID client_thid;
    
    sceKernelLockMutex(client_list_mtx, 1, nullptr);
    
    it = client_list;
    
    /* Iterate over the client list and close their sockets */
    while (it) {
        next = it->next;
        client_thid = it->thid;
        
        /* Abort the client's control socket, only abort
        * receiving data so we can still send control messages */
        sceNetInetShutdown(it->ctrl_sockfd, SHUT_RDWR);
        
        // If there's an open data connection, abort it
        if (it->data_con_type != FTP_DATA_CONNECTION_NONE) {
            sceNetInetShutdown(it->data_sockfd, SHUT_RDWR);
            
            if (it->data_con_type == FTP_DATA_CONNECTION_PASSIVE)
                sceNetInetShutdown(it->pasv_sockfd, SHUT_RDWR);
        }
        
        /* Wait until the client threads ends */
        SceUInt timeout = 1000000;
        sceKernelWaitThreadEnd(client_thid, &timeout);
        
        it = next;
    }
    
    sceKernelUnlockMutex(client_list_mtx, 1);
}

static int client_thread(SceSize args, void *argp) {
    char cmd[16] = {0};
    cmd_dispatch_func dispatch_func;
    ftppsp_client_info_t *client = *reinterpret_cast<ftppsp_client_info_t **>(argp);
    
    DEBUG("Client thread %i started!\n", client->num);
    
    client_send_ctrl_msg(client, "220 FTPPSP Server ready.\r\n");
    
    while (true) {
        std::memset(client->recv_buffer, 0, sizeof(client->recv_buffer));
        
        client->n_recv = sceNetInetRecv(client->ctrl_sockfd, client->recv_buffer, sizeof(client->recv_buffer), 0);
        if (client->n_recv > 0) {
            DEBUG("Received %i bytes from client number %i:\n", client->n_recv, client->num);
            
            INFO("\t%i> %s", client->num, client->recv_buffer);
            
            /* The command is the first chars until the first space */
            std::sscanf(client->recv_buffer, "%s", cmd);
            
            client->recv_cmd_args = strchr(client->recv_buffer, ' ');
            if (client->recv_cmd_args)
                client->recv_cmd_args++; /* Skip the space */
            else
                client->recv_cmd_args = client->recv_buffer;
                
            /* Wait 1 ms before sending any data */
            sceKernelDelayThread(1*1000);
            
            if ((dispatch_func = get_dispatch_func(cmd)))
                dispatch_func(client);
            else
                client_send_ctrl_msg(client, "502 Sorry, command not implemented. :(\r\n");
        }
        else if (client->n_recv == 0) {
            /* Value 0 means connection closed by the remote peer */
            INFO("Connection closed by the client %i.\n", client->num);
            /* Delete itself from the client list */
            client_list_delete(client);
            break;
        } /*else if (client->n_recv == EINTR) {
            // Socket aborted (ftppsp_fini() called)
            INFO("Client %i socket aborted.\n", client->num);
            break;
        }*/ else {
            /* Other errors */
            INFO("Client %i socket error: 0x%08X\n", client->num, client->n_recv);
            client_list_delete(client);
            break;
        }
    }
    
    /* Close the client's socket */
    sceNetInetClose(client->ctrl_sockfd);
    
    /* If there's an open data connection, close it */
    if (client->data_con_type != FTP_DATA_CONNECTION_NONE) {
        sceNetInetClose(client->data_sockfd);
        
        if (client->data_con_type == FTP_DATA_CONNECTION_PASSIVE)
            sceNetInetClose(client->pasv_sockfd);
    }
    
    DEBUG("Client thread %i exiting!\n", client->num);
    
    delete[] client;
    
    sceKernelExitDeleteThread(0);
    return 0;
}

static int server_thread(SceSize args, void *argp) {
    int ret;
    UNUSED(ret);
    
    struct sockaddr_in serveraddr;
    
    DEBUG("Server thread started!\n");
    
    /* Create server socket */
    server_sockfd = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
    
    DEBUG("Server socket fd: %d\n", server_sockfd);
    
    /* Fill the server's address */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = SCE_HTONL(INADDR_ANY);
    serveraddr.sin_port = SCE_HTONS(FTP_PORT);
    
    /* Bind the server's address to the socket */
    ret = sceNetInetBind(server_sockfd, reinterpret_cast<struct sockaddr *>(&serveraddr), sizeof(serveraddr));
    DEBUG("sceNetInetBind(): 0x%08X\n", ret);
    
    /* Start listening */
    ret = sceNetInetListen(server_sockfd, 128);
    DEBUG("sceNetInetListen(): 0x%08X\n", ret);
    
    while (true) {
        /* Accept clients */
        struct sockaddr_in clientaddr;
        int client_sockfd;
        socklen_t addrlen = sizeof(clientaddr);
        
        DEBUG("Waiting for incoming connections...\n");
        
        client_sockfd = sceNetInetAccept(server_sockfd, reinterpret_cast<struct sockaddr *>(&clientaddr), &addrlen);
        if (client_sockfd >= 0) {
            DEBUG("New connection, client fd: 0x%08X\n", client_sockfd);
            
            /* Get the client's IP address */
            char remote_ip[16] = {0};
            sceNetInetInetNtop(AF_INET, &clientaddr.sin_addr.s_addr, remote_ip, sizeof(remote_ip));
            
            INFO("Client %i connected, IP: %s port: %i\n", number_clients, remote_ip, clientaddr.sin_port);
            
            /* Create a new thread for the client */
            char client_thread_name[64] = {0};
            std::sprintf(client_thread_name, "FTPpsp_client_%i_thread", number_clients);
            
            SceUID client_thid = sceKernelCreateThread(client_thread_name, client_thread, 0x12, 0x2000, PSP_THREAD_ATTR_USBWLAN, nullptr);
            
            DEBUG("Client %i thread UID: 0x%08X\n", number_clients, client_thid);
            
            /* Allocate the ftppsp_client_info_t struct for the new client */
            ftppsp_client_info_t *client = new ftppsp_client_info_t[sizeof(*client)];
            client->num = number_clients;
            client->thid = client_thid;
            client->ctrl_sockfd = client_sockfd;
            client->data_con_type = FTP_DATA_CONNECTION_NONE;
            std::strcpy(client->cur_path, FTP_DEFAULT_PATH);
            std::memcpy(&client->addr, &clientaddr, sizeof(client->addr));
            
            /* Add the new client to the client list */
            client_list_add(client);
            
            /* Start the client thread */
            sceKernelStartThread(client_thid, sizeof(client), &client);
        }
        else {
            /* if sceNetAccept returns < 0, it means that the listening
            * socket has been closed, this means that we want to
            * finish the server thread */
            DEBUG("Server socket closed, 0x%08X\n", client_sockfd);
            break;
        }
    }
    
    DEBUG("Server thread exiting!\n");
    
    sceKernelExitDeleteThread(0);
    return 0;
}

int ftppsp_init(char *psp_ip, unsigned short int *psp_port) {
    if (ftp_initialized)
        return -1;
    
    union SceNetApctlInfo info;
    if (sceNetApctlGetInfo(8, &info) < 0)
        return -1;
    else
        std::strcpy(psp_ip, info.ip);
        
    *psp_port = FTP_PORT;
    
    /* Save the IP of PSP to a global variable */
    if (sceNetInetInetPton(AF_INET, info.ip, &psp_addr) < 0)
        return -1;
        
    /* Create server thread */
    server_thid = sceKernelCreateThread("FTPpsp_server_thread", server_thread, 0x20, 0x6000, PSP_THREAD_ATTR_USBWLAN, nullptr);
    DEBUG("Server thread UID: 0x%08X\n", server_thid);
    
    /* Create the client list mutex */
    client_list_mtx = sceKernelCreateMutex("ftpsp_list_mutex", PSP_MUTEX_ATTR_FIFO, 0, nullptr);
    DEBUG("Client list mutex UID: 0x%08X\n", client_list_mtx);
    
    /* Init device list */
    for (int i = 0; i < MAX_DEVICES; i++)
        device_list[i].valid = 0;
        
    for (int i = 0; i < MAX_CUSTOM_COMMANDS; i++)
        custom_command_dispatchers[i].valid = 0;
        
    /* Start the server thread */
    sceKernelStartThread(server_thid, 0, nullptr);
    ftp_initialized = 1;
    return 0;
}

void ftppsp_fini(void) {
    if (ftp_initialized) {
        /* In order to "stop" the blocking sceNetAccept,
        * we have to close the server socket; this way
        * the accept call will return an error */
        sceNetInetClose(server_sockfd);
        
        /* Wait until the server threads ends */
        SceUInt timeout = 1000000;
        sceKernelWaitThreadEnd(server_thid, &timeout);
        //sceKernelTerminateDeleteThread(server_thid);
        
        /* To close the clients we have to do the same:
        * we have to iterate over all the clients
        * and shutdown their sockets */
        client_list_thread_end();
        
        /* Delete the client list mutex */
        sceKernelDeleteMutex(client_list_mtx);
        
        client_list = nullptr;
        number_clients = 0;
        
        if (net_memory)
            std::free(net_memory);
            
        net_memory = nullptr;
        ftp_initialized = 0;
    }
}

int ftppsp_is_initialized(void) {
    return ftp_initialized;
}

int ftppsp_add_device(const char *devname) {
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (!device_list[i].valid) {
            std::strcpy(device_list[i].name, devname);
            device_list[i].valid = 1;
            return 1;
        }
    }

    return 0;
}

int ftppsp_del_device(const char *devname) {
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (std::strcmp(devname, device_list[i].name) == 0) {
            device_list[i].valid = 0;
            return 1;
        }
    }

    return 0;
}

void ftppsp_set_info_log_cb(ftppsp_log_cb_t cb) {
    info_log_cb = cb;
}

void ftppsp_set_debug_log_cb(ftppsp_log_cb_t cb) {
    debug_log_cb = cb;
}

void ftppsp_set_file_buf_size(unsigned int size) {
    file_buf_size = size;
}

int ftppsp_ext_add_custom_command(const char *cmd, cmd_dispatch_func func) {
    for (int i = 0; i < MAX_CUSTOM_COMMANDS; i++) {
        if (!custom_command_dispatchers[i].valid) {
            custom_command_dispatchers[i].cmd = cmd;
            custom_command_dispatchers[i].func = func;
            custom_command_dispatchers[i].valid = 1;
            return 1;
        }
    }
    
    return 0;
}

int ftppsp_ext_del_custom_command(const char *cmd) {
    for (int i = 0; i < MAX_CUSTOM_COMMANDS; i++) {
        if (std::strcmp(cmd, custom_command_dispatchers[i].cmd) == 0) {
            custom_command_dispatchers[i].valid = 0;
            return 1;
        }
    }
    
    return 0;
}

void ftppsp_ext_client_send_ctrl_msg(ftppsp_client_info_t *client, const char *msg) {
    client_send_ctrl_msg(client, msg);
}

void ftppsp_ext_client_send_data_msg(ftppsp_client_info_t *client, const char *str) {
    client_send_data_msg(client, str);
}
