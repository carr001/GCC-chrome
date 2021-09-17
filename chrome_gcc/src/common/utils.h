#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "src/common/defs.h"

namespace rtc{
template< typename T, typename T1>
T SafeClamp(T value,T1 min,T1 max){
    if(value>max){
        value = max;
    }
    if(value<min){
        value = min;
    }
    return value;
};
// #define SafeClamp(v,min,max)  \
//     (if(value>max){           \
//         value = max;          \
//     }                         \
//     if(value<min){            \
//         value = min;          \
//     })                        
} // namespace rtc


std::string number_to_fix_len_string(uint64_t number,int len);

void pause_at_func(std::string s);

std::string packet_type_to_string(PACKET_TYPE t);
PACKET_TYPE string_to_packet_type(std::string s);



#endif  // UTILS_H
