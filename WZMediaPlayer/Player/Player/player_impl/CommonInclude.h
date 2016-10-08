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
    TRY_AGAIN_LATER
};

#endif /* CommonInclude_h */
