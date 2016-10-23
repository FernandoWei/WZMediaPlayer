//
//  MediaLog.hpp
//  WZMediaPlayer
//
//  Created by Hongqiang.Wei on 16/10/23.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef MediaLog_hpp
#define MediaLog_hpp

#include "CommonInclude.h"

class MediaLog {
    MediaLog(const MediaLog& log) = delete;
    MediaLog(MediaLog&& log) = delete;
    MediaLog& operator=(const MediaLog& log) = delete;
    MediaLog& operator=(MediaLog&& log) = delete;
    
public:
    MediaLog();
    virtual ~MediaLog();
    template <typename T, typename...Args>
    void log(const T& t, Args... args);
    
private:
    virtual std::string toString() const;
    template <typename T, typename... Args>
    void logImpl(const T& t, Args... args);
    template <typename T>
    void logImpl(const T& t);
};

#endif /* MediaLog_hpp */