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
    
    /* Helper for v1 printing, get these strings their zero byte. */
    void SafePrint(std::string &data, char *value, size_t size) {
        char safe[31];
        if (size > 30)
            return;
            
        std::memcpy(safe, value, size);
        safe[size] = 0;
        data = std::string(safe);
    }
    
    /* Print out ID3v1 info. */
    void ProcessID3v1(AudioMetadata &tag, mpg123_id3v1 *v1) {
        MP3::SafePrint(tag.title, v1->title, sizeof(v1->title));
        MP3::SafePrint(tag.artist, v1->artist, sizeof(v1->artist));
        MP3::SafePrint(tag.album, v1->album, sizeof(v1->album));
        MP3::SafePrint(tag.year, v1->year, sizeof(v1->year));
        MP3::SafePrint(tag.comment, v1->comment, sizeof(v1->comment));
    }
    
    /* Split up a number of lines separated by \n, \r, both or just zero byte
    and print out each line. */
    void PrintLines(std::string &data, mpg123_string *inlines) {
        int hadcr = 0, hadlf = 0;
        char *lines = nullptr;
        char *line = nullptr;
        size_t len = 0;
        
        if (inlines != nullptr && inlines->fill) {
            lines = inlines->p;
            len = inlines->fill;
        }
        else
            return;
            
        line = lines;
        for(size_t i = 0; i < len; ++i) {
            if (lines[i] == '\n' || lines[i] == '\r' || lines[i] == 0) {
                char save = lines[i]; /* saving, changing, restoring a byte in the data */
                
                if (save == '\n')
                    ++hadlf;
                
                if (save == '\r') 
                    ++hadcr;
                
                if ((hadcr || hadlf) && hadlf % 2 == 0 && hadcr % 2 == 0)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
                    line = "";
#pragma GCC diagnostic pop 
                    
                if (line) {
                    lines[i] = 0;
                    data = line;
                    line = nullptr;
                    lines[i] = save;
                }
            }
            else {
                hadlf = hadcr = 0;
                if (line == nullptr)
                    line = lines + i;
            }
        }
    }
    
    /* Print out the named ID3v2  fields. */
    void ProcessID3v2(AudioMetadata &tag, mpg123_id3v2 *v2) {
        MP3::PrintLines(tag.title, v2->title);
        MP3::PrintLines(tag.artist, v2->artist);
        MP3::PrintLines(tag.album, v2->album);
        MP3::PrintLines(tag.year, v2->year);
        MP3::PrintLines(tag.comment, v2->comment);
        MP3::PrintLines(tag.genre, v2->genre);
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
                MP3::ProcessID3v1(metadata, id3v1);
            if (id3v2)
                MP3::ProcessID3v2(metadata, id3v2);
                
            for (size_t i = 0; i < id3v2->pictures; ++i) {
                mpg123_picture *pic = &id3v2->picture[i];
                
                // Front cover or other
                if ((pic->type == 3)) {
                    if ((!strcasecmp(pic->mime_type.p, "image/jpeg")) || (!strcasecmp(pic->mime_type.p, "image/jpg"))) {
                        metadata.cover_image = Textures::LoadImageBufferJPEG(pic->data, pic->size);
                        break;
                    }
                    else if (!strcasecmp(pic->mime_type.p, "image/png")) {
                        metadata.cover_image = Textures::LoadImageBufferPNG(pic->data, pic->size);
                        
                        // I have trust issues
                        if (!metadata.cover_image)
                            metadata.cover_image = Textures::LoadImageBufferJPEG(pic->data, pic->size);
                        
                        break;
                    }
                }
            }
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
