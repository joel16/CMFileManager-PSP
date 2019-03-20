#include <malloc.h>

#include "audio.h"
#include "fs.h"
#define JAR_XM_IMPLEMENTATION
#include "jar_xm.h"
#include "utils.h"

static jar_xm_context_t *xm;
static u64 samples_read = 0, max_samples = 0;
static void *data = NULL;

int XM_Init(const char *path) {
    SceUID file = 0;
    int ret = 0;
    u64 bytes_read = 0;

    if (R_FAILED(ret = file = sceIoOpen(path, PSP_O_RDONLY, 0)))
        return -1;

    SceIoStat stat;
    if (R_FAILED(ret = sceIoGetstat(path, &stat))) {
        sceIoClose(file);
        return -1;
    }

    data = malloc(stat.st_size);
    if (!data) {
        free(data);
        sceIoClose(file);
        return -1;
    }

    bytes_read = sceIoRead(file, data, stat.st_size);
    if (bytes_read != stat.st_size) {
        free(data);
        sceIoClose(file);
        return -1;
    }

    sceIoClose(file);

    jar_xm_create_context_safe(&xm, data, stat.st_size, 44100);
    max_samples = jar_xm_get_remaining_samples(xm); // Initial remaining = max
    return 0;
}

void XM_Decode(void *buf, unsigned int length, void *userdata) {
    jar_xm_generate_samples_16bit(xm, (short *)buf, length);
    jar_xm_get_position(xm, NULL, NULL, NULL, &samples_read);

    if (samples_read == max_samples)
        playing = false;
}

u64 XM_GetPosition(void) {
    return samples_read;
}

u64 XM_GetLength(void) {
    return max_samples;
}

u64 XM_GetPositionSeconds(const char *path) {
    return (samples_read / 44100);
}

u64 XM_GetLengthSeconds(const char *path) {
    return (max_samples / 44100);
}

void XM_Term(void) {
    samples_read = 0;
    jar_xm_free_context(xm);
    free(data);
}
