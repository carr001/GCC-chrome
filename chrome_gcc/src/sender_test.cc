
#include "src/sender.h"
#include "src/common/defs.h"
#include "src/common/network_control.h"
#include "src/quic/quic_chromium_clock.h"
#include "glog/logging.h"

void sender_test1(){
    QuicClock* clock = new QuicChromiumClock();

    webrtc::NetworkControllerConfig net_controler;
    net_controler.constraints.at_time = webrtc::Timestamp::Micros(clock->Now().GetUint64());
    net_controler.constraints.min_data_rate = webrtc::DataRate::BitsPerSec(5000);
    net_controler.constraints.max_data_rate = webrtc::DataRate::BitsPerSec(1000000000);
    net_controler.stream_based_config.requests_alr_probing=false;
    net_controler.stream_based_config.at_time = webrtc::Timestamp::Micros(clock->Now().GetUint64());
   
    Sender* sender = new Sender(net_controler);
    for(int i=0;i<INTMAX_MAX;i++){
        sender->SendPacket();
        sender->ReceivePacket();
    }
}

void sender_test2(){
    LOG(INFO)<<"Go";
    std::cout<<"start to receive data"<<std::endl;
}

void sender_test3(){
    QuicClock* clock = new QuicChromiumClock();

    webrtc::NetworkControllerConfig net_controler;
    net_controler.constraints.at_time = webrtc::Timestamp::Micros(clock->Now().GetUint64());
    net_controler.constraints.min_data_rate = webrtc::DataRate::BitsPerSec(5000);
    net_controler.constraints.max_data_rate = webrtc::DataRate::BitsPerSec(1000000000);
    net_controler.stream_based_config.requests_alr_probing=false;
    net_controler.stream_based_config.at_time = webrtc::Timestamp::Micros(clock->Now().GetUint64());
   
    Sender* sender =  new Sender(net_controler);
    std::cout<<" [SimpleInfo] Start sender test"<<std::endl;
    QuicTime::Delta duaration = QuicTime::Delta::FromSeconds(350);
    QuicTime::Delta duaration2 =  QuicTime::Delta::FromSeconds(0);
    QuicTime timestamp = clock->Now();
    while (clock->Now()<timestamp+duaration){
        sender->SendPacket();
        sender->ReceivePacket();
        if(clock->Now()>timestamp+duaration2){// serve as timer
            std::cout<<" [CountDown] "<<(duaration-duaration2).Getint64()/1000000<<"s left.."<<std::endl;
            duaration2 = duaration2+ QuicTime::Delta::FromSeconds(1);
        }
    }
    std::cout<<" [SimpleInfo] End sender test"<<std::endl;
}

int main(int argc,char* argv[]){
    // sender_test1();
    // sender_test2();
    sender_test3();
}


