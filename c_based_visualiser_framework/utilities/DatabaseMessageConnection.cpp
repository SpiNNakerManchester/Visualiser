#include "DatabaseMessageConnection.h"
#include <stdio.h>
#include <string.h>
#include <map>
#include <deque>
#include <cstdlib>
#include <unistd.h>

DatabaseMessageConnection::~DatabaseMessageConnection() {
	// TODO Auto-generated destructor stub
}

DatabaseMessageConnection::DatabaseMessageConnection(int listen_port) {
	init_sdp_listening(listen_port);
}

// setup socket for SDP frame receiving on port SDPPORT defined about (usually 17894)
void DatabaseMessageConnection::init_sdp_listening(int listen_port) {
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
            printf("Hand shaker listener: socket");
            perror("Hand shaker listener: socket");
            continue;
        }

        if (bind(this->sockfd_input, p_input->ai_addr,
                 p_input->ai_addrlen) == -1) {
            close(this->sockfd_input);
            printf("Hand shaker listener: bind");
            perror("Hand shaker listener: bind");
            continue;
        }

        break;
    }

    if (p_input == NULL) {
        fprintf(stderr, "Hand shaker: failed to bind socket\n");
        printf("Hand shaker: failed to bind socket\n");
        exit(-1);
    }

    freeaddrinfo(servinfo_input);
}

char* DatabaseMessageConnection::recieve_notification(){
	socklen_t addr_len_input = sizeof(struct sockaddr_in);
	char sdp_header_len = 26;
	unsigned char buffer_input[1515];
	bool received = false;
    char* absolute_path = NULL;

	while (!received){
		int numbytes_input = recvfrom(
		    this->sockfd_input, (char *) buffer_input, sizeof(buffer_input), 0,
		    (sockaddr*) &this->response_address, (socklen_t*) &addr_len_input);
		if (numbytes_input == -1) {
			fprintf(stderr, "Packet not received, exiting\n");

			// will only get here if there's an error getting the input frame
			// off the Ethernet
			exit(-1);
		}
		if (numbytes_input < 2) {
			fprintf(stderr, "Error - packet too short\n");
			continue;
		}
		printf("packet received\n");
		if (!absolute_path){
		    absolute_path = (char*) malloc(sizeof(char) * (numbytes_input -1));
		    for(int position =2; position < numbytes_input; position++){
                absolute_path[position-2] = buffer_input[position];
            }
            absolute_path[numbytes_input-2] = '\0';
		}
		received = true;
	}
	return absolute_path;
}

void DatabaseMessageConnection::send_ready_notification(){

	char message[2];
	message[1] = (1 << 6);
	message[0] = 1;
	int length = 2;

	int response = sendto(this->sockfd_input, message, length, 0,
			          (struct sockaddr *) &this->response_address,
			          sizeof(this->response_address));
}

void DatabaseMessageConnection::close_connection(){
	close(this->sockfd_input);
}
