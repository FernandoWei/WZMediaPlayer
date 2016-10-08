//
//  VideoDecoder.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef VideoDecoder_hpp
#define VideoDecoder_hpp

#include "MediaDecoder.hpp"
#include <videotoolbox/VideoToolbox.h>

class VideoDecoder : public MediaDecoder {
    
public:
    VideoDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData, AVStream* stream);
    ~VideoDecoder();
    
public:
    void virtual prepare();
    PlayerState virtual decode(AVPacket* pkt);
    void flush();
    
private:
    void prepareHWDecoder();
    void prepareCodecData();
    void resetHWDecoder();
    void duplicatePacket();
    void synchronize();
    
private:
    std::vector<uint8_t> mSPS;
    std::vector<uint8_t> mPPS;
    
    VTDecompressionSessionRef mDecoderSession;
    CVPixelBufferRef mPixelBuffer;
};

#endif /* VideoDecoder_hpp */
