#include <iostream>
#include "src/packet/packet_maker.h"
#include "src/quic/quic_chromium_clock.h"

void packet_maker_test1(){
    QuicChromiumClock clock;
    QuicTime a = clock.Now();
    std::cout<<a<<std::endl;
}
void packet_maker_test2(){
    GCCDataPacketMaker packet_maker;
    QuicChromiumClock clock;
    for(int i=0;i<30;i++){
        GCCDataPacket p = packet_maker.MakePacket();
        p.send_time = webrtc::Timestamp::Micros(clock.Now().GetUint64()) ;
        std::cout<<p.Serialize()<<std::endl;
        // std::cout<<p->packet_number<<" "<<p->send_time<<std::endl;
    }
}

int main(int argc, char* argv[]){
    // packet_maker_test1();    
    packet_maker_test2();    
}


