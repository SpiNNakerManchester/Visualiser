/*
 * Copyright (c) 2015 The University of Manchester
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __SDPMessage__
#define __SDPMessage__

#define REPLY_NOT_EXPECTED 0x07
#define REPLY_EXPECTED 0x87
#define REPLY_EXPECTED_NOT_P2P (REPLY_EXPECTED | 0x20)

#define SCP_SIGNAL_CMD 22
#define SCP_IPTAG_CMD 26
#define SIGNAL_MC_TYPE 0
#define IPTAG_SET 1

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
    unsigned char app_id;
    unsigned char app_mask;
    unsigned short signal;
    unsigned int core_mask;

    SCPSyncSignalMessage(unsigned char app_id, sync_type sync_signal) {
        this->sdp_header.flags = REPLY_EXPECTED;
        this->sdp_header.tag = 0xFF;
        this->sdp_header.dest_cpu = 0;
        this->sdp_header.dest_port = 0;
        this->sdp_header.dest_x = 255;
        this->sdp_header.dest_y = 255;
        this->sdp_header.source_cpu = 0x1F;
        this->sdp_header.source_port = 0x7;
        this->sdp_header.source_x = 255;
        this->sdp_header.source_y = 255;
        this->scp_header.cmd = SCP_SIGNAL_CMD;
        this->scp_header.sequence = 0;
        this->signal_type = SIGNAL_MC_TYPE;
        this->signal = (unsigned short) sync_signal;
        this->app_mask = 0xFF;
        this->app_id = app_id;
        this->core_mask = 0xFFFF;
    }
} SCPSignalMessage;

typedef struct SCPIPTagSetMessage {
    SDPHeader sdp_header;
    SCPHeader scp_header;
    unsigned int tag:16;
    unsigned int cmd:12;
    unsigned int use_sender:2;
    unsigned int strip_sdp_headers:2;
    unsigned int port;
    unsigned int ip_address;

    SCPIPTagSetMessage(unsigned int x, unsigned int y, unsigned int tag,
            unsigned int strip_sdp_headers) {
        this->sdp_header.flags = REPLY_EXPECTED_NOT_P2P;
        this->sdp_header.tag = 0xFF;
        this->sdp_header.dest_cpu = 0;
        this->sdp_header.dest_port = 0;
        this->sdp_header.dest_x = x;
        this->sdp_header.dest_y = y;
        this->sdp_header.source_cpu = 0x1F;
        this->sdp_header.source_port = 0x7;
        this->sdp_header.source_x = 0;
        this->sdp_header.source_y = 0;
        this->scp_header.cmd = SCP_IPTAG_CMD;
        this->scp_header.sequence = 0;
        this->strip_sdp_headers = strip_sdp_headers;
        this->use_sender = 1;
        this->cmd = IPTAG_SET;
        this->tag = tag;
        this->port = 0;
        this->ip_address = 0;
    }
} SCPSetTagMessage;

#endif
