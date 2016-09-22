//
//  CommonInclude.h
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef CommonInclude_h
#define CommonInclude_h

extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
    //#include <libavutil/time.h>
    //#include <libswresample/swresample.h>
}

enum class PlayerState {
    OK,
    ERROR,
    END_OF_FILE,
    TRY_AGAIN_LATER
};

#endif /* CommonInclude_h */
