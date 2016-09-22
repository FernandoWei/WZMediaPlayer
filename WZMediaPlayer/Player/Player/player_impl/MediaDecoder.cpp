//
//  MediaDecoder.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "MediaDecoder.hpp"


MediaDecoder::MediaDecoder():
mPrepared(false)
{
    ;
}

void MediaDecoder::start(){
    if (!mPrepared){
        prepare();
    }
    
    while (true){
        AVPacket pkt;
        while (true){
            switch (dequeuePacket(&pkt)) {
                case PlayerState::TRY_AGAIN_LATER:
                    break;
                default:
                    break;
            }
        }

    }
    
}