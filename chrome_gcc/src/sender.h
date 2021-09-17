
#include <netinet/ip.h>
#include <iostream>

#include "src/quic/quic_chromium_clock.h"
#include "src/packet/packet_maker.h"
#include "src/packet/packet_parser.h"
#include "src/packet/packet.h"
#include "src/time/timestamp.h"
#include "src/google_cc/goog_cc_network_control.h"


class Sender{

class SenderDebuger{
public:
    SenderDebuger():
        packet_sent(0),
        rr_packet_received(0),
        remb_packet_received(0),
        pring_info_duration(webrtc::TimeDelta::Seconds(1)),
        clock(new QuicChromiumClock()),
        last_print_timestamp( webrtc::Timestamp::Micros(clock->Now().GetUint64()))
        {}
    void OnReceivePacket(SendControl send_control){
        if( webrtc::Timestamp::Micros(clock->Now().GetUint64()) > last_print_timestamp+pring_info_duration){
            std::cout<<" [SenderDebuger] "
            <<"Pacing Rate: "<<send_control.pacing_rate/1000.f<<" kbit/s "<<"|| "
            <<"Packets sent: "<<packet_sent<<" "<<"|| "
            <<"RR Packet received: "<<rr_packet_received<<" "<<"|| "
            <<"REMB Packet received: "<<remb_packet_received<<" "
            <<std::endl;
            last_print_timestamp = webrtc::Timestamp::Micros(clock->Now().GetUint64());
            packet_sent=0;
            rr_packet_received=0;
            remb_packet_received=0;
        }
    };
    const webrtc::TimeDelta pring_info_duration;
    QuicClock* clock ;
    webrtc::Timestamp last_print_timestamp;
    uint64_t packet_sent;
    uint64_t rr_packet_received;
    uint64_t remb_packet_received;
};
public:
    struct DebugState {
        explicit DebugState(const Sender& sender);
        DebugState(const DebugState& state);
        SendControl send_control;
        uint64_t receiver_limit;
        uint64_t packets_lost_delta = 0;
        uint64_t packets_received_delta = 0;
    };
    using DebugState = struct DebugState;
public:
    Sender(webrtc::NetworkControllerConfig config);
    ~Sender(){};

    void SendPacket();
    void ReceivePacket();

    std::string GetDebugState() const;
    DebugState ExportDebugState() const;
private:
    void OnRRPacketReceived();
    void OnREMBPacketReceived();
    void SetNextSendTime();
    void MakePacketAndSendFromBuf();
    bool IsTimeToSendNextPacket();
    void Bind();
    void OnPacketReceived();

    struct sockaddr_in local_addr_;
    struct sockaddr_in remote_addr_;
    int sender_socket_;
    char send_buf_[SEND_BUF_SIZE] = {'\0'};
    char receive_buf_[RECEIVE_BUF_SIZE] = {'\0'};

    GCCDataPacketMaker data_packet_maker_;
    GCCRRPacketParser rr_packet_parser_;
    GCCREMBPacketParser remb_packet_parser_;
    const QuicClock* clock_;
    webrtc::GoogCcNetworkController* gcc_controler_;
    GCCRRPacket recent_rr_packet_;

    uint64_t least_unacked_packet_number_;
    uint64_t next_send_number_;
    SendControl send_control_;
    SenderDebuger* debuger_;
};

std::ostream& operator<<(std::ostream& os,const Sender::DebugState& state);


