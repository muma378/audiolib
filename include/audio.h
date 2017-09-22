//
//  audio.h
//  
//
//  Created by Xiao Yang on 16/4/8.
//
//
#define _CRT_SECURE_NO_WARNINGS 1

#ifndef ____audio__
#define ____audio__

#include <fstream>
#include <iostream>
#include <vector>
#include <queue>
#include "common.h"
#include "riff.h"

#define HEADER_SIZE_DEF     44
//  parameters from riff_flag to sample_width are fixed
#define FIXED_HEADER_SIZE   36
#define MINIMUM_FMT_SIZE    16

// the following are deprecaed
// intervals between sample_width and data_flag may be filled with '\0'
#define FLOAT_HEADER_SIZE   8
#define MAX_PEEK_BYTES      40


enum Strategy {NORM, SMART, BALANCED};

typedef struct {
    size8_t   riff_flag[4]; // "RIFF"
    size32_t  size;         // size of overall file, equals to data_size + 44
    size8_t   wave_flag[4]; // "WAVE"
    size8_t   fmt_flag[4];  // "fmt\0"
    size32_t  length;       // length of format data as listed below, always 16, 18 or 40
    size16_t  tag;          // type of format, 1 stands for PCM
    size16_t  channels;     // number of channels
    size32_t  sample_rate;  // sample rate = number of samples per second, or Hertz
    size32_t  byte_rate;    // SampleRate * BitsPerSample * Channels / 8, number of bytes per second
    size16_t  sample_bytes; // sample_width * channels / 8, bytes per sample
    size16_t  sample_width; // bits used to represent each sample of each channel, note 'sample' indicated here is not as same as 'sample' in sample_bytes, it is not concerning channels
    size8_t   data_flag[4]; // "data"
    size32_t  data_size;    // size of data section
}wave_header_t;  // header lenght 44 bytes

typedef union {
    wave_header_t   header;
    char            buffer[HEADER_SIZE_DEF];
}header_buffer_t;

class BaseWave {
   
protected:
    wave_header_t   wave_header;// 44 bytes' header information
    const char*     content = nullptr;    // pointer to content
    const char*     filename = nullptr;
    
    header_buffer_t header_buffer;  // to read and save
    std::fstream         fs;         // file pointer if exists
    const uint SUFFIX_LENGTH = 4;   // .wav
    const char INDEX_SEP = '_';
    
    void set_sample_rate(const uint sample_rate);
    void set_data_size(const uint data_size);
    void set_content_ptr(const char* ptr);
    void update_channels_num(uint channel_num);
    void seek_dataflag();   // deprecated, underlying error may happen
    void seek_dataflag(chunk_buffer_t&);
    void push_clip(std::vector<BaseWave&> clips_vec, const uint clip_begining_byte, const uint clip_ending_byte);


public:
    BaseWave(){};
    BaseWave(const BaseWave& other);
    ~BaseWave(){
        delete [] content;
        delete [] filename;
        if (fs.is_open()){
            fs.close();
        }
    };
    
    static const unsigned short HEADER_SIZE = HEADER_SIZE_DEF;
    static const char* RIFF;
    static const char* WAVE;
    static const char* FMT;
    static const char* DATA;
    
    // append "\0" at the end of the flag so that it could be printed correctly
    static const char* flag_to_str(const size8_t* flag, uint length)  {
        char * cstr = new char [length + 1];
#ifdef __STDC_LIB_EXT1__
        strcpy_s(cstr, length, (char*)(flag));
#else
        strncpy(cstr, (char*)(flag), length);
#endif
        cstr[length] = '\0';
        return cstr;
    }
    
    friend std::ostream& operator<<(std::ostream& out, const BaseWave& w){
        const char* riff_flag_str = flag_to_str(w.wave_header.riff_flag, 4);
        const char* wave_flag_str = flag_to_str(w.wave_header.wave_flag, 4);
        const char* fmt_flag_str  = flag_to_str(w.wave_header.fmt_flag, 4);
        const char* data_flag_str = flag_to_str(w.wave_header.data_flag, 4);
        
        out << w.filename << " with header info: \n";
        out << "riff flag: " << riff_flag_str << "\t\tfile size: " << w.wave_header.size;
        out << "\nwave flag: " << wave_flag_str << "\t\tfmt flag: " << fmt_flag_str;
        out << "\nfmt length: " << w.wave_header.length << "\t\ttag: " << w.wave_header.tag;
        out << "\nchannels: " << w.wave_header.channels << "\t\tsample rate: " << w.wave_header.sample_rate;
        out << "\nbyte rate: " << w.wave_header.byte_rate << "\tbytes per frame: " << w.wave_header.sample_bytes;
        out << "\nbits per sample: " << w.wave_header.sample_width << "\tdata flag: " << data_flag_str;
        out << "\ndata size: " << w.wave_header.data_size << std::endl;
        
        delete [] riff_flag_str;
        delete [] wave_flag_str;
        delete [] fmt_flag_str;
        delete [] data_flag_str;
        return out;
    }
    
    BaseWave& operator=(const BaseWave& other);
    
    void open(const char* filename);    // open a wav file
    void deprecated_open(const char* filename);     // open with an nonstandard way
    void write();
    void write(const char* filename);   
    bool is_stereo() const;
    bool is_normalized() const;
    bool is_valid(wave_header_t header) const;  // to check if all flags were set correctly
    
    const uint sec2byte(const float duration) const;    // gets the number of bytes used in the duration
    const uint sec2sample(const float duration) const;    // gets the number of samples need in the duration
    void set_header(const BaseWave& wav);   // copy wav.wave_header to this
    void set_header(const uint channels, const uint sample_rate, const uint sample_width, const uint data_size);
    const char* get_filename() const;
    void set_filename(const char* new_name);
    const uint get_samples_num() const;
    const float get_duration() const;
    
    void normalize();   // remove extra bytes, leave clean header only
    void interleaved_copy(char* dst, uint size, uint cycle_len, uint samp_len);  // only copy first samp_len bytes in each cycle from this->content to dst
    void get_samples(std::vector<float>& samples, const uint begining_byte, const uint bytes_num) const;
    const float get_samples_avg(const uint begining_byte, const uint bytes_num) const;  // get the avarage value of samples from the begining byte with the size of bytes_num
    const char* get_clip_name(uint index);      // return "$(filename)_$(index).wav"
    
    BaseWave& stereo2mono();
    void downsample(const uint low_samp_rate=8000);     // lowring samples according to the new low_sample_rate
    std::vector<BaseWave*>& truncate(const uint max_duration, std::vector<BaseWave*>& wav_vec);         // split wav into pieces if its duration was over the max_duration
    // truncate but make sure no voice were to be splited
    std::vector<BaseWave*>& smart_truncate(const uint max_duration, std::vector<BaseWave*>& wav_vec, float window=0.5, float threshold=200.0, const float offset=0.1);
    // truncate but try to make each clips' size equals
    std::vector<BaseWave*>& balanced_truncate(const uint max_duration, std::vector<BaseWave*>& wav_vec, const uint min_duration=0, float window=0.5, float threshold=200.0, const float offset=0.1);
    std::queue<uint>& equi_divide(std::queue<uint>& kerf, const uint min_duration, const uint max_duration);

    BaseWave* extract(const uint begining_byte, const uint ending_byte) const;
    BaseWave* extract(const float begining_sec, const float ending_sec) const;
    
    
};


#endif /* defined(____audio__) */
