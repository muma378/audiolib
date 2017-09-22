//
//  audio.cpp
//  
//
//  Created by Xiao Yang on 16/4/8.
//
//

#define __STDC_WANT_LIB_EXT1__ 1
#define _CRT_SECURE_NO_WARNINGS 1


#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <stdlib.h>
#include <queue>
#include <assert.h>

#include "exceptions.h"
#include "common.h"
#include "audio.h"
#include "riff.h"

using namespace std;

const char* BaseWave::RIFF = "RIFF";
const char* BaseWave::WAVE = "WAVE";
const char* BaseWave::FMT  = "fmt ";
const char* BaseWave::DATA = "data";


inline void BaseWave::set_sample_rate(const uint sample_rate){
    wave_header.sample_rate = sample_rate;
    wave_header.byte_rate = sample_rate * wave_header.sample_width * wave_header.channels / 8;
    return;
};

inline void BaseWave::set_data_size(const uint data_size){
    wave_header.data_size = data_size;
    wave_header.size = data_size + 44;
    return;
};

inline void BaseWave::set_content_ptr(const char *ptr){
    content = ptr;
}

const char* BaseWave::get_filename() const{
    return filename;
}

BaseWave::BaseWave(const BaseWave& other): wave_header(other.wave_header), content(other.content){
}

BaseWave& BaseWave::operator=(const BaseWave& other) {
    if (this != &other) {
        set_header(other);
        delete [] content;      // free the old space if it was allocated before
        char* data_ptr = new char[wave_header.data_size];
        std::memcpy(data_ptr, other.content, wave_header.data_size);
        set_content_ptr(data_ptr);
    }
    return *this;
};

bool BaseWave::is_valid(wave_header_t h) const {
    if (strncmp((const char*)h.riff_flag, RIFF, 4)==0 &&   // RIFF
        strncmp((const char*)h.wave_flag, WAVE, 4)==0 &&   // WAVE
        strncmp((const char*)h.fmt_flag, FMT, 4)==0){     // fmt\0
        if (h.tag != 1) // PCM
            LOG_WARN("wave tag is not PCM");
        return true;    // only process the situation above
    }
    return false;
}

void BaseWave::set_filename(const char *new_name){
    uint name_len = (unsigned)strlen(new_name);
    if (filename) { // not a null ptr
        delete [] filename;
    }
    char* temp_name = new char[name_len+1];
    for (uint i=0; i<name_len; i++) {
        temp_name[i] = new_name[i];
    }
    temp_name[name_len] = '\0';
    filename = temp_name;
}


void BaseWave::set_header(const uint channels, const uint sample_rate, const uint sample_width, const uint data_size){
    // set flags
    set_flag_vals(wave_header.riff_flag, RIFF);
    set_flag_vals(wave_header.wave_flag, WAVE);
    set_flag_vals(wave_header.fmt_flag, FMT);
    set_flag_vals(wave_header.data_flag, DATA);
    
    // set parameters
    wave_header.length      = 16;
    wave_header.tag         = 1;
    wave_header.channels    = channels;
    wave_header.sample_rate = sample_rate;
    wave_header.sample_width= sample_width;
    wave_header.data_size   = data_size;
    
    wave_header.size        = wave_header.data_size + 44;
    wave_header.byte_rate   = sample_rate * sample_width * channels / 8;
    wave_header.sample_bytes= sample_width * channels / 8;
    return;
}

void BaseWave::set_header(const BaseWave& other){
    set_header(other.wave_header.channels, other.wave_header.sample_rate, other.wave_header.sample_width, other.wave_header.data_size);
}

bool BaseWave::is_normalized() const{
    return (wave_header.length == 16);
}

void BaseWave::normalize(){
    set_header(wave_header.channels, wave_header.sample_rate, wave_header.sample_width, wave_header.data_size);
}


void BaseWave::update_channels_num(uint channel_num){
    uint decrease_rate = wave_header.channels / channel_num;
    wave_header.channels = channel_num;
    wave_header.byte_rate /= decrease_rate;
    wave_header.sample_bytes /= decrease_rate;
    wave_header.tag = 1;
    wave_header.length = 16;
    set_data_size(wave_header.data_size/decrease_rate);
    return;
}


void BaseWave::deprecated_open(const char* filename){
    fs.open(filename, fstream::in | fstream::binary);
    fs.read(header_buffer.buffer, FIXED_HEADER_SIZE);       // read header to the union
    seek_dataflag();
    fs.read(header_buffer.buffer+FIXED_HEADER_SIZE, FLOAT_HEADER_SIZE);
   
    if (is_valid(header_buffer.header)){
        wave_header = header_buffer.header;
        delete [] content;
        char* data_ptr = new char[wave_header.data_size];
        fs.read(data_ptr, wave_header.data_size);
        fs.close();
        set_content_ptr(data_ptr);
        set_filename(filename);
    }else{
        fs.close();
        throw UnreadableException("invalid wave header format");
    }
}


void BaseWave::open(const char* filename){
    fs.open(filename, fstream::in | fstream::binary);
    fs.read(header_buffer.buffer, FIXED_HEADER_SIZE);
    int offset = header_buffer.header.length - MINIMUM_FMT_SIZE;
    assert(offset >= 0);
    if(offset){    // unfinished fmt chunk
        fs.seekg(offset, fs.cur);   // reach the end of chunk
    }
    
    if (!is_valid(header_buffer.header)) {
        fs.close();
        throw UnreadableException("invalid wave format");
    }
    
    chunk_buffer_t chunk_buffer;
    seek_dataflag(chunk_buffer);    // find the flag 'data' and its content
    // copy relative info to the 'header' and 'body'
    strncpy(header_buffer.header.data_flag, chunk_buffer.chunk.ck_id, CHUNK_HEAD_SIZE);
    header_buffer.header.data_size = chunk_buffer.chunk.ck_size;
    wave_header = header_buffer.header;
    delete[] content;
    set_content_ptr(chunk_buffer.chunk.ck_data);
    set_filename(filename);
    fs.close();
}



void BaseWave::seek_dataflag(chunk_buffer_t& chunk_buffer){
    chunk_buffer.chunk.ck_id[0] = '\0';
    chunk_buffer.chunk.ck_size = 0;
    
    int remaining = header_buffer.header.size - header_buffer.header.length;
    while (strncmp((const char*)chunk_buffer.chunk.ck_id, DATA, 4)!=0){
        if (chunk_buffer.chunk.ck_size < 0) {
            throw UnreadableException("illegal chunk included");
        }else{
            // shift the file pointer
            if (remaining >= chunk_buffer.chunk.ck_size) {
                fs.seekg(chunk_buffer.chunk.ck_size, fs.cur);
                remaining -= chunk_buffer.chunk.ck_size;
            }else{
                throw UnreadableException("unable to find flag - data");
            }
            // read the header in the next chunk
            fs.read(chunk_buffer.buffer, CHUNK_HEAD_SIZE);
            remaining -= CHUNK_HEAD_SIZE;
        }
    };
    char* data_ptr = new char[chunk_buffer.chunk.ck_size];
    fs.read(data_ptr, chunk_buffer.chunk.ck_size);
    chunk_buffer.chunk.ck_data = data_ptr;
}


void BaseWave::seek_dataflag(){
    char next_character = fs.peek();
    char* trash = new char;
    int counter = 0;
    while (strncmp(&next_character, DATA, 1)) {
        fs.read(trash, 1);  // thrown to bin
        if (counter++ > MAX_PEEK_BYTES){
            throw UnreadableException("unable to extract size info about data");
        }
        next_character = fs.peek(); // return the next character but not extracting
    }
    delete trash;
    return;
}


void BaseWave::write(){
    if (filename != nullptr && strlen(filename)) {
        write(filename);
    }else{
        throw UnwritableException("no filename is specified to write");
    }
}

void BaseWave::write(const char* filename){
    ofstream ofs(filename, fstream::out|fstream::binary);
    header_buffer.header = wave_header;
    if (ofs.is_open()) {
        ofs.write(header_buffer.buffer, HEADER_SIZE);
        ofs.write(content, wave_header.data_size);
        ofs.close();
    }
}

const float BaseWave::get_duration() const{
    return wave_header.data_size / float(wave_header.byte_rate);
}

const uint BaseWave::get_samples_num() const{
    return uint(wave_header.data_size/wave_header.sample_bytes);
}


const uint BaseWave::sec2byte(const float duration) const{
    return uint(wave_header.byte_rate * duration);
}


const uint BaseWave::sec2sample(const float duration) const{
    return uint(wave_header.sample_rate * wave_header.channels * duration);
}

void BaseWave::get_samples(vector<float>& samples, const uint begining_byte, const uint bytes_num) const{
    switch (wave_header.sample_width){
        case 8:
            pack(content, samples, bytes_num, begining_byte);
            break;
        case 16:
            pack((size16_t*)content, samples, bytes_num/2, begining_byte/2);
            break;
        case 32:
            pack((size32_t*)content, samples, bytes_num/4, begining_byte/4);
            break;
        default:
            const char* err_msg = ("width of sample detected is not in the range: " + to_string(wave_header.sample_width)).c_str();
            throw UnreadableException(err_msg);
    }
    return;
}

const float BaseWave::get_samples_avg(const uint begining_byte, const uint bytes_num) const{
    switch (wave_header.sample_width) {
        case 8:
            return avg_pack(content, bytes_num, begining_byte);     // samples_num = bytes_num * 8/
        case 16:
            return avg_pack((size16_t*)content, bytes_num/2, begining_byte/2);
        case 32:
            return avg_pack((size32_t*)content, bytes_num/4, begining_byte/4);
            
        default:
			string err_msg("width of sample detected is not in the range: ");
			err_msg += to_string(wave_header.sample_width);
            throw UnreadableException(err_msg.c_str());
    }
}

void BaseWave::interleaved_copy(char* dst, uint size, uint cycle_len, uint samp_len){
    const uint interval = cycle_len / samp_len;
    switch (wave_header.sample_width) {
        case 8:
            intercpy((size8_t*)content, (size8_t*)dst, wave_header.data_size, interval);
            break;
        case 16:
            intercpy((size16_t*)content, (size16_t*)dst, wave_header.data_size/2, interval);
            break;
        case 32:
            intercpy((size32_t*)content, (size32_t*)dst, wave_header.data_size/4, interval);
            break;
        default:
            const char* err_msg = ("width of sample detected is not in the range: " + to_string(wave_header.sample_width)).c_str();
            throw UnreadableException(err_msg);
    }
    return;
}

bool BaseWave::is_stereo() const{
    return (wave_header.channels == 2);
}

// covernt stereo to mono
BaseWave& BaseWave::stereo2mono(){
    if (wave_header.channels != 2) {
        throw UnreadableException("wav to be converted is not a stereo");
    }
    BaseWave* mono = new BaseWave(*this);
    mono->update_channels_num(1);
    char* mono_content = new char[mono->wave_header.data_size];
    interleaved_copy(mono_content, wave_header.data_size, wave_header.sample_bytes, mono->wave_header.sample_bytes);
    mono->content = mono_content;
    return *mono;
    
}

// decrease the rate of sampling
void BaseWave::downsample(const uint new_samp_rate){
    uint shrink_rate = uint(wave_header.sample_rate / new_samp_rate);
    uint size_aft_shrink = wave_header.data_size / shrink_rate;
    char* samples = new char[size_aft_shrink];
    uint byte_distance = wave_header.sample_bytes * shrink_rate;  // number of bytes filled in two coming continuous frames
    
    if (shrink_rate > 1){
        // only copies N continuous bytes in M bytes while M/N equals to shrink_rate
        interleaved_copy(samples, wave_header.data_size, byte_distance, wave_header.sample_bytes);
        set_data_size(size_aft_shrink);
        set_sample_rate(new_samp_rate);
        delete [] content;  // delete space allocated
        content = samples;
	}
	else{
		if (shrink_rate < 1){
			cout << "sampling rate is " << wave_header.sample_rate << ", which is less than " << new_samp_rate << endl;
		}
	}
    return;
        
}

// return the name of a clip with index
const char* BaseWave::get_clip_name(uint index){
    string filename_str(filename);
    filename_str.insert(filename_str.length()-SUFFIX_LENGTH, INDEX_SEP+to_string(index));
	unsigned long name_length = filename_str.length() + 1;
	char* clip_name = new char[name_length];
    strncpy(clip_name, filename_str.c_str(), name_length);
    return clip_name;
}

// push a clip begins with clip_begining_byte and ends with clip_ending_byte to a vector
// meanwhile, name it with its index in the vector
void BaseWave::push_clip(vector<BaseWave&> clips_vec, const uint clip_begining_byte, const uint clip_ending_byte){
    const char* clip_name = get_clip_name(clips_vec.size);  // starts with 0
    BaseWave* clip = extract(clip_begining_byte, clip_ending_byte);     // copy the content to create a clip
    clip->set_filename(clip_name);
    clips_vec.push(clip)
    delete[] clip_name;
}


// split wav into clips if its duration was over the max_duration
vector<BaseWave*>& BaseWave::truncate(const uint max_duration, vector<BaseWave*>& clips_vec){
    clips_vec.clear();
    const uint max_clip_bytes = sec2byte(max_duration);
    if (max_clip_bytes > wave_header.data_size) {
        clips_vec.push_back(this);    // no need to truncate, return its self
    }else{
        uint clip_begining_byte = 0;
        uint clip_ending_byte = max_clip_bytes;
        while (clip_ending_byte < wave_header.data_size) {      // if the last clip split has not reached the ending of the file
            push_clip(clips_vec, clip_begining_byte, clip_ending_byte);
            clip_begining_byte = clip_ending_byte;
            clip_ending_byte = clip_begining_byte + max_clip_bytes;
        }
        push_clip(clips_vec, clip_begining_byte, wave_header.data_size);

    }
    return clips_vec;
};

// truncate but make sure no voice were to be splited
vector<BaseWave*>& BaseWave::smart_truncate(const uint max_duration, vector<BaseWave*>& clips_vec, const float window, const float threshold, const float offset){
    clips_vec.clear();
    
    const uint max_clip_bytes = sec2byte(max_duration);
    const uint bytes_in_window = sec2byte(window);
    
    if (max_clip_bytes >= wave_header.data_size) {
        clips_vec.push_back(this);    // no need to truncate, return its self
    }else{
        uint clip_begining_byte = 0;
        uint clip_ending_byte = max_clip_bytes;
        while (clip_ending_byte < wave_header.data_size) {      // if the last clip split has not reached the ending of the file
            
            // lookin for the ending with no voice included
            uint ending_offset = 0;
            float tuned_threshold = threshold;
            while (true) {
                uint window_begining_byte = clip_ending_byte - bytes_in_window - ending_offset;
                if (window_begining_byte <= clip_begining_byte) {  // failed to find an avaible ending with this threshold
                    tuned_threshold *= 2;  // loosen the threshold
                    ending_offset = 0;     // start from begining
                }
                
                /* evaluates the energy in a period and executes truncation if the period was thought to be a mute */
                if (get_samples_avg(window_begining_byte, bytes_in_window) > tuned_threshold) {   // if the average value of the window was under the threshold
                    ending_offset += sec2byte(offset);    // shifting the potential ending
                }else{
                    clip_ending_byte -= ending_offset;  // the ending was found
                    break;
                }
            }

            push_clip(clips_vec, clip_begining_byte, clip_ending_byte);
            
            clip_begining_byte = clip_ending_byte;
            clip_ending_byte = clip_begining_byte + max_clip_bytes;
        }
        push_clip(clips_vec, clip_begining_byte, wave_header.data_size);
    }
    return clips_vec;
}


vector<BaseWave*>& BaseWave::balanced_truncate(const uint max_duration, std::vector<BaseWave*>& clips_vec, const uint min_duration, const float window, const float threshold, const float offset){
    clips_vec.clear();
    
    const uint max_clip_bytes = sec2byte(max_duration);
    const uint min_clip_bytes = sec2byte(min_duration);
    const uint bytes_in_window = sec2byte(window);
    
    if (max_clip_bytes >= wave_header.data_size) {
        clips_vec.push_back(this);
    }else{
        queue<uint> breakpoints;
        equi_divide(breakpoints, min_duration, max_duration);
        uint clip_begining_byte = 0;
        uint clip_ending_byte = breakpoints.front();
        breakpoints.pop();
        
        float l_threshold = threshold;
        float r_threshold = threshold;
        
        while (!breakpoints.empty()) {
            /* shift related to window begining */
            int left_offset = -bytes_in_window;     // = window_begining_byte for left side
            int right_offset = 0;   //  = window_begining_byte for right side
            while (true) {
                if (get_samples_avg(clip_ending_byte + left_offset, bytes_in_window) < l_threshold) {
                    clip_ending_byte += left_offset + bytes_in_window;
                    break;
                }
                // not only look left, but also look right to find a proper breakpoint
                if (get_samples_avg(clip_ending_byte + right_offset, bytes_in_window) < r_threshold) {
                    clip_ending_byte += right_offset + bytes_in_window;
                    break;
                }
                
                // shift the offset
                left_offset -= sec2byte(offset);
                right_offset += sec2byte(offset);
                
                // loose the threshold if no avaible breakpoint was found
                if (clip_ending_byte + left_offset + bytes_in_window < clip_begining_byte + min_clip_bytes){      // new clip_ending shall not be under the minimum duration
                    l_threshold *= 2;
                    left_offset = -bytes_in_window;
                }
                if (clip_ending_byte + right_offset + bytes_in_window > clip_begining_byte +  max_clip_bytes ) {  // neither shall new clip_ending over the maximum duration
                    r_threshold *= 2;
                    right_offset = 0;
                }
            }
            
            push_clip(clips_vec, clip_begining_byte, clip_ending_byte);
            clip_begining_byte = clip_ending_byte;
            
            // TODO: what if the last clip was less than the minimum duration?
            do{
                if (breakpoints.empty()) {
                    break;
                }
                clip_ending_byte = breakpoints.front();
                breakpoints.pop();
            }while (clip_ending_byte < clip_begining_byte + min_clip_bytes);
            
            assert(clip_ending_byte > clip_begining_byte + min_clip_bytes);
            assert(clip_ending_byte < clip_begining_byte + max_clip_bytes);
            assert(clip_begining_byte < clip_ending_byte);
        }
        push_clip(clips_vec, clip_begining_byte, clip_ending_byte);
    }
    return clips_vec;
}

/*
 * find an available length to satisfy the requirement for duration
 * meanwhile to make the wave was splited with same size
 */
queue<uint>& BaseWave::equi_divide(queue<uint>& breakpoints, const uint min_duration, const uint max_duration){
    const float duration = get_duration();
    float avg_length = (max_duration + min_duration) / 2.0;
    
    float segment_num = duration / avg_length;
    uint length = 0;

    if (segment_num != int(segment_num)) {
        float ceiling_len = duration / (int(segment_num)-1);
        float floor_len = duration / (int(segment_num)+1);
        if (ceiling_len > max_duration ) {
            ceiling_len = max_duration;
        }
        if (floor_len < min_duration) {
            floor_len = min_duration;
        }
        /* choose the one with less error to avg_length */
        if ( avg_length-floor_len < ceiling_len-avg_length ){
            length = sec2byte(uint(floor_len));
        }else{
            length = sec2byte(uint(ceiling_len));
        }
        
    }else{
        length = sec2byte(uint(avg_length));
    }
    
    uint breakpoint = length;
    while (breakpoint < wave_header.data_size) {
        breakpoints.push(breakpoint);
        breakpoint += length;
    }
    breakpoints.push(wave_header.data_size);
    return breakpoints;
}

/*
 * get all segments filled with voice or silent
 */
vector<uint>& BaseWave::watershed(vector<uint>& segments, const uint start, const uint end, const float threshold, const float window, const float step, const bool flip){
    
}


// allocates a new wav with specified length
BaseWave* BaseWave::extract(const uint begining_byte, const uint ending_byte) const {
    if (ending_byte <= begining_byte) {
        throw InvalidOperation("specified ending is earlier than the begining");
    }
    if (ending_byte > wave_header.data_size || begining_byte > wave_header.data_size) {
        throw InvalidOperation("positions specified to be truncated exceeds the length");
    }
    
    uint clip_size = ending_byte - begining_byte;
    char* clip_content = new char[clip_size];
    memcpy(clip_content, content+begining_byte, clip_size);    //copy a slice of content to the clip
    
    BaseWave* clip = new BaseWave(*this);
    clip->set_data_size(clip_size);
    clip->set_content_ptr(clip_content);
    return clip;
}

BaseWave* BaseWave::extract(const float begining_sec, const float ending_sec) const {
    return extract(sec2byte(begining_sec), sec2byte(ending_sec));
}


