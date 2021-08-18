#ifndef __SDPMessage__
#define __SDPMessage__

#define REPLY_NOT_EXPECTED 0x07
#define REPLY_EXPECTED 0x87

#define SCP_SIGNAL_CMD 22
#define SIGNAL_MC_TYPE 0

typedef enum sync_type {
    SYNC_0=4,
    SYNC_1=5
} sync_type;

typedef struct SDPHeader {
    unsigned int flags:8;
    unsigned int tag:8;
    unsigned int dest_cpu:5;
    unsigned int dest_port:3;
    unsigned int source_cpu:5;
    unsigned int source_port:3;
    unsigned int dest_y:8;
    unsigned int dest_x:8;
    unsigned int source_y:8;
    unsigned int source_x:8;

    void set_no_reply(unsigned int tag,
            unsigned int dest_x, unsigned int dest_y, unsigned int dest_cpu,
            unsigned int dest_port) {
        this->flags = REPLY_NOT_EXPECTED;
        this->tag = tag;
        this->dest_x = dest_x;
        this->dest_y = dest_y;
        this->dest_cpu = dest_cpu;
        this->dest_port = dest_port;
        this->source_x = 0;
        this->source_y = 0;
        this->source_cpu = 0;
        this->source_port = 0;
    }
} SDPHeader;

typedef struct SCPHeader {
    unsigned short cmd;
    unsigned short sequence;
} SCPHeader;

typedef struct SCPSyncSignalMessage {
    SDPHeader sdp_header;
    SCPHeader scp_header;
    unsigned int signal_type;
    unsigned short signal;
    unsigned char app_mask;
    unsigned char app_id;
    unsigned int core_mask;

    SCPSyncSignalMessage(unsigned char app_id, sync_type sync_signal) {
        this->sdp_header.flags = REPLY_EXPECTED;
        this->sdp_header.tag = 0;
        this->sdp_header.dest_cpu = 0;
        this->sdp_header.dest_port = 0;
        this->sdp_header.dest_x = 0;
        this->sdp_header.dest_y = 0;
        this->sdp_header.source_cpu = 0;
        this->sdp_header.source_port = 0;
        this->sdp_header.source_x = 0;
        this->sdp_header.source_y = 0;
        this->scp_header.cmd = SCP_SIGNAL_CMD;
        this->scp_header.sequence = 0;
        this->signal_type = SIGNAL_MC_TYPE;
        this->signal = (unsigned short) sync_signal;
        this->app_mask = 0xFF;
        this->app_id = app_id;
        this->core_mask = 0xFFFF;
    }
} SCPSignalMessage;

#endif
