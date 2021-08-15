#include <FLAC/metadata.h>

#include "audio.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"
#include "textures.h"

namespace FLAC {
    static drflac *flac;
    static drflac_uint64 frames_read = 0;
    
    int Init(const std::string &path) {
        flac = drflac_open_file(path.c_str(), nullptr);
        if (flac == nullptr)
            return -1;
            
        FLAC__StreamMetadata *tags;
        if (FLAC__metadata_get_tags(path.c_str(), &tags)) {
            for (FLAC__uint32 i = 0; i < tags->data.vorbis_comment.num_comments; i++)  {
                char *tag = reinterpret_cast<char *>(tags->data.vorbis_comment.comments[i].entry);
                
                if (!strncasecmp("TITLE=", tag, 6)) {
                    metadata.has_meta = true;
                    metadata.title = tag + 6;
                }
                
                if (!strncasecmp("ALBUM=", tag, 6)) {
                    metadata.has_meta = true;
                    metadata.album = tag + 6;
                }
                
                if (!strncasecmp("ARTIST=", tag, 7)) {
                    metadata.has_meta = true;
                    metadata.artist = tag + 7;
                }
                
                if (!strncasecmp("DATE=", tag, 5)) {
                    metadata.has_meta = true;
                    metadata.year = tag + 5;
                }
                
                if (!strncasecmp("COMMENT=", tag, 8)) {
                    metadata.has_meta = true;
                    metadata.comment = tag + 8;
                }
                
                if (!strncasecmp("GENRE=", tag, 6)) {
                    metadata.has_meta = true;
                    metadata.genre = tag + 6;
                }
            }
        }
        
        if (tags)
            FLAC__metadata_object_delete(tags);
            
        FLAC__StreamMetadata *picture;
        if (FLAC__metadata_get_picture(path.c_str(), &picture, FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER, "image/jpg", nullptr, 512, 512, -1, -1)) {
            metadata.has_meta = true;
            metadata.cover_image = Textures::LoadImageBufferJPEG(picture->data.picture.data, picture->length);
            FLAC__metadata_object_delete(picture);
        }
        else if (FLAC__metadata_get_picture(path.c_str(), &picture, FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER, "image/jpeg", nullptr, 512, 512, -1, -1)) {
            metadata.has_meta = true;
            metadata.cover_image = Textures::LoadImageBufferJPEG(picture->data.picture.data, picture->length);
            FLAC__metadata_object_delete(picture);
        }
        else if (FLAC__metadata_get_picture(path.c_str(), &picture, FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER, "image/png", nullptr, 512, 512, -1, -1)) {
            metadata.has_meta = true;
            metadata.cover_image = Textures::LoadImageBufferPNG(picture->data.picture.data, picture->length);
            FLAC__metadata_object_delete(picture);
        }
        
        return 0;
    }
    
    u32 GetSampleRate(void) {
        return flac->sampleRate;
    }
    
    u8 GetChannels(void) {
        return flac->channels;
    }
    
    void Decode(void *buf, unsigned int length, void *userdata) {
        frames_read += drflac_read_pcm_frames_s16(flac, static_cast<drflac_uint64>(length), static_cast<drflac_int16 *>(buf));
        if (frames_read >= flac->totalPCMFrameCount)
            playing = false;
    }
    
    u64 GetPosition(void) {
        return frames_read;
    }
    
    u64 GetLength(void) {
        return flac->totalPCMFrameCount;
    }
    
    u64 Seek(u64 index) {
        drflac_uint64 seek_frame = (flac->totalPCMFrameCount * (index / 225.0));
        
        if (drflac_seek_to_pcm_frame(flac, seek_frame) == DRFLAC_TRUE) {
            frames_read = seek_frame;
            return frames_read;
        }
        
        return -1;
    }
    
    void Exit(void) {
        frames_read = 0;
        
        if (metadata.has_meta) {
            metadata.has_meta = false;
            
            if (metadata.cover_image)
                g2dTexFree(&metadata.cover_image);
        }
        
        drflac_close(flac);
    }
    
    // Functions needed for libFLAC
    int chmod(const char *pathname, mode_t mode) {
        return 0;
    }
    
    int chown(const char *path, int owner, int group) {
        return 0;
    }
    
    int utime(const char *filename, const void *buf) {
        return 0;
    }
}
