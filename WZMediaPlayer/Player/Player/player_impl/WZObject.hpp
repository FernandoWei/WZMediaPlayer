//
//  WZObject.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/12/1.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef WZObject_hpp
#define WZObject_hpp

#include "MediaLog.hpp"
#include <string>

namespace WZ {
    namespace Utility {
        class WZObject {
        public:
            WZObject() = default;
            virtual ~WZObject(){}
        public:
            inline virtual std::string getClassName() const {return nullptr;}
            
        public:
            void log(const char* format, ...);
        };
    }
}


#endif /* WZObject_hpp */
