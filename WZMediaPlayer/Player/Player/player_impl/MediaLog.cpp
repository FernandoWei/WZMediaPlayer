//
//  MediaLog.cpp
//  WZMediaPlayer
//
//  Created by Hongqiang.Wei on 16/10/23.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "MediaLog.hpp"
#include <iomanip>

MediaLog::MediaLog():mMutexForLog(){}

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
    std::lock_guard<std::mutex> guard(mMutexForLog);
    printCurrentTimeAndClassname();
    logImpl(t, args...);
}

std::string MediaLog::toString() const {
    return std::string("MediaLog");
}

void MediaLog::printCurrentTimeAndClassname() const {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    auto now_ms_part = now_ms - std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    struct std::tm* tm = std::localtime(&now_time_t);
    std::cout << tm->tm_year + 1900 << "-";
    std::cout << std::setfill('0') << std::setw(2) << tm->tm_mon + 1 << "-";
    std::cout << std::setfill('0') << std::setw(2) << tm->tm_mday << " ";
    std::cout << std::setfill('0') << std::setw(2) << tm->tm_hour << ":";
    std::cout << std::setfill('0') << std::setw(2) << tm->tm_min << ":";
    std::cout << std::setfill('0') << std::setw(2) << tm->tm_sec << ".";
    std::cout << std::setfill('0') << std::setw(3) << now_ms_part.count();
    std::cout << " [" << toString() << "]: ";
}
