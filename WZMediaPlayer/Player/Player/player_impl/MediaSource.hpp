//
//  MediaSource.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef MediaSource_hpp
#define MediaSource_hpp

#include "CommonInclude.h"

class MediaSource {
public:
    MediaSource() = delete;
    MediaSource(const MediaSource& source) = delete;
    MediaSource(MediaSource&& source) = delete;
    MediaSource& operator=(const MediaSource& source) = delete;
    MediaSource& operator=(MediaSource&& source) = delete;
    explicit MediaSource(const std::string& url);
    ~MediaSource();
    
public:
    PlayerState prepare();
    void stop();
    PlayerState read(AVPacket* packet);
    int getVideoStreamIndex() const;
    int getAudioStreamIndex() const;
    AVStream* getAudioStream() const;
    AVStream* getVideoStream() const;
    
    
private:
    void init();
    PlayerState open();
    void close();
    
    static int didStopped(void* thiz);
    
private:
    int mVideoStreamIndex;
    int mAudioStreamIndex;
    AVStream* mVideoStream;
    AVStream* mAudioStream;
    AVFormatContext* mFormatCtx;
    
    std::string mUrl;
    std::atomic_bool mStopped;
};

#endif /* MediaSource_hpp */
