#include "audio.h"
#include "xmp.h"

namespace XM {
    static xmp_context xmp;
    static struct xmp_frame_info frame_info;
    static struct xmp_module_info module_info;
    static SceUInt64 samples_read = 0, total_samples = 0;
    
    int Init(const std::string &path) {
        xmp = xmp_create_context();
        if (xmp_load_module(xmp, const_cast<char *>(path.c_str())) < 0)
            return -1;
            
        xmp_start_player(xmp, 44100, 0);
        xmp_get_frame_info(xmp, &frame_info);
        total_samples = (frame_info.total_time * 44.1);
        
        xmp_get_module_info(xmp, &module_info);
        if (module_info.mod->name[0] != '\0') {
            metadata.has_meta = true;
            metadata.title = module_info.mod->name;
        }
        
        return 0;
    }
    
    u32 GetSampleRate(void) {
        return 44100;
    }
    
    u8 GetChannels(void) {
        return 2;
    }
    
    void Decode(void *buf, unsigned int length, void *userdata) {
        xmp_play_buffer(xmp, buf, static_cast<int>(length) * (sizeof(s16) * 2), 0);
        samples_read += length;
        
        if (samples_read >= total_samples)
            playing = false;
    }
    
    u64 GetPosition(void) {
        return samples_read;
    }
    
    u64 GetLength(void) {
        return total_samples;
    }
    
    u64 Seek(u64 index) {
        int seek_sample = (total_samples * (index / 225.0));
        
        if (xmp_seek_time(xmp, (seek_sample / 44.1)) >= 0) {
            samples_read = seek_sample;
            return samples_read;
        }
        
        return -1;
    }
    
    void Exit(void) {
        samples_read = 0;
        xmp_end_player(xmp);
        xmp_release_module(xmp);
        xmp_free_context(xmp);
    }
}
