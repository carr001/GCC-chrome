#include "src/receiver.h"

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>

#include "src/common/utils.h"
#include "src/common/optional.h"
#include "src/quic/quic_chromium_clock.h"
#include "src/quic/quic_time.h"
#include "src/google_cc/delay_based_bwe.h"
#include "glog/logging.h"
Receiver::Receiver(webrtc::NetworkControllerConfig config)
    :
      clock_(new QuicChromiumClock()),
      gcc_controler_(new webrtc::GoogCcNetworkController(config)),
      loss_collection_(new webrtc::ReceiverLossCollection()),
      last_remb_rate_(webrtc::DataRate::Zero()),
      receiver_debuger_(ReceiverDebuger()),
      last_ack_rate_(0)
      {
    local_addr_.sin_family      = AF_INET;
    // local_addr_.sin_port = htonl(RECEIVERPORT);
    local_addr_.sin_port        = htons(RECEIVERPORT);
    local_addr_.sin_addr.s_addr = htonl(RECEIVER_IP);

    // remote_addr_.sin_family      = AF_INET;
    // remote_addr_.sin_port        = htons(SENDERPORT);
    // remote_addr_.sin_addr.s_addr = htonl(SENDER_IP);
    Bind();
}
Receiver::~Receiver(){
    close(local_socket_);
}
void Receiver::Bind(){
    local_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    int num  = bind(local_socket_, (struct sockaddr*)&local_addr_, sizeof(struct sockaddr));
    if(num<0){
        pause_at_func(__FUNCTION__);
    }
    std::cout<<"bind port "<<RECEIVERPORT<<" succeed."<<std::endl;
}

void Receiver::SendPacketFromBuf(){
    ssize_t num;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    num = sendto(local_socket_,send_buf_,strlen(send_buf_),MSG_DONTWAIT,(struct sockaddr *)&remote_addr_,addrlen);
    if(num<0){
        pause_at_func(__FUNCTION__);
    }
}

void Receiver::SetPacketToBuf(Packet* packet){
    memset(send_buf_,'\0',SEND_BUF_SIZE);
    const std::string s = packet->Serialize();
    memcpy(send_buf_,s.c_str(),s.length()*sizeof(char));
}

void Receiver::OnPacketReceived(){
    PACKET_TYPE type = PacketParser::GetType(receive_buf_);
    switch (type)
    {
    case PACKET_TYPE::DATA:
        OnDataPacketReceived();
        break;
    default:
        break;
    }
    // debuger_->PrintPacketInfo(packet);
}

void Receiver::OnDataPacketReceived(){
    GCCDataPacket data_packet = GCCDataPacket();
    data_packet_parser_.UnSerialize(receive_buf_,&data_packet);

    webrtc::TransportPacketsFeedback msg;
    msg.feedback_time = webrtc::Timestamp::Micros(clock_->Now().GetUint64());
    webrtc::PacketResult result;
    result.sent_packet.sequence_number = data_packet.sequence_number;
    result.sent_packet.send_time = data_packet.send_time;
    result.sent_packet.size = webrtc::DataSize::Bytes(data_packet.size);
    result.receive_time = webrtc::Timestamp::Micros(clock_->Now().GetUint64());
    msg.packet_feedbacks.push_back(result);

    gcc_controler_->OnTransportPacketsFeedback(msg);
    webrtc::DelayBasedBwe::Result delay_result = gcc_controler_->recent_result();
    if(delay_result.updated){
        OnDelayBaseBweUpdated( delay_result);
    }
    auto ackrate = gcc_controler_->GetAcknowlegeRate();
    if(ackrate){
        uint64_t rate = ackrate.value().GetUint64();
        if(rate!=last_ack_rate_){
            LOG(INFO)<<"Ack_rate: "<<rate/1000.f<<" kbps";
            last_ack_rate_=rate;
        }
    }
    webrtc::LossReportUpdate loss_result = loss_collection_->OnTransportPacketsFeedback(msg);
    if(loss_result.updated){
        OnLossReportGenerated(loss_result);
    }
}

void Receiver::OnDelayBaseBweUpdated(webrtc::DelayBasedBwe::Result report){
    GCCREMBPacket packet ;
    if(last_remb_rate_.IsZero()){
        last_remb_rate_ = report.target_bitrate;
        packet.bandwidth = report.target_bitrate;

        SetPacketToBuf(&packet);
        SendPacketFromBuf();
    }else if(last_remb_rate_!=report.target_bitrate) {
        packet.bandwidth = report.target_bitrate;

        SetPacketToBuf(&packet);
        SendPacketFromBuf();
        last_remb_rate_ = report.target_bitrate;
    }

}

void Receiver::OnLossReportGenerated(webrtc::LossReportUpdate report){
    GCCRRPacket packet;
    packet.end_time              = report.report.end_time;
    packet.start_time            = report.report.start_time;
    packet.packets_lost_delta    = report.report.packets_lost_delta;
    packet.packets_received_delta= report.report.packets_received_delta;
    SetPacketToBuf(&packet);
    SendPacketFromBuf();
}


void Receiver::ReceivePacket(){
    // std::cout<<"RecvData entered"<<std::endl;
    memset(receive_buf_,'\0',RECEIVE_BUF_SIZE);
    ssize_t num;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    num =recvfrom(local_socket_,receive_buf_,RECEIVE_BUF_SIZE,MSG_DONTWAIT,(struct sockaddr*)&remote_addr_,&addrlen); 
    // std::cout<<"buff received"<<std::endl;
    if (num < 0){
        // std::cout<<"No received packets yet"<<std::endl;
    }else{
        // recent_packet_number_ = parser_->GetPacketNumber(receive_buf_);
        receiver_debuger_.packet_received++;
        // std::cout<<receive_buf_<<std::endl;
        OnPacketReceived();
    }
    receiver_debuger_.PrintInfo();
}


