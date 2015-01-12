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
	//check that its a data message
	if (message.header.f != 0 or message.header.p != 0 or message.header.d != 1
		or message.header.t != 1 or message.header.type != 2){
		printf("this packet was determined to be a "
				"command packet. therefore not processing it.");
		printf ("eieio message: p=%i f=%i d=%i t=%i type=%i tag=%i "
						"count=%i data={", message.header.p,
						message.header.f, message.header.d,
						message.header.t, message.header.type,
						message.header.tag, message.header.count);
		for(int position = 0; position < strlen(message.data); position++){
			printf ("%c", message.data[position]);
		}
		printf ("}\n");

	} else {
		int y_pos = (int(message.data[0]) << 24) +
					(int(message.data[1]) << 16) +
					(int(message.data[2]) << 8) +
					(int(message.data[3]));

		for (int position = 0; position < message.header.count; position++){
			int data_position = (position * SIZE_OF_KEY) + 4;
			int key;
			key = (int(message.data[data_position]) << 24) +
				  (int(message.data[data_position + 1]) << 16) +
				  (int(message.data[data_position + 2]) << 8) +
				  (int(message.data[data_position + 3]));
			map<int, int> temp = *(this->neuron_id_mapping);
			int x_pos = temp[key];
			int time;
			pair<int, int> point(x_pos, y_pos);
			points.push_back(point);
		}
	}
}

PacketConverter::~PacketConverter() {
	// TODO Auto-generated destructor stub
}

