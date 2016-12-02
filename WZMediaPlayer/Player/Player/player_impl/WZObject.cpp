//
//  WZObject.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/12/1.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "WZObject.hpp"

namespace WZ {
    namespace Utility {
        
        MediaLog g_Log;
        
        void WZObject::log(const char* format, ...){
            va_list va;
            va_start(va, format);
            g_Log.log(std::shared_ptr<WZ::Utility::WZObject>(this), format, va);
            va_end(va);
        }
    }
}
