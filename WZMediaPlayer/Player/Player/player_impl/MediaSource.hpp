//
//  MediaSource.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef MediaSource_hpp
#define MediaSource_hpp

extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
    //#include <libavutil/time.h>
    //#include <libswresample/swresample.h>
}

#include <string>

extern enum class PlayerState;

class MediaSource {
public:
    MediaSource() = delete;
    explicit MediaSource(const std::string& url);
    ~MediaSource(){}
    
public:
    void prepare();
    PlayerState read(AVPacket* packet);
    int getVideoStreamIndex();
    int getAudioStreamIndex();
    
    
private:
    void open();
    
private:
    int mVideoStreamIndex;
    int mAudioStreamIndex;
    
    
    
    
};

#endif /* MediaSource_hpp */
