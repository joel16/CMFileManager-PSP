#include <opusfile.h>

#include "audio.h"
#include "log.h"
#include "textures.h"

namespace OPUS {
    static OggOpusFile *opus;
    static ogg_int64_t samples = 0, totalSamples = 0;
    
    int Init(const std::string &path) {
        int error = 0;
        if ((opus = op_open_file(path.c_str(), &error)) == nullptr) {
            Log::Error("op_open_file failed to open: %s", path.c_str());
            return -1;
        }
            
        if ((error = op_current_link(opus)) < 0) {
            Log::Error("op_current_link failed to link: %s", path.c_str());
            return -1;
        }
            
        totalSamples = op_pcm_total(opus, -1);
        const OpusTags *tags = op_tags(opus, 0);

        if (tags != nullptr) {
            metadata.hasMeta = true;
        }

        if (opus_tags_query_count(tags, "title") > 0) {
            metadata.title = opus_tags_query(tags, "title", 0);
        }
        
        if (opus_tags_query_count(tags, "album") > 0) {
            metadata.album = opus_tags_query(tags, "album", 0);
        }
        
        if (opus_tags_query_count(tags, "artist") > 0) {
            metadata.artist = opus_tags_query(tags, "artist", 0);
        }
        
        if (opus_tags_query_count(tags, "date") > 0) {
            metadata.year = opus_tags_query(tags, "date", 0);
        }
        
        if (opus_tags_query_count(tags, "comment") > 0) {
            metadata.comment = opus_tags_query(tags, "comment", 0);
        }
        
        if (opus_tags_query_count(tags, "genre") > 0) {
            metadata.genre = opus_tags_query(tags, "genre", 0);
        }
        
        if (opus_tags_query_count(tags, "METADATA_BLOCK_PICTURE") > 0) {
            OpusPictureTag picture = { 0 };
            opus_picture_tag_init(&picture);
            const char *metadataBlock = opus_tags_query(tags, "METADATA_BLOCK_PICTURE", 0);
            
            int error = opus_picture_tag_parse(&picture, metadataBlock);
            if (error == 0) {
                if (picture.type == 3) {
                    if (picture.format == OP_PIC_FORMAT_JPEG) {
                        metadata.image = Textures::LoadImageBufferJPEG(picture.data, picture.data_length);
                    }
                    else if (picture.format == OP_PIC_FORMAT_PNG) {
                        metadata.image = Textures::LoadImageBufferPNG(picture.data, picture.data_length);
                    }
                }
            }
            
            opus_picture_tag_clear(&picture);
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
        int samplesRead = op_read_stereo(opus, static_cast<opus_int16 *>(buf), length);
        if (samplesRead) {
            samples = op_pcm_tell(opus);
        }
            
        if (samples >= totalSamples) {
            playing = false;
        }
    }
    
    u64 GetPosition(void) {
        return samples;
    }
    
    u64 GetLength(void) {
        return totalSamples;
    }
    
    u64 Seek(u64 index) {
        if (op_seekable(opus) >= 0) {
            ogg_int64_t seek = (totalSamples * (index / 225.0));
            
            if (op_pcm_seek(opus, seek) >= 0) {
                samples = seek;
                return samples;
            }
        }
        
        return -1;
    }
    
    void Exit(void) {
        samples = 0;

        if (opus) {
            op_free(opus);
            opus = nullptr;
        }
    }
}
