//
//  utility.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/12/2.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef utility_hpp
#define utility_hpp

#include "WZObject.hpp"

extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
}

#include <chrono>
#include <thread>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <array>
#include <list>

namespace WZ {
    namespace Utility {
        
#define NUM_TWENTY (20)
        
#define NON_FIRST_BUFFER_SECONDS (3)
#define VIDEO_PKT_COUNT_PER_SECOND (25)
#define AUDIO_PKT_COUNT_PER_SECOND (50)
        
#define checkError(err, info) \
if (noErr != err){ \
std::cout << info << " failed with error: " << static_cast<int>(err) << std::endl; \
return; \
} \

#define checkErrorWithReturn(err, info, result) \
if (noErr != err){ \
std::cout << info << " failed with error: " << static_cast<int>(err) << std::endl; \
return result; \
} \

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
        
        enum class DiscardFrameType {
            DISCARD_NONE,
            DISCARD_DEFAULT
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
        
        NALType getNALType(uint8_t* data, int index);
        uint32_t getNALSize(uint8_t* data, int index);
        
    }
}


#endif /* utility_hpp */
