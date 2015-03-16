/*
 * packetconverter.cpp
 *
 *  Created on: 16 Dec 2014
 *      Author: stokesa6
 */

#include "PacketConverter.h"
#include "../utilities/EIEIOMessage.h"
#include <deque>
#include <list>
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <string.h>

using namespace std;

PacketConverter::PacketConverter(SocketQueuer *packet_queue,
                                deque< pair<int, int> > *points_to_plot,
                                pthread_mutex_t *point_mutex,
                                map<int, int> *neuron_id_mapping) {

    this->points_to_plot = points_to_plot;
    this->packet_queue = packet_queue;
    this->point_mutex = point_mutex;
    this->neuron_id_mapping = neuron_id_mapping;
}

void PacketConverter::InternalThreadEntry(){
    while (1) {
        eieio_message message;
        message = this->packet_queue->get_next_packet();
        list<pair<int, int> > data;
        translate_eieio_message_to_points(message, data);
        //add points to the point queue
        pthread_mutex_lock(this->point_mutex);
        for(list<pair<int, int> >::iterator iter = data.begin();
                iter != data.end(); ++iter) {
            this->points_to_plot->push_back(*iter);
        }
        pthread_mutex_unlock(this->point_mutex);
    }
}

void PacketConverter::translate_eieio_message_to_points(
        eieio_message &message, list<pair<int, int> > &points){

    //fprintf(stderr, "eieio message: p=%i f=%i d=%i t=%i type=%i tag=%i "
    //        "count=%i data={", message.header.p,
    //        message.header.f, message.header.d,
    //        message.header.t, message.header.type,
    //        message.header.tag, message.header.count);
    //for(int position = 0; position < ((message.header.count + 1) * 4);
    //        position++){
    //    printf (" 0x%.2x", (uint) message.data[position]);
    //}
    //printf (" }\n");

    //check that its a data message
    if (message.header.f != 0 or message.header.p != 0 or message.header.d != 1
        or message.header.t != 1 or message.header.type != 2){
        printf("this packet was determined to not be a "
            "data packet with a fixed payload as a time stamp and then keys."
            "therefore not processing it.");
        printf("eieio message: p=%i f=%i d=%i t=%i type=%i tag=%i count=%i\n",
            message.header.p, message.header.f, message.header.d,
            message.header.t, message.header.type, message.header.tag,
            message.header.count);

    } else {
        uint time = (message.data[3] << 24) |
                    (message.data[2] << 16) |
                    (message.data[1] << 8) |
                    (message.data[0]);

        for (int position = 0; position < message.header.count; position++){
            int data_position = (position * SIZE_OF_KEY) + 4;
            int key = (message.data[data_position + 3] << 24) |
                    (message.data[data_position + 2] << 16) |
                    (message.data[data_position + 1] << 8) |
                    (message.data[data_position]);
            int neuron_id = (*(this->neuron_id_mapping))[key];
            if (neuron_id_mapping->find(key) == neuron_id_mapping->end()) {
                fprintf(stderr, "Missing neuron id for key %d\n", key);
                continue;
            }
            //fprintf(stderr, "time = %i, key = %i, neuron_id = %i\n", time, key,
            //        neuron_id);
            pair<int, int> point(time, neuron_id);
            points.push_back(point);
        }
    }
}

PacketConverter::~PacketConverter() {
    // TODO Auto-generated destructor stub
}

