#ifndef _CMFILEMANAGER_AUDIO_H_
#define _CMFILEMANAGER_AUDIO_H_

#include <glib2d.h>
#include <psptypes.h>
#include <string>

extern bool playing, paused;

typedef struct {
    bool has_meta = false;
    std::string title;
    std::string album;
    std::string artist;
    std::string year;
    std::string comment;
    std::string genre;
    g2dTexture *cover_image;
} AudioMetadata;

extern AudioMetadata metadata;

namespace FLAC {
    int Init(const std::string &path);
    u32 GetSampleRate(void);
    u8 GetChannels(void);
    void Decode(void *buf, unsigned int length, void *userdata);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 Seek(u64 index);
    void Exit(void);
}

namespace MP3 {
    int Init(const std::string &path);
    u32 GetSampleRate(void);
    u8 GetChannels(void);
    void Decode(void *buf, unsigned int length, void *userdata);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 Seek(u64 index);
    void Exit(void);
}

namespace OGG {
    int Init(const std::string &path);
    u32 GetSampleRate(void);
    u8 GetChannels(void);
    void Decode(void *buf, unsigned int length, void *userdata);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 Seek(u64 index);
    void Exit(void);
}

namespace OPUS {
    int Init(const std::string &path);
    u32 GetSampleRate(void);
    u8 GetChannels(void);
    void Decode(void *buf, unsigned int length, void *userdata);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 Seek(u64 index);
    void Exit(void);
}

namespace WAV {
    int Init(const std::string &path);
    u32 GetSampleRate(void);
    u8 GetChannels(void);
    void Decode(void *buf, unsigned int length, void *userdata);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 Seek(u64 index);
    void Exit(void);
}

namespace XM {
    int Init(const std::string &path);
    u32 GetSampleRate(void);
    u8 GetChannels(void);
    void Decode(void *buf, unsigned int length, void *userdata);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 Seek(u64 index);
    void Exit(void);
}

namespace Audio {
    void Init(const std::string &path);
    bool IsPaused(void);
    void Pause(void);
    void Stop(void);
    u64 GetPosition(void);
    u64 GetLength(void);
    u64 GetPositionSeconds(void);
    u64 GetLengthSeconds(void);
    u64 Seek(u64 index);
    void Exit(void);
}

#endif
