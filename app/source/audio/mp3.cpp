#include <mpg123.h>
#include <cstdio>
#include <cstring>

#include "audio.h"
#include "log.h"
#include "textures.h"

namespace MP3 {
    static mpg123_handle *mp3;
    static u64 frames_read = 0, total_samples = 0;
    static int channels = 0;
    static long sample_rate = 0;

    static inline void ReadID3v1Field(const char* field, size_t length, std::string& s) {
        char buf[length + 1];
        
        // copy the data into a temporary buffer to add a zero byte at the end of the text
        std::memcpy(buf, field, length);
        buf[length] = 0;
        s = buf;
    }

    static void ProcessID3v1(AudioMetadata& info, const mpg123_id3v1& id3) {
        std::string temp;

        if (info.title.empty()) {
            MP3::ReadID3v1Field(id3.title, 30, temp);
            info.title = temp;
        }

        if (info.album.empty()) {
            MP3::ReadID3v1Field(id3.album, 30, temp);
            info.album = temp;
        }
        
        if (info.artist.empty()) {
            MP3::ReadID3v1Field(id3.artist, 30, temp);
            info.artist = temp;
        }

        if (info.year.empty()) {
            MP3::ReadID3v1Field(id3.year, 30, temp);
            info.year = temp;
        }
    }

    void ProcessID3v2(AudioMetadata& info, const mpg123_id3v2& id3) {
        if (id3.title && id3.title->p)
            info.title = id3.title->p;

        if (id3.album && id3.album->p)
            info.album = id3.album->p;

        if (id3.artist && id3.artist->p)
            info.artist = id3.artist->p;

        if (id3.genre && id3.genre->p)
            info.genre = id3.genre->p;

        if (id3.year && id3.year->p)
            info.year = id3.year->p;

        size_t i = 0;
        for (i = 0; i < id3.pictures; ++i) {
            const mpg123_picture& pic = id3.picture[i];
            printf("\ncount: %d, pic.type: %d, pic.mime_type.p: %s\n", i, pic.type, pic.mime_type.p);
            
            if ((pic.type == mpg123_id3_pic_front_cover) || (pic.type == mpg123_id3_pic_back_cover)) {
                if ((!strcasecmp(pic.mime_type.p, "image/jpg")) || (!strcasecmp(pic.mime_type.p, "image/jpeg"))) {
                    metadata.cover_image = Textures::LoadImageBufferJPEG(pic.data, pic.size);
                    break;
                }
                else if (!strcasecmp(pic.mime_type.p, "image/png")) {
                    metadata.cover_image = Textures::LoadImageBufferPNG(pic.data, pic.size);
                    break;
                }
            }
        }
    }
    
    int Init(const std::string &path) {
        int error = 0;

        mp3 = mpg123_new(nullptr, &error);
        if (error != MPG123_OK) {
            Log::Error("mpg123_new(%s) failed: ", path.c_str(), mpg123_strerror(mp3));
            return error;
        }

        error = mpg123_param(mp3, MPG123_FLAGS, MPG123_FORCE_SEEKABLE | MPG123_FUZZY | MPG123_SEEKBUFFER | MPG123_GAPLESS, 0.0);
        if (error != MPG123_OK)
            return error;
            
        // Let the seek index auto-grow and contain an entry for every frame
        error = mpg123_param(mp3, MPG123_INDEX_SIZE, -1, 0.0);
        if (error != MPG123_OK)
            return error;
            
        error = mpg123_param(mp3, MPG123_ADD_FLAGS, MPG123_PICTURE, 0.);
        if (error != MPG123_OK) {
            Log::Error("mpg123_param(%s) MPG123_ADD_FLAGS failed: ", path.c_str(), mpg123_strerror(mp3));
            return error;
        }
            
        error = mpg123_open(mp3, path.c_str());
        if (error != MPG123_OK) {
            Log::Error("mpg123_open(%s) failed: ", path.c_str(), mpg123_strerror(mp3));
            return error;
        }

        mpg123_seek(mp3, 0, SEEK_SET);
        metadata.has_meta = mpg123_meta_check(mp3);
        
        mpg123_id3v1 *id3v1;
        mpg123_id3v2 *id3v2;
        if (metadata.has_meta & MPG123_ID3 && mpg123_id3(mp3, &id3v1, &id3v2) == MPG123_OK) {
            if (id3v1)
                MP3::ProcessID3v1(metadata, *id3v1);
            if (id3v2)
                MP3::ProcessID3v2(metadata, *id3v2);
        }
        
        mpg123_getformat(mp3, &sample_rate, &channels, nullptr);
        mpg123_format(mp3, sample_rate, channels, MPG123_ENC_SIGNED_16);
        total_samples = mpg123_length(mp3);
        return 0;
    }
    
    u32 GetSampleRate(void) {
        return sample_rate;
    }
    
    u8 GetChannels(void) {
        return channels;
    }
    
    void Decode(void *buf, unsigned int length, void *userdata) {
        int ret = 0;
        size_t done = 0;
        
        ret = mpg123_read(mp3, static_cast<unsigned char *>(buf), length * (sizeof(s16) * MP3::GetChannels()), &done);
        
        frames_read = mpg123_tell(mp3);
        if (frames_read >= total_samples || ret == MPG123_DONE)
            playing = false;
    }
    
    u64 GetPosition(void) {
        return frames_read;
    }
    
    u64 GetLength(void) {
        return total_samples;
    }
    
    u64 Seek(u64 index) {
        off_t seek_frame = (total_samples * (index / 225.0));
        
        if (mpg123_seek(mp3, seek_frame, SEEK_SET) >= 0) {
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
        
        mpg123_close(mp3);
        mpg123_delete(mp3);
        mpg123_exit();
    }
}
