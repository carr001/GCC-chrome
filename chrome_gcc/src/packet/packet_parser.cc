#include "src/packet/packet_parser.h"

#include <string>
#include <iostream>

#include "src/time/timestamp.h"
#include "src/common/defs.h"
#include "src/common/utils.h"
#include "src/common/data_rate.h"

PACKET_TYPE PacketParser::GetType(const char* data){
    std::string s(data);
    return string_to_packet_type(s.substr(0,TYPE_LEN));
}

void GCCDataPacketParser::UnSerialize(const char* data,Packet* p){
    if( GetType(data)==PACKET_TYPE::DATA){
        GCCDataPacket* packet = (GCCDataPacket*) p;
        std::string s(data);

        packet->sequence_number  = std::stoul( s.substr(TYPE_LEN,PACKET_NUMBER_LEN));
        packet->send_time        = webrtc::Timestamp::Micros(
                std::stoul( s.substr(TYPE_LEN+PACKET_NUMBER_LEN,TIME_SRTING_LEN)));
        packet->size = std::stoul( s.substr(TYPE_LEN+PACKET_NUMBER_LEN+TIME_SRTING_LEN,PACKET_SIZE_LEN));
    }
}

void GCCRRPacketParser::UnSerialize(const char* data,Packet* p){
    if( GetType(data)==PACKET_TYPE::RR){
        GCCRRPacket* packet = (GCCRRPacket*) p;
        std::string s(data);

        packet->start_time  =webrtc::Timestamp::Micros(
             std::stoul( s.substr(TYPE_LEN,TIME_SRTING_LEN)));
        packet->end_time    = webrtc::Timestamp::Micros(
             std::stoul( s.substr(TYPE_LEN+TIME_SRTING_LEN,TIME_SRTING_LEN)));
        packet->packets_lost_delta  = std::stoul( s.substr(TYPE_LEN+2*TIME_SRTING_LEN,PACKET_LOST_LEN));
        packet->packets_received_delta = std::stoul( s.substr(TYPE_LEN+2*TIME_SRTING_LEN+PACKET_LOST_LEN,PACKET_RECEIVE_LEN));
    }
}

void GCCREMBPacketParser::UnSerialize(const char* data,Packet* p){
    if( GetType(data)==PACKET_TYPE::REMB){
        GCCREMBPacket* packet = (GCCREMBPacket*) p;
        std::string s(data);
        packet->bandwidth  = webrtc::DataRate::BitsPerSec(std::stoul( s.substr(TYPE_LEN,DATA_RATE_LEN)));

    }
}






