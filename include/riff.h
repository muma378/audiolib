//  riff.h - it declares common data structures and methods used in processing Resource Interchange File Format(RIFF)
//  audio
//
//  Created by Xiao Yang on 16/7/4.
//  Copyright (c) 2016年 Young. All rights reserved.
//

#ifndef audio_riff_h
#define audio_riff_h

#include "common.h"

#define CHUNK_HEAD_SIZE 8


typedef struct riff_chunk{
    size8_t  ck_id[4];   // flag is always consist of 4 characters, such as 'data', 'slnt', 'file' and etc. we do only focus on flag 'data' here
    size32_t ck_size;      // to indicate the size of the following content
    size8_t* ck_data;   // pointer to the content of the flag
    struct riff_chunk* next_ck;  // linker to next flag, usually is set to nullptr
}riff_chunk_t;


typedef union {
    riff_chunk_t    chunk;
    char            buffer[CHUNK_HEAD_SIZE];
}chunk_buffer_t;

#endif