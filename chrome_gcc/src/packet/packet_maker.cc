#include "src/packet/packet_maker.h"

#include <iostream>

#include "src/common/utils.h"

GCCDataPacketMaker::GCCDataPacketMaker()
    :
    recent_packet_number_(0),
    packet_template_(new GCCDataPacket())
    {}

GCCDataPacket GCCDataPacketMaker::MakePacket(){
    ResetTemplate();
    recent_packet_number_ ++;
    
    packet_template_->sequence_number = recent_packet_number_ ;
    // if(recent_packet_number_== 20){
    //     std::cout<<packet_template_->Serialize() <<std::endl;
    //     pause_at_func(__FUNCTION__);
    // }
    return *packet_template_;
}

uint64_t GCCDataPacketMaker::GetRecentPacketNumber(){
    return recent_packet_number_;
}

void GCCDataPacketMaker::ResetTemplate(){
    packet_template_->sequence_number = 0 ;
    packet_template_->send_time = webrtc::Timestamp::Micros(0) ;
}

GCCRRPacketMaker::GCCRRPacketMaker()
    :
    packet_template_(new GCCRRPacket())
    {}

GCCRRPacket GCCRRPacketMaker::MakePacket(webrtc::Timestamp start_time, 
        webrtc::Timestamp  end_time,
        uint64_t packets_lost_delta,
        uint64_t packets_received_delta){
    ResetTemplate();

    packet_template_->start_time = start_time;    
    packet_template_->end_time = end_time;    
    packet_template_->packets_lost_delta = packets_lost_delta;    
    packet_template_->packets_received_delta = packets_received_delta;    

    return *packet_template_;
}

void GCCRRPacketMaker::ResetTemplate(){
    packet_template_->start_time = webrtc::Timestamp::Micros(0) ;
    packet_template_->end_time = webrtc::Timestamp::Micros(0) ;
    packet_template_->packets_lost_delta = 0;    
    packet_template_->packets_received_delta = 0;    
}


