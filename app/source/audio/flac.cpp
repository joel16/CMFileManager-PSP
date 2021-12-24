#include <FLAC/stream_decoder.h>

#include "audio.h"
#include "log.h"
#include "textures.h"

namespace FLAC {
    typedef struct {
        const FLAC__Frame *frame = nullptr;
        const FLAC__int32 * const *buffer = nullptr;
        FLAC__uint8 channels = 0;
        FLAC__uint32 sample_rate = 0;
        FLAC__uint32 bps = 0;
        FLAC__uint64 position = 0;
        FLAC__uint64 total_samples = 0;
    } FLACInfo;
    
    static FLAC__StreamDecoder *flac = nullptr;
    static FLACInfo cb_info { 0 };
    
    static FLAC__StreamDecoderWriteStatus write_cb(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data) {
        FLACInfo *info = reinterpret_cast<FLACInfo *>(client_data);

        if (info->total_samples == 0) {
            std::printf("No samples to decode!\n");
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
        if ((info->channels != 2) || (info->bps != 16)) {
            std::printf("Not stereo 16-bit!\n");
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
        
        info->frame = frame;
        info->buffer = buffer;
        info->position = 0;
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }
    
    static void metadata_cb(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *stream, void *client_data) {
        FLACInfo *info = reinterpret_cast<FLACInfo *>(client_data);

        switch (stream->type) {
            case FLAC__METADATA_TYPE_STREAMINFO:
                info->total_samples = stream->data.stream_info.total_samples;
                info->sample_rate = stream->data.stream_info.sample_rate;
                info->channels = stream->data.stream_info.channels;
                info->bps = stream->data.stream_info.bits_per_sample;
                break;

            case FLAC__METADATA_TYPE_VORBIS_COMMENT:
                for (FLAC__uint32 i = 0; i < stream->data.vorbis_comment.num_comments; i++) {
                    char *tag = reinterpret_cast<char *>(stream->data.vorbis_comment.comments[i].entry);
                    
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
                break;

            case FLAC__METADATA_TYPE_PICTURE:
                if (stream->data.picture.type == FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER) {
                    if ((!strcasecmp(stream->data.picture.mime_type, "image/jpeg")) || (!strcasecmp(stream->data.picture.mime_type, "image/jpg"))) {
                        metadata.has_meta = true;
                        
                        if (!metadata.cover_image)
                            metadata.cover_image = Textures::LoadImageBufferJPEG(stream->data.picture.data, stream->data.picture.data_length);
                    }
                    else if (!strcasecmp(stream->data.picture.mime_type, "image/png")) {
                        metadata.has_meta = true;
                        
                        if (!metadata.cover_image)
                            metadata.cover_image = Textures::LoadImageBufferPNG(stream->data.picture.data, stream->data.picture.data_length);
                    }
                }
                break;

            default:
                break;
        }
    }
    
    static void error_cb(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {
        std::string error;

        switch(status) {
            case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
                error = "Lost sync";
                break;
                
            case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
                error = "Bad header";
                break;
                
            case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
                error = "Frame CRC mismatch";
                break;
            
            case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
                error = "Unparseable stream";
                break;
                
            default:
                error = "???";
        }
        
        std::printf("error: %s\n", error.c_str());
    }
    
    int Init(const std::string &path) {
        if ((flac = FLAC__stream_decoder_new()) == nullptr)
            return -1;

        if (FLAC__stream_decoder_set_metadata_respond(flac, FLAC__METADATA_TYPE_STREAMINFO) == false) {
            Log::Error("FLAC__METADATA_TYPE_STREAMINFO response failed\n");
            return -1;
        }

        if (FLAC__stream_decoder_set_metadata_respond(flac, FLAC__METADATA_TYPE_SEEKTABLE) == false)
            Log::Error("FLAC__METADATA_TYPE_SEEKTABLE response failed\n");

        if (FLAC__stream_decoder_set_metadata_respond(flac, FLAC__METADATA_TYPE_VORBIS_COMMENT) == false)
            Log::Error("FLAC__METADATA_TYPE_VORBIS_COMMENT response failed\n");

        if (FLAC__stream_decoder_set_metadata_respond(flac, FLAC__METADATA_TYPE_PICTURE) == false)
            Log::Error("FLAC__METADATA_TYPE_PICTURE response failed\n");

        FLAC__StreamDecoderInitStatus ret;	
        if ((ret = FLAC__stream_decoder_init_file(flac, path.c_str(), write_cb, metadata_cb, error_cb, &cb_info)) != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
            Log::Error("FLAC__stream_decoder_init_file failed: %s\n", FLAC__StreamDecoderInitStatusString[ret]);
            return ret;
        }
        
        return FLAC__stream_decoder_process_until_end_of_metadata(flac);
    }
    
    u32 GetSampleRate(void) {
        return cb_info.sample_rate;
    }
    
    u8 GetChannels(void) {
        return cb_info.channels;
    }
    
    void Decode(void *buf, unsigned int length, void *userdata) {
        unsigned int decoded = 0;
        FLAC__bool ret = false;
        
        if (length <= 0)
            return;
            
        FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(flac);
        if (state == FLAC__STREAM_DECODER_END_OF_STREAM)
            playing = false;
        else if (state == FLAC__STREAM_DECODER_ABORTED)
            return;
            
        while(decoded < length) {
            if (cb_info.frame == nullptr || cb_info.position == cb_info.frame->header.blocksize) {
                ret = FLAC__stream_decoder_process_single(flac);
                
                if (ret == false) {
                    playing = false;
                    return;
                }
                
                state = FLAC__stream_decoder_get_state(flac);
                
                if (state == FLAC__STREAM_DECODER_END_OF_STREAM || state == FLAC__STREAM_DECODER_ABORTED) {
                    playing = false;
                    return;
                }
            }
            
            for(; decoded < length && cb_info.position < cb_info.frame->header.blocksize; cb_info.position++, decoded++) {
                // Copy to buffer here; convert from BE to LE
                short *buffer = static_cast<short *>(buf);
                buffer[decoded * 2] = cb_info.buffer[0][cb_info.position];
                buffer[decoded * 2 + 1] = cb_info.buffer[1][cb_info.position];
            }
        }
    }
    
    u64 GetPosition(void) {
        FLAC__uint64 position = 0;
        FLAC__stream_decoder_get_decode_position(flac, &position);
        return position;
    }
    
    u64 GetLength(void) {
        return cb_info.total_samples;
    }
    
    u64 Seek(u64 index) {
        return 0;
    }
    
    void Exit(void) {
        cb_info = { 0 };
        FLAC__stream_decoder_finish(flac);
        FLAC__stream_decoder_delete(flac);
    }
}
