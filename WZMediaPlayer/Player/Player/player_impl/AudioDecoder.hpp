//
//  AudioDecoder.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef AudioDecoder_hpp
#define AudioDecoder_hpp

#include <stdio.h>
#include "MediaDecoder.hpp"

class AudioDecoder : public MediaDecoder {
public:
    AudioDecoder();
    ~AudioDecoder();
public:
    void virtual prepare();
    void virtual decode(AVPacket* pkt);
};

#endif /* AudioDecoder_hpp */
