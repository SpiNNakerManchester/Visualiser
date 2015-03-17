/*
 * socketqueuer.h
 *
 *  Created on: 3 Dec 2014
 *      Author: stokesa6
 */

#include "Threadable.h"
#include "EIEIOMessage.h"
#include <deque>
#include <pthread.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)
typedef unsigned int uint;
typedef unsigned short ushort;
typedef int socklen_t;
#endif

#ifndef C_VIS_DATABASE_SOCKETQUEUER_H_
#define C_VIS_DATABASE_SOCKETQUEUER_H_

#include <stdio.h>

class SocketQueuer : public Threadable {
public:
    SocketQueuer(int, char*);
    virtual ~SocketQueuer();
    eieio_message get_next_packet();
    void free_packet(eieio_message);
    bool is_queue_empty();
protected:
    virtual void InternalThreadEntry();
private:
    void init_sdp_listening(int);
    void send_void_message(char *, int);
    std::deque<eieio_message> queue;
    int sockfd_input;
    struct sockaddr_in si_other; // for incoming frames
    socklen_t addr_len_input;
    char sdp_header_len;
    pthread_mutex_t spike_mutex;
    pthread_cond_t cond;
};

#endif /* C_VIS_DATABASE_SOCKETQUEUER_H_ */
