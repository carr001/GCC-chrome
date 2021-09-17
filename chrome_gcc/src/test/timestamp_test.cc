#include <iostream>
#include "timestamp.h"
#include "quic_chromium_clock.h"

void timestamp_test1(){
    webrtc::Timestamp a = webrtc::Timestamp::Micros(100);
    std::cout<< webrtc::ToString(a) <<std::endl;    
}

void timestamp_test2(){
    QuicClock* clock = new QuicChromiumClock();
    webrtc::Timestamp a = webrtc::Timestamp::Micros(clock->Now().GetUint64());
    std::cout<< webrtc::ToString(a) <<std::endl;    
    std::cout<< a.GetUint64() <<std::endl;    
}

int main(int argc,char* argv[]){
    timestamp_test1();
    timestamp_test2();
}

