#include "src/receiver.h"

#include <iostream>

#include "src/quic/quic_chromium_clock.h"
#include "src/quic/quic_time.h"
#include "glog/logging.h"

void receiver_test1(){
    QuicClock* clock = new QuicChromiumClock();

    webrtc::NetworkControllerConfig net_controler;
    net_controler.constraints.at_time = webrtc::Timestamp::Micros(clock->Now().GetUint64());
    net_controler.constraints.min_data_rate = webrtc::DataRate::BitsPerSec(5000);
    net_controler.constraints.max_data_rate = webrtc::DataRate::BitsPerSec(1000000000);
    net_controler.stream_based_config.requests_alr_probing=false;
    net_controler.stream_based_config.at_time = webrtc::Timestamp::Micros(clock->Now().GetUint64());
   
    Receiver* receiver =  new Receiver(net_controler);
    std::cout<<"start to receive data"<<std::endl;

    while (1){
        receiver->ReceivePacket();
    }
}

void receiver_test2(){
    QuicClock* clock = new QuicChromiumClock();

    webrtc::NetworkControllerConfig net_controler;
    net_controler.constraints.at_time = webrtc::Timestamp::Micros(clock->Now().GetUint64());
    net_controler.constraints.min_data_rate = webrtc::DataRate::BitsPerSec(5000);
    net_controler.constraints.max_data_rate = webrtc::DataRate::BitsPerSec(1000000000);
    net_controler.stream_based_config.requests_alr_probing=false;
    net_controler.stream_based_config.at_time = webrtc::Timestamp::Micros(clock->Now().GetUint64());
   
    Receiver* receiver =  new Receiver(net_controler);
    std::cout<<" [SimpleInfo] Start receiver test"<<std::endl;
    std::cout<<" [SimpleInfo] Start to receive data"<<std::endl;
    QuicTime::Delta duaration = QuicTime::Delta::FromSeconds(10);
    QuicTime::Delta duaration2 =  QuicTime::Delta::FromSeconds(0);
    QuicTime timestamp = clock->Now();
    while (clock->Now()<timestamp+duaration){
        receiver->ReceivePacket();
        if(clock->Now()>timestamp+duaration2){
            std::cout<<" [CountDown] "<<(duaration-duaration2).Getint64()/1000000<<"s left.."<<std::endl;
            duaration2 = duaration2+ QuicTime::Delta::FromSeconds(1);
        }
    }
    std::cout<<" [SimpleInfo] End receiver test"<<std::endl;
}


int main(int argc,char* argv[]){
    // receiver_test1();
    receiver_test2();
}

