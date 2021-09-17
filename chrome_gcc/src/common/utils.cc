#include <iostream>
#include <unistd.h>

#include "src/common/utils.h"

std::string number_to_fix_len_string(uint64_t number,int len){
    char* c = (char*) malloc(len);
    for(int i=0;i<len+1;i++){
        c[i] = '0';
    }
    std::string s = std::to_string(number);
    if(s.length()<=len){
        for(int i=len-1,j=s.length()-1;j>-1;i--,j--){
            c[i] = s[j];
        }
    }else{
    std::cout<<" #############        We are in " <<__FUNCTION__<<"   #################"<<std::endl;
        std::cout<<"Number to big, conversion not completed"<<std::endl;
    }
    c[len] = '\0';
    s = std::string(c);
    delete c;
    return s;
}

void pause_at_func(std::string s){
    std::cout<<" #############        We are in " <<s<<"   #################"<<std::endl;
    std::cout<<" ############# Fatal error occured, The program is paused for debug #################"<<std::endl;
    // ClientPrintDebugInfo();
    pause();
}

std::string packet_type_to_string(PACKET_TYPE t){
    switch (t)
    {
    case PACKET_TYPE::DATA:
        return TYPE_DATA_STR;
        break;
    case PACKET_TYPE::RR:
        return TYPE_RR_STR;
        break;
    case PACKET_TYPE::REMB:
        return TYPE_REMB_STR;
        break;
    default:
        break;
    }
    return "";
};

PACKET_TYPE string_to_packet_type(std::string s){
    if(s==TYPE_DATA_STR)
        return PACKET_TYPE::DATA;
    if(s==TYPE_RR_STR)
        return PACKET_TYPE::RR;
    if(s==TYPE_REMB_STR)
        return PACKET_TYPE::REMB;
    pause_at_func(__FUNCTION__);
    return PACKET_TYPE::DATA;
};
