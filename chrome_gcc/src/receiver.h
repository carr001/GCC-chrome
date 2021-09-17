#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>

#include "src/packet/packet_parser.h"
#include "src/packet/packet_maker.h"
#include "src/quic/quic_clock.h"
#include "src/quic/quic_chromium_clock.h"
#include "src/google_cc/goog_cc_network_control.h"
#include "src/google_cc/receiver_loss_collection.h"
#include "src/common/data_rate.h"

class Receiver{

class ReceiverDebuger{
public:
    ReceiverDebuger():
        packet_received(0),
        pring_info_duration(webrtc::TimeDelta::Seconds(1)),
        clock(new QuicChromiumClock()),
        last_print_timestamp( webrtc::Timestamp::Micros(clock->Now().GetUint64()))
        {}
    void PrintInfo(){
        if( webrtc::Timestamp::Micros(clock->Now().GetUint64()) > last_print_timestamp+pring_info_duration){
            double reveive_rate = (packet_received*DEFAULT_PACKET_SIZE*8);
            std::cout<<" [ReceiverDebuger] "
                <<packet_received
                <<" packets received in 1s. "
                <<"Receive data rate: "
                <<reveive_rate/1000.f
                <<" kb/s"
                <<std::endl;
            packet_received = 0;
            last_print_timestamp = webrtc::Timestamp::Micros(clock->Now().GetUint64());
        }
    };
    const webrtc::TimeDelta pring_info_duration;
    QuicClock* clock ;
    webrtc::Timestamp last_print_timestamp;
    uint64_t packet_received;
};



public:
    Receiver(webrtc::NetworkControllerConfig config);
    ~Receiver();

    void ReceivePacket();
    
private:
    void OnLossReportGenerated(webrtc::LossReportUpdate);
    void OnDelayBaseBweUpdated(webrtc::DelayBasedBwe::Result);
    void OnDataPacketReceived();
    void OnPacketReceived();
    void SetPacketToBuf(Packet*);
    void SendPacketFromBuf();
    void Bind();
    //debug
    void PrintReceiveRate();

    struct sockaddr_in local_addr_;
    struct sockaddr_in remote_addr_;
    int local_socket_;
    char receive_buf_[RECEIVE_BUF_SIZE] = {'\0'};
    char send_buf_[SEND_BUF_SIZE] = {'\0'};

    QuicClock* clock_;
    webrtc::GoogCcNetworkController* gcc_controler_;
    webrtc::ReceiverLossCollection* loss_collection_;
    // GCCRRPacketMaker RR_packet_maker_;
    // GCCREMBPacketMaker REMB_packet_maker_;
    GCCDataPacketParser data_packet_parser_;
    webrtc::DataRate last_remb_rate_;

    // for print info
    ReceiverDebuger receiver_debuger_;
    uint64_t last_ack_rate_;
    // size_t packets_received_in_one_second_;
    // // size_t totoal_bytes_received_in_one_second;
    // QuicTime last_print_timestamp_;
    // QuicTime::Delta print_receive_info_duration_;

};





