#include <iostream>

#include "quic_clock.h"
#include "quic_chromium_clock.h"



void quic_chromium_clock_test1(){
    QuicClock* clock = new QuicChromiumClock();
    std::cout<< clock->Now().GetUint64() <<std::endl;    
    
}


int main(int argc,char* argv[]){
    quic_chromium_clock_test1();
}

