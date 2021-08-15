#include "audio.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

namespace WAV {
    static drwav wav;
    static drwav_uint64 frames_read = 0;
    
    int Init(const std::string &path) {
        if (!drwav_init_file(&wav, path.c_str(), nullptr))
            return -1;
            
        return 0;
    }
    
    u32 GetSampleRate(void) {
        return wav.sampleRate;
    }
    
    u8 GetChannels(void) {
        return wav.channels;
    }
    
    void Decode(void *buf, unsigned int length, void *userdata) {
        frames_read += drwav_read_pcm_frames_s16(&wav, static_cast<drwav_uint64>(length), static_cast<drwav_int16 *>(buf));
        if (frames_read >= wav.totalPCMFrameCount)
            playing = false;
    }
    
    u64 GetPosition(void) {
        return frames_read;
    }
    
    u64 GetLength(void) {
        return wav.totalPCMFrameCount;
    }
    
    u64 Seek(u64 index) {
        drwav_uint64 seek_frame = (wav.totalPCMFrameCount * (index / 225.0));
        
        if (drwav_seek_to_pcm_frame(&wav, seek_frame) == DRWAV_TRUE) {
            frames_read = seek_frame;
            return frames_read;
        }
        
        return -1;
    }
    
    void Exit(void) {
        frames_read = 0;
        drwav_uninit(&wav);
    }
}
