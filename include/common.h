//  common.h
//  audio
//
//  Created by Xiao Yang on 16/4/15.
//  Copyright (c) 2016年 Young. All rights reserved.
//

#ifndef audio_common_h
#define audio_common_h

#include <vector>


#define LOG_ERROR(msg) { std::cout << "Error: " << msg << std::endl; }
#define LOG_WARN(msg) { std::cout << "Warning: " << msg << std::endl; }

typedef char   size8_t;
typedef short  size16_t;
typedef int    size32_t;
typedef unsigned int uint;


// set values for flags
template <typename T>
void set_flag_vals(T* flag, std::string vals) {
    for (int i=0; i<vals.length() ; i++) {
        flag[i] = vals.at(i);
    }
    return;
}

template <typename T>
void pack(T* content, std::vector<float>& samples, uint sample_num, uint start){
    samples.clear();
    for (int i=start; i<sample_num+start; i++) {
        samples.push_back(*(content+i));
    }
    return;
}

template <typename T>
const float avg_pack(T* content, uint sample_num, uint start) {
    float sum = 0;
    for (int i=start; i<sample_num+start; i++) {
        sum += abs(*(content+i));
    }
    return sum/sample_num;
}

template <typename T>
void intercpy(T* src, T* dst, const uint samp_size, const uint interval){
    for (uint i=0, j=0; i<samp_size; i+=interval) {
        dst[j++] = src[i];
    }
    return;
}

// to catch error caused by platform changed
void validate_typesize();


#endif
