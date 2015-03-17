/*
 * UdpListener.h
 *
 *  Created on: 2 Dec 2014
 *      Author: alan
 */
#include "EIEIOMessage.h"
#ifndef WIN32
#include <netdb.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#include <ws2tcpip.h>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)
#define close(sock)
typedef unsigned int uint;
typedef unsigned short ushort;
#endif

#ifndef UDPLISTENER_H_
#define UDPLISTENER_H_

class DatabaseMessageConnection {
	int sockfd_input;
	struct sockaddr_in response_address;
public:
	virtual ~DatabaseMessageConnection();
	DatabaseMessageConnection(int listen_port);
	char* recieve_notification();
	void send_ready_notification();
	void close_connection();
private:
	void init_sdp_listening(int listen_port);
};

#endif /* UDPLISTENER_H_ */
