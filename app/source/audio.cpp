#include <pspaudio.h>
#include <pspthreadman.h>
#include <cstring>

#include "fs.h"
#include "kernel_functions.h"
#include "audio.h"
#include "pspaudiolib_cm.h"

AudioMetadata metadata = { 0 };
bool playing = true, paused = false;

namespace Audio {
    enum AudioFileType {
        FILE_TYPE_NONE,
        FILE_TYPE_FLAC,
        FILE_TYPE_MP3,
        FILE_TYPE_OGG,
        FILE_TYPE_OPUS,
        FILE_TYPE_WAV,
        FILE_TYPE_XM
    };

    typedef struct {
        int (* init)(const std::string &path);
        u32 (* rate)(void);
        u8 (* channels)(void);
        void (* decode)(void *buf, unsigned int length, void *userdata);
        u64 (* position)(void);
        u64 (* length)(void);
        u64 (* seek)(u64 index);
        void (* term)(void);
    } Decoder;
    
    static enum AudioFileType file_type = FILE_TYPE_NONE;
    static Decoder decoder = { 0 };
    
    static void Decode(void *buf, unsigned int length, void *userdata) {
        if ((!playing) || (paused)) {
            std::memset(buf, 0, length * 4);
        } 
        else {
            (*decoder.decode)(buf, length, userdata);
        }
    }
    
    void Init(const std::string &path) {
        playing = true;
        paused = false;
        const char *ext = FS::GetFileExt(path.c_str());

        if (strncasecmp(ext, "flac", 4) == 0) {
            file_type = FILE_TYPE_FLAC;
        }
        else if (strncasecmp(ext, "mp3", 3) == 0) {
            file_type = FILE_TYPE_MP3;
        }
        else if (strncasecmp(ext, "ogg", 3) == 0) {
            file_type = FILE_TYPE_OGG;
        }
        else if (strncasecmp(ext, "opus", 4) == 0) {
            file_type = FILE_TYPE_OPUS;
        }
        else if (strncasecmp(ext, "wav", 3) == 0) {
            file_type = FILE_TYPE_WAV;
        }
        else if ((strncasecmp(ext, "it", 2) == 0) || (strncasecmp(ext, "mod", 3) == 0) || (strncasecmp(ext, "s3m", 3) == 0)
            || (strncasecmp(ext, "xm", 2) == 0)) {
            file_type = FILE_TYPE_XM;
        }
            
        switch(file_type) {
            case FILE_TYPE_FLAC:
                decoder.init = FLAC::Init;
                decoder.rate = FLAC::GetSampleRate;
                decoder.channels = FLAC::GetChannels;
                decoder.decode = FLAC::Decode;
                decoder.position = FLAC::GetPosition;
                decoder.length = FLAC::GetLength;
                decoder.seek = FLAC::Seek;
                decoder.term = FLAC::Exit;
                break;
                
            case FILE_TYPE_MP3:
                decoder.init = MP3::Init;
                decoder.rate = MP3::GetSampleRate;
                decoder.channels = MP3::GetChannels;
                decoder.decode = MP3::Decode;
                decoder.position = MP3::GetPosition;
                decoder.length = MP3::GetLength;
                decoder.seek = MP3::Seek;
                decoder.term = MP3::Exit;
                break;
                
            case FILE_TYPE_OGG:
                decoder.init = OGG::Init;
                decoder.rate = OGG::GetSampleRate;
                decoder.channels = OGG::GetChannels;
                decoder.decode = OGG::Decode;
                decoder.position = OGG::GetPosition;
                decoder.length = OGG::GetLength;
                decoder.seek = OGG::Seek;
                decoder.term = OGG::Exit;
                break;
                
            case FILE_TYPE_OPUS:
                decoder.init = OPUS::Init;
                decoder.rate = OPUS::GetSampleRate;
                decoder.channels = OPUS::GetChannels;
                decoder.decode = OPUS::Decode;
                decoder.position = OPUS::GetPosition;
                decoder.length = OPUS::GetLength;
                decoder.seek = OPUS::Seek;
                decoder.term = OPUS::Exit;
                break;
                
            case FILE_TYPE_WAV:
                decoder.init = WAV::Init;
                decoder.rate = WAV::GetSampleRate;
                decoder.channels = WAV::GetChannels;
                decoder.decode = WAV::Decode;
                decoder.position = WAV::GetPosition;
                decoder.length = WAV::GetLength;
                decoder.seek = WAV::Seek;
                decoder.term = WAV::Exit;
                break;
                
            case FILE_TYPE_XM:
                decoder.init = XM::Init;
                decoder.rate = XM::GetSampleRate;
                decoder.channels = XM::GetChannels;
                decoder.decode = XM::Decode;
                decoder.position = XM::GetPosition;
                decoder.length = XM::GetLength;
                decoder.seek = XM::Seek;
                decoder.term = XM::Exit;
                break;
            
            default:
                break;
        }
        
        (* decoder.init)(path);
        pspAudioInit((* decoder.channels)() == 2? PSP_AUDIO_FORMAT_STEREO : PSP_AUDIO_FORMAT_MONO);
        pspAudioSetFrequency((* decoder.rate)() == 48000? 48000 : 44100);
        pspAudioSetChannelCallback(0, Audio::Decode, nullptr);
    }
    
    bool IsPaused(void) {
        return paused;
    }
    
    void Pause(void) {
        paused = !paused;
    }
    
    void Stop(void) {
        playing = !playing;
    }
    
    u64 GetPosition(void) {
        return (* decoder.position)();
    }
    
    u64 GetLength(void) {
        return (* decoder.length)();
    }
    
    u64 GetPositionSeconds(void) {
        return (Audio::GetPosition() / (* decoder.rate)());
    }
    
    u64 GetLengthSeconds(void) {
        return (Audio::GetLength() / (* decoder.rate)());
    }

    u64 Seek(u64 index) {
        return (* decoder.seek)(index);
    }
    
    void Exit(void) {
        playing = true;
        paused = false;
        
        pspAudioSetChannelCallback(0, nullptr, nullptr); // Clear channel callback
        pspAudioEndPre();
        sceKernelDelayThread(50 * 1000);
        pspAudioEnd();
        (* decoder.term)();
        
        // Clear metadata struct
        if (metadata.hasMeta && metadata.image) {
            g2dTexFree(&metadata.image);
        }
        
        metadata = { 0 };
        decoder =  { 0 };
    }
}
