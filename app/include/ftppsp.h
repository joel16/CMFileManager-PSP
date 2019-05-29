/*
 * Copyright (c) 2019 Sergi Granell (xerpi), Joel16
 */

# pragma once

#include <pspsdk.h>
#include <arpa/inet.h>

typedef void (*ftppsp_log_cb_t)(const char *);

/* Returns PSVita's IP and FTP port. 0 on success */
int ftppsp_init(char *psp_ip, unsigned short int *psp_port);
void ftppsp_fini(void);
int ftppsp_is_initialized(void);
int ftppsp_add_device(const char *devname);
int ftppsp_del_device(const char *devname);
void ftppsp_set_info_log_cb(ftppsp_log_cb_t cb);
void ftppsp_set_debug_log_cb(ftppsp_log_cb_t cb);
void ftppsp_set_file_buf_size(unsigned int size);

/* Extended functionality */

typedef enum {
	FTP_DATA_CONNECTION_NONE,
	FTP_DATA_CONNECTION_ACTIVE,
	FTP_DATA_CONNECTION_PASSIVE,
} DataConnectionType;

typedef struct ftppsp_client_info {
	/* Client number */
	int num;
	/* Thread UID */
	SceUID thid;
	/* Control connection socket FD */
	int ctrl_sockfd;
	/* Data connection attributes */
	int data_sockfd;
	DataConnectionType data_con_type;
	struct sockaddr_in data_sockaddr;
	/* PASV mode client socket */
	struct sockaddr_in pasv_sockaddr;
	int pasv_sockfd;
	/* Remote client net info */
	struct sockaddr_in addr;
	/* Receive buffer attributes */
	int n_recv;
	char recv_buffer[1024];
	/* Points to the character after the first space */
	const char *recv_cmd_args;
	/* Current working directory */
	char cur_path[1024];
	/* Rename path */
	char rename_path[1024];
	/* Client list */
	struct ftppsp_client_info *next;
	struct ftppsp_client_info *prev;
	/* Offset for transfer resume */
	unsigned int restore_point;
} ftppsp_client_info_t;


typedef void (*cmd_dispatch_func)(ftppsp_client_info_t *client); // Command handler

int ftppsp_ext_add_custom_command(const char *cmd, cmd_dispatch_func func);
int ftppsp_ext_del_custom_command(const char *cmd);
void ftppsp_ext_client_send_ctrl_msg(ftppsp_client_info_t *client, const char *msg);
void ftppsp_ext_client_send_data_msg(ftppsp_client_info_t *client, const char *str);
