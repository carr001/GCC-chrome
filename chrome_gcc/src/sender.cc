#include "src/sender.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

#include "src/common/defs.h"
#include "src/common/utils.h"
#include "src/common/network_types.h"
#include "src/common/data_rate.h"
#include "src/common/network_control.h"
#include "src/quic/quic_time.h"
#include "src/quic/quic_chromium_clock.h"
#include "glog/logging.h"

Sender::Sender(webrtc::NetworkControllerConfig config)
    :clock_(new QuicChromiumClock()),
    debuger_(new SenderDebuger()),
    gcc_controler_(new webrtc::GoogCcNetworkController(config)),
    recent_rr_packet_(GCCRRPacket())
    {
        remote_addr_.sin_family      = AF_INET;
        remote_addr_.sin_port        = htons(RECEIVERPORT);
        remote_addr_.sin_addr.s_addr = htonl(RECEIVER_IP);

        local_addr_.sin_family      = AF_INET;
        local_addr_.sin_port        = htons(SENDERPORT);
        local_addr_.sin_addr.s_addr = htonl(SENDER_IP);
        Bind();
    }

void Sender::Bind(){
    sender_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    int num  =bind(sender_socket_, (struct sockaddr*)&local_addr_, sizeof(struct sockaddr));
    if(num<0){
        pause_at_func(__FUNCTION__);
    }
    std::cout<<"bind port "<<SENDERPORT<<" succeed."<<std::endl;
}

void Sender::SendPacket(){
    if(IsTimeToSendNextPacket()){
        // QuicTime sta = clock_->Now();
        MakePacketAndSendFromBuf();
        // std::cout<<clock_->Now()-sta<<std::endl;
    }
}

void Sender::MakePacketAndSendFromBuf(){
    // SerializedPacket packet(*packet_maker_.MakePacket());
    GCCDataPacket packet = data_packet_maker_.MakePacket();
    packet.send_time = webrtc::Timestamp::Micros(clock_->Now().GetUint64()) ;
    const std::string to_buf= packet.Serialize();

    int num = sendto(sender_socket_, to_buf.c_str(),to_buf.length(),MSG_DONTWAIT,(struct sockaddr *)&remote_addr_,sizeof(remote_addr_));
    // std::cout<< to_buf<<std::endl;
    // std::cout<< strlen(to_buf)<<std::endl;
    if(num<0){
        // to make sure the logic right, the send operation be done
        pause_at_func(__FUNCTION__);
    }
    debuger_->packet_sent ++;
}

void Sender::SetNextSendTime(){
    if(send_control_.pacing_rate>0){
        QuicTime::Delta duration_between_next_send = 
            QuicTime::Delta::FromMicroseconds(uint64_t(DEFAULT_PACKET_SIZE*uint64_t(8)*ONE_SECOND/double(send_control_.pacing_rate))) ;
        // std::cout<<"do="<<DEFAULT_PACKET_SIZE*BITS_PER_BYTE*ONE_SECOND<<
        //     " no="<<double(send_control_.pacing_rate)<<std::endl;
        // std::cout<<uint64_t(DEFAULT_PACKET_SIZE*BITS_PER_BYTE*ONE_SECOND/double(send_control_.pacing_rate))<<std::endl;
        QuicTime now = clock_->Now();
        send_control_.next_send_timestamp   = now + duration_between_next_send;
        // std::cout<<"now:"<<now<<" dura:"<<duration_between_next_send<<std::endl;
    }else{
        pause_at_func(__FUNCTION__);
    }
}

bool Sender::IsTimeToSendNextPacket(){
    if(clock_->Now()>send_control_.next_send_timestamp){
        // std::cout <<unacked_packets_->bytes_in_flight()<<std::endl;
        // std::cout <<packets_lost_->size()<<std::endl;
            SetNextSendTime();
            return true;
       }
    return false;
}

void Sender::ReceivePacket(){
    memset(receive_buf_,'\0',SEND_BUF_SIZE);
    socklen_t addrlen = sizeof(struct sockaddr_in);
    int num =recvfrom(sender_socket_,receive_buf_,SEND_BUF_SIZE,MSG_DONTWAIT,(struct sockaddr*)&remote_addr_,&addrlen); 
    if (num > 0){
        OnPacketReceived();
    }
    debuger_->OnReceivePacket(send_control_);
}

void Sender::OnPacketReceived(){
    PACKET_TYPE type = PacketParser::GetType(receive_buf_);
    switch (type)
    {
    case PACKET_TYPE::RR:
        OnRRPacketReceived();
        debuger_->rr_packet_received++;
        break;
    case PACKET_TYPE::REMB:
        OnREMBPacketReceived();
        debuger_->remb_packet_received++;
        break;
    default:
        break;
    }
    send_control_.pacing_rate = gcc_controler_->GetSenderSideDataRate().GetUint64();
    if(send_control_.pacing_rate<MIN_PACING_RATE){
        send_control_.pacing_rate = MIN_PACING_RATE;
    }
    LOG(INFO)<<GetDebugState();
}

void Sender::OnRRPacketReceived(){
    recent_rr_packet_ = GCCRRPacket();
    rr_packet_parser_.UnSerialize(receive_buf_,&recent_rr_packet_);
    
    webrtc::TransportLossReport msg;
    msg.start_time = recent_rr_packet_.start_time;
    msg.end_time = recent_rr_packet_.end_time;
    msg.packets_lost_delta = recent_rr_packet_.packets_lost_delta;
    msg.packets_received_delta = recent_rr_packet_.packets_received_delta;
    msg.receive_time = webrtc::Timestamp::Micros(clock_->Now().GetUint64());
    gcc_controler_->OnTransportLossReport(msg);
}

void Sender::OnREMBPacketReceived(){
    GCCREMBPacket remb_packet = GCCREMBPacket();
    remb_packet_parser_.UnSerialize(receive_buf_,&remb_packet);

    webrtc::RemoteBitrateReport msg;
    msg.bandwidth = remb_packet.bandwidth;
    msg.receive_time = webrtc::Timestamp::Micros(clock_->Now().GetUint64());
    gcc_controler_->OnRemoteBitrateReport(msg);
}

Sender::DebugState Sender::ExportDebugState() const {
  return DebugState(*this);
}

Sender::DebugState::DebugState(const Sender& sender)
    : send_control(sender.send_control_),
    receiver_limit(sender.gcc_controler_->GetReceiverLimit().GetUint64()),
    packets_lost_delta(sender.recent_rr_packet_.packets_lost_delta),
    packets_received_delta(sender.recent_rr_packet_.packets_received_delta)
    {}

std::string Sender::GetDebugState() const {
  std::ostringstream stream;
  stream << ExportDebugState();
  return stream.str();
}

std::ostream& operator<<(std::ostream& os, const Sender::DebugState& state) {
  if(state.packets_received_delta!=0){
    os << "Pacing_rate: " << state.send_control.pacing_rate/1000.f<< " kbps, "
        << "Receiver_limit: " << state.receiver_limit/1000.f<< " kbps, "
        << "Packets_lost_delta: " << state.packets_lost_delta<< ", "
        << "Packets_received_delta: " << state.packets_received_delta<<", "
        << "Loss_rate: " << double(state.packets_lost_delta)/(state.packets_lost_delta+state.packets_received_delta)<<" %";
  }else{
    os << "Pacing_rate: " << state.send_control.pacing_rate/1000.f<< " kbps, "
        << "Receiver_limit: " << state.receiver_limit/1000.f<< " kbps";
  }
  return os;
}
