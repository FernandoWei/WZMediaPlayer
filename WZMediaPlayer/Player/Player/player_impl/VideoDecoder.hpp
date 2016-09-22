//
//  VideoDecoder.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef VideoDecoder_hpp
#define VideoDecoder_hpp

#include <stdio.h>
#include "MediaDecoder.hpp"

class VideoDecoder : public MediaDecoder {
    
public:
    VideoDecoder();
    ~VideoDecoder();
    
public:
    void virtual prepare();
    void virtual decode(AVPacket* pkt);
    
};

#endif /* VideoDecoder_hpp */