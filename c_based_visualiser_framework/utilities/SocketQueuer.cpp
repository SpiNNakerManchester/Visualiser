/*
 * socketqueuer.cpp
 *
 *  Created on: 3 Dec 2014
 *      Author: stokesa6
 */

#include "SocketQueuer.h"
#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <unistd.h>

using namespace std;

SocketQueuer::SocketQueuer(int listen_port, char *hostname) {
	this->addr_len_input = sizeof(this->si_other);
	this->sdp_header_len = 26;
	fprintf(stderr, "Socket Queuer Listening...\n");
	init_sdp_listening(listen_port);
	if (hostname != NULL) {
	    send_void_message(hostname, 17893);
	}
	if (pthread_mutex_init(&this->spike_mutex, NULL) == -1) {
	        fprintf(stderr, "Error initializing mutex!\n");
	        exit(-1);
	    }
	pthread_cond_init(&this->cond, 0);
}

// setup socket for SDP frame receiving on port SDPPORT defined about (usually 17894)
void SocketQueuer::init_sdp_listening(int listen_port) {
    char portno_input[6];
    snprintf(portno_input, 6, "%d", listen_port);
    struct addrinfo hints_input;
    bzero(&hints_input, sizeof(hints_input));
    hints_input.ai_family = AF_INET; // set to AF_INET to force IPv4
    hints_input.ai_socktype = SOCK_DGRAM; // type UDP (socket datagram)
    hints_input.ai_flags = AI_PASSIVE; // use my IP

    int rv_input;
    struct addrinfo *servinfo_input;
    if ((rv_input = getaddrinfo(NULL, portno_input, &hints_input,
            &servinfo_input)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_input));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    struct addrinfo *p_input;
    for (p_input = servinfo_input; p_input != NULL; p_input =
            p_input->ai_next) {
        if ((this->sockfd_input = socket(p_input->ai_family,
                                         p_input->ai_socktype,
                                         p_input->ai_protocol))
                == -1) {
            printf("SDP SpiNNaker listener: socket");
            perror("SDP SpiNNaker listener: socket");
            continue;
        }

        if (bind(this->sockfd_input, p_input->ai_addr,
                 p_input->ai_addrlen) == -1) {
            close(this->sockfd_input);
            printf("SDP SpiNNaker listener: bind");
            perror("SDP SpiNNaker listener: bind");
            continue;
        }

        break;
    }

    if (p_input == NULL) {
        fprintf(stderr, "SDP listener: failed to bind socket\n");
        printf("SDP listener: failed to bind socket\n");
        exit(-1);
    }

    freeaddrinfo(servinfo_input);
}
void SocketQueuer::send_void_message(char *hostname, int port) {
    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;
    struct addrinfo* addr_info = 0;
    if (getaddrinfo(hostname, NULL, &hints, &addr_info) != 0) {
        fprintf(stderr, "Could not resolve hostname, exiting\n");
        exit(-1);
    }
    ((struct sockaddr_in *) addr_info->ai_addr)->sin_port = htons(port);

    char data [1];
    if (sendto(this->sockfd_input, data, 1, 0,
        addr_info->ai_addr, addr_info->ai_addrlen) == -1) {
        fprintf(stderr, "Could not send packet \n");
        exit(-1);
    }
}

void SocketQueuer::InternalThreadEntry(){
	unsigned char buffer_input[1515];
	while (1) {                             // for ever ever, ever ever.
		int numbytes_input = recvfrom(this->sockfd_input, (char *) buffer_input,
				sizeof(buffer_input), 0, (sockaddr*) &this->si_other,
				(socklen_t*) &this->addr_len_input);
		if (numbytes_input == -1) {
			fprintf(stderr, "Packet not received, exiting\n");
			exit(-1);
		}
		if (numbytes_input < 9) {
			fprintf(stderr, "Error - packet too short\n");
			continue;
		}
        // create eieio message
		struct eieio_message* new_message = new eieio_message();

		new_message->header.count = buffer_input[0];
		new_message->header.p = ((buffer_input[1] >> 7) & 1);
		new_message->header.f = ((buffer_input[1] >> 6) & 1);
		new_message->header.d = ((buffer_input[1] >> 5) & 1);
		new_message->header.t = ((buffer_input[1] >> 4) & 1);
		new_message->header.type = ((buffer_input[1] >> 2) & 3);
		new_message->header.tag = (buffer_input[1] & 3);
		new_message->data = (unsigned char *) malloc(numbytes_input - 2);
		memcpy(new_message->data, &buffer_input[2], numbytes_input - 2);
		// load message into buffer
		pthread_mutex_lock(&this->spike_mutex);
		this->queue.push_back(*new_message);
		pthread_cond_signal(&this->cond);
		pthread_mutex_unlock(&this->spike_mutex);
	}
}

eieio_message SocketQueuer::get_next_packet(){
	eieio_message packet;
	pthread_mutex_lock(&this->spike_mutex);
	while(this->queue.size() == 0) {
        pthread_cond_wait(&this->cond, &this->spike_mutex);
    }
	packet = this->queue.front();
	this->queue.pop_front();
	pthread_mutex_unlock(&this->spike_mutex);
	return packet;
}

bool SocketQueuer::is_queue_empty(){
	return this->queue.empty();
}

void free_packet(eieio_message message){
	free(message.data);

}

SocketQueuer::~SocketQueuer() {
	// TODO Auto-generated destructor stub
}
