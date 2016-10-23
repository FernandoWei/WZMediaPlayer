//
//  MediaLog.cpp
//  WZMediaPlayer
//
//  Created by Hongqiang.Wei on 16/10/23.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "MediaLog.hpp"

template <typename T>
void MediaLog::logImpl(const T& t){
    std::cout << t << std::endl;
}

template <typename T, typename... Args>
void MediaLog::logImpl(const T& t, Args... args){
    std::cout << t << " ";
    log(args...);
}

template <typename T, typename... Args>
void MediaLog::log(const T& t, Args... args){
    std::cout << toString();
    logImpl(t, args...);
}

std::string MediaLog::toString() const {
    return std::string("MediaLog");
}