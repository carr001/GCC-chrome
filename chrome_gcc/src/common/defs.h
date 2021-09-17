#ifndef DEFS_H
#define DEFS_H

#include "src/time/timestamp.h"
#include "src/quic/quic_time.h"
// time
#define ONE_SECOND 1000000


// packet maker
#define MAXDGMSIZE (1472)
#define DEFAULT_PACKET_SIZE MAXDGMSIZE
#define TYPE_DATA_STR "00"
#define TYPE_RR_STR "01"
#define TYPE_REMB_STR "10"
#define TYPE_LEN 2
#define PACKET_NUMBER_LEN 10
#define PACKET_SIZE_LEN PACKET_NUMBER_LEN
#define PACKET_LOST_LEN PACKET_NUMBER_LEN
#define PACKET_RECEIVE_LEN PACKET_NUMBER_LEN
#define DATA_RATE_LEN PACKET_NUMBER_LEN
#define TIME_SRTING_LEN 15
// minus 1 means '\0'
#define DATA_PADDING_LEN (DEFAULT_PACKET_SIZE-TYPE_LEN-PACKET_NUMBER_LEN-TIME_SRTING_LEN-PACKET_SIZE_LEN-1)
// #define RR_PADDING_LEN (DEFAULT_PACKET_SIZE-TYPE_LEN-PACKET_LOST_LEN-TIME_SRTING_LEN-TIME_SRTING_LEN-PACKET_RECEIVE_LEN-1)
// #define REMB_PADDING_LEN (DEFAULT_PACKET_SIZE-TYPE_LEN-PACKET_LOST_LEN-TIME_SRTING_LEN-TIME_SRTING_LEN-PACKET_RECEIVE_LEN-1)

#define INITIAL_PACING_RATE ((1000)*500)
#define MIN_PACING_RATE ((1000)*100)


#define RECEIVER_IP (0xc0a8bd80)
#define SENDER_IP (0xc0a8bd81)
#define RECEIVERPORT 53769
#define SENDERPORT 53768

#define SEND_BUF_SIZE 10000
#define RECEIVE_BUF_SIZE SEND_BUF_SIZE


#define MIN(x,y)  ((x)<(y)?(x):(y))
#define MAX(x,y)  ((x)>(y)?(x):(y))


enum PACKET_TYPE{
    DATA,
    RR,
    REMB,
};
struct SendControl{
    SendControl()
    :
    pacing_rate(INITIAL_PACING_RATE),
    next_send_timestamp(QuicTime::Zero())
    {};
    uint64_t pacing_rate;
    QuicTime next_send_timestamp;
};
using SendControl = struct SendControl;

#endif
