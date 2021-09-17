#include "src/packet/packet.h"
#include "src/common/defs.h"

class PacketParser{
public:
    static PACKET_TYPE GetType(const char*);
    virtual void UnSerialize(const char*,Packet*)=0;
};

class GCCDataPacketParser:public PacketParser{
public:
    GCCDataPacketParser()=default;
    ~GCCDataPacketParser()=default;
    void UnSerialize(const char*,Packet*) override;
};
class GCCRRPacketParser:public PacketParser{
public:
    GCCRRPacketParser()=default;
    ~GCCRRPacketParser()=default;
    void UnSerialize(const char*,Packet*) override;
};
class GCCREMBPacketParser:public PacketParser{
public:
    GCCREMBPacketParser()=default;
    ~GCCREMBPacketParser()=default;
    void UnSerialize(const char*,Packet*) override;
};




