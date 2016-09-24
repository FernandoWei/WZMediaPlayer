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
    VideoDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData);
    ~VideoDecoder();
    
public:
    void virtual prepare();
    PlayerState virtual decode(AVPacket* pkt);
    void flush();
    
};

#endif /* VideoDecoder_hpp */
