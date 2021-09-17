#include "src/packet/packet.h"

#include <string>
#include <iostream>

#include "src/common/utils.h"
#include "src/common/defs.h"
#include "src/common/data_rate.h"

Packet::Packet(){}

GCCDataPacket::GCCDataPacket()
    :sequence_number(0),send_time(webrtc::Timestamp::Micros(0)),size(DEFAULT_PACKET_SIZE){}
GCCRRPacket::GCCRRPacket()
    :start_time(webrtc::Timestamp::Micros(0)),
    end_time(webrtc::Timestamp::Micros(0)),
    packets_lost_delta(0),
    packets_received_delta(0)
    {}
GCCREMBPacket::GCCREMBPacket()
    :bandwidth(webrtc::DataRate::BitsPerSec(0)){}

const std::string GCCDataPacket::Serialize(){
    std::string content("");
    content += packet_type_to_string(type);
    content += number_to_fix_len_string(sequence_number,PACKET_NUMBER_LEN);
    content += number_to_fix_len_string(send_time.GetUint64(),TIME_SRTING_LEN);
    content += number_to_fix_len_string(size,PACKET_SIZE_LEN);
    content += number_to_fix_len_string(0,DATA_PADDING_LEN);
    return content;
}

const std::string GCCRRPacket::Serialize(){
    std::string content("");
    content += packet_type_to_string(type);
    content += number_to_fix_len_string(start_time.GetUint64(),TIME_SRTING_LEN);
    content += number_to_fix_len_string(end_time.GetUint64(),TIME_SRTING_LEN);
    content += number_to_fix_len_string(packets_lost_delta,PACKET_LOST_LEN);
    content += number_to_fix_len_string(packets_received_delta,PACKET_RECEIVE_LEN);
    // content += number_to_fix_len_string(0,DATA_PADDING_LEN);
    return content;
}

const std::string GCCREMBPacket::Serialize(){
    std::string content("");
    content += packet_type_to_string(type);
    content += number_to_fix_len_string(bandwidth.GetUint64(),DATA_RATE_LEN);
    // content += number_to_fix_len_string(0,DATA_PADDING_LEN);
    return content;
}


