#include <opusfile.h>

#include "audio.h"
#include "textures.h"

namespace OPUS {
    static OggOpusFile *opus;
    static ogg_int64_t samples_read = 0, max_samples = 0;
    
    int Init(const std::string &path) {
        int error = 0;
        if ((opus = op_open_file(path.c_str(), &error)) == nullptr)
            return -1;
            
        if ((error = op_current_link(opus)) < 0)
            return -1;
            
        max_samples = op_pcm_total(opus, -1);
        const OpusTags *tags = op_tags(opus, 0);
        if (opus_tags_query_count(tags, "title") > 0) {
            metadata.has_meta = true;
            metadata.title = opus_tags_query(tags, "title", 0);
        }
        
        if (opus_tags_query_count(tags, "album") > 0) {
            metadata.has_meta = true;
            metadata.album = opus_tags_query(tags, "album", 0);
        }
        
        if (opus_tags_query_count(tags, "artist") > 0) {
            metadata.has_meta = true;
            metadata.artist = opus_tags_query(tags, "artist", 0);
        }
        
        if (opus_tags_query_count(tags, "date") > 0) {
            metadata.has_meta = true;
            metadata.year = opus_tags_query(tags, "date", 0);
        }
        
        if (opus_tags_query_count(tags, "comment") > 0) {
            metadata.has_meta = true;
            metadata.comment = opus_tags_query(tags, "comment", 0);
        }
        
        if (opus_tags_query_count(tags, "genre") > 0) {
            metadata.has_meta = true;
            metadata.genre = opus_tags_query(tags, "genre", 0);
        }
        
        if (opus_tags_query_count(tags, "METADATA_BLOCK_PICTURE") > 0) {
            metadata.has_meta = true;
            OpusPictureTag picture_tag = { 0 };
            opus_picture_tag_init(&picture_tag);
            const char *metadata_block = opus_tags_query(tags, "METADATA_BLOCK_PICTURE", 0);
            
            int error = opus_picture_tag_parse(&picture_tag, metadata_block);
            if (error == 0) {
                if (picture_tag.type == 3) {
                    if (picture_tag.format == OP_PIC_FORMAT_JPEG)
                        metadata.cover_image = Textures::LoadImageBufferJPEG(picture_tag.data, picture_tag.data_length);
                    else if (picture_tag.format == OP_PIC_FORMAT_PNG)
                        metadata.cover_image = Textures::LoadImageBufferPNG(picture_tag.data, picture_tag.data_length);
                }
            }
            
            opus_picture_tag_clear(&picture_tag);
        }
        
        return 0;
    }
    
    u32 GetSampleRate(void) {
        return 48000;
    }
    
    u8 GetChannels(void) {
        return 2;
    }
    
    void Decode(void *buf, unsigned int length, void *userdata) {
        int read = op_read_stereo(opus, static_cast<opus_int16 *>(buf), static_cast<int>(length * (sizeof(s16) * 2)));
        if (read)
            samples_read = op_pcm_tell(opus);
            
        if (samples_read >= max_samples)
            playing = false;
    }
    
    u64 GetPosition(void) {
        return samples_read;
    }
    
    u64 GetLength(void) {
        return max_samples;
    }
    
    u64 Seek(u64 index) {
        if (op_seekable(opus) >= 0) {
            ogg_int64_t seek_sample = (max_samples * (index / 225.0));
            
            if (op_pcm_seek(opus, seek_sample) >= 0) {
                samples_read = seek_sample;
                return samples_read;
            }
        }
        
        return -1;
    }
    
    void Exit(void) {
        samples_read = 0;
        op_free(opus);
    }
}
