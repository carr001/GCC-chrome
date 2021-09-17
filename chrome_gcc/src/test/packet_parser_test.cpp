#include <iostream>
#include "packet_parser.h"
#include "packet.h"
#include "timestamp.h"
#include "quic_chromium_clock.h"
void packet_parser_test1(){
    QuicClock* clock = new QuicChromiumClock();

    const char* c;
    GCCDataPacket cp;
    cp.sequence_number = 1;
    cp.send_time = webrtc::Timestamp::Micros(clock->Now().GetUint64());
    std::cout<<cp.send_time.GetUint64()<<std::endl;

    // std::cout<<cp.Serialize()<<std::endl;
    GCCDataPacketParser s;
    const char* p = cp.Serialize().c_str();
    std::cout<<p<<std::endl;

    GCCDataPacket packet = GCCDataPacket();
    s.UnSerialize(p,&packet);
    std::cout<<packet.Serialize()<<std::endl;
    std::cout<<""<<std::endl;
}

void packet_parser_test2(){
    QuicClock* clock = new QuicChromiumClock();

    const char* c;
    GCCRRPacket cp;
    cp.start_time = webrtc::Timestamp::Micros(clock->Now().GetUint64());
    cp.end_time = webrtc::Timestamp::Micros(clock->Now().GetUint64());
    cp.packets_received_delta = 10;
    cp.packets_lost_delta = 10;
    std::cout<<cp.end_time.GetUint64()<<std::endl;

    // std::cout<<cp.Serialize()<<std::endl;
    GCCRRPacketParser par;
    std::string s = cp.Serialize();
    const char* p = s.c_str();
    std::cout<<p<<std::endl;

    GCCRRPacket packet = GCCRRPacket();
    par.UnSerialize(p,&packet);
    std::cout<<packet.Serialize()<<std::endl;
    std::cout<<""<<std::endl;
}

void packet_parser_test3(){
    QuicClock* clock = new QuicChromiumClock();

    const char* c;
    GCCREMBPacket cp;
    cp.bandwidth = webrtc::DataRate::BitsPerSec(1024);
    std::cout<<cp.bandwidth.GetUint64()<<std::endl;

    // std::cout<<cp.Serialize()<<std::endl;
    GCCREMBPacketParser par;
    std::string s = cp.Serialize();
    const char* p = s.c_str();
    std::cout<<p<<std::endl;

    GCCREMBPacket packet = GCCREMBPacket();
    par.UnSerialize(p,&packet);
    std::cout<<packet.Serialize()<<std::endl;
    std::cout<<""<<std::endl;
}

int main(int argc, char* argv[]){
    // packet_parser_test1();
    // packet_parser_test2();
    packet_parser_test3();
    // std::cout<<"test end"<<std::endl;
};

