//
//  CommonInclude.h
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef CommonInclude_h
#define CommonInclude_h

#include <chrono>
#include <thread>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <array>
#include <list>

extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
}

#define NUM_TWENTY (20)

#define NON_FIRST_BUFFER_SECONDS (3)
#define VIDEO_PKT_COUNT_PER_SECOND (25)
#define AUDIO_PKT_COUNT_PER_SECOND (50)

enum class PlayerState {
    OK,
    ERROR,
    STOPPED,
    END_OF_FILE,
    TRY_AGAIN_LATER,
    IGNORE,
    NEED_MORE_DATA,
    INVALID_DECODER_SESSION
};

enum class NALType{
    NAL_Slice,
    NAL_PA,
    NAL_PB,
    NAL_PC,
    NAL_IDR_Slice,
    NAL_SEI,
    NAL_SPS,
    NAL_PPS,
    NAL_AUD,
    NAL_ERR
};

NALType getNALType(uint8_t* data, int index){
    NALType type;
    switch(data[index] & 0x1F){
        case 0x01:{
            type = NALType::NAL_Slice;
            break;
        }case 0x02:{
            type = NALType::NAL_PA;
            break;
        }case 0x03:{
            type = NALType::NAL_PB;
            break;
        }case 0x04:{
            type = NALType::NAL_PC;
            break;
        }case 0x05:{
            type = NALType::NAL_IDR_Slice;
            break;
        }case 0x06:{
            type = NALType::NAL_SEI;
            break;
        }case 0x07:{
            type = NALType::NAL_SPS;
            break;
        }case 0x08:{
            type = NALType::NAL_PPS;
            break;
        }case 0x09:{
            type = NALType::NAL_AUD;
            break;
        }default:{
            type = NALType::NAL_ERR;
            break;
        }
    }
    return type;
}

uint32_t getNALSize(uint8_t* data, int index){
    return data[index - 1]
    + (data[index - 2] << 8)
    + (data[index - 3] << 16)
    + (data[index - 4] << 24);
}

#endif /* CommonInclude_h */
