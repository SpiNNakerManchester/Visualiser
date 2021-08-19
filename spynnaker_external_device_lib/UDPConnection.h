/*
 * Copyright (c) 2015-2021 The University of Manchester
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _UDP_CONNECTION_H_
#define _UDP_CONNECTION_H_

#ifndef WIN32
#include <netdb.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#include <ws2tcpip.h>
#define bzero(b, len) (memset((b), '\0', (len)), (void) 0)
#define bcopy(b1, b2, len) (memmove((b2), (b1), (len)), (void) 0)
#define close(sock)

#define SHUT_RD   SD_RECEIVE
#define SHUT_WR   SD_SEND
#define SHUT_RDWR SD_BOTH

typedef unsigned int uint;
typedef unsigned short ushort;
#endif

#include <stdlib.h>
#include <string.h>

#define SCP_SCAMP_PORT 17893

static inline struct sockaddr *get_address(char *ip_address, int port) {
    struct hostent *lookup_address = gethostbyname(ip_address);
    if (lookup_address == NULL) {
        throw "local_host address not found";
    }
    struct sockaddr_in *local_address =
        (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    local_address->sin_family = AF_INET;
    memcpy(&(local_address->sin_addr.s_addr), lookup_address->h_addr,
               lookup_address->h_length);
    local_address->sin_port = htons(port);

    return (struct sockaddr *) local_address;
}


class UDPConnection {

public:
    UDPConnection(int local_port=0, char *local_host=NULL, int remote_port=0,
                  char *remote_host=NULL);
    ~UDPConnection();
    int receive_data(unsigned char *data, int length);
    int receive_data_with_address(unsigned char *data, int length,
                                  struct sockaddr *address);
    void send_data(unsigned char *data, int length);
    void send_data_to(unsigned char *data, int length,
                      struct sockaddr* address);
    int get_local_port() {
        return this->local_port;
    }

private:
    int sock;
    bool can_send;
    int local_port;
    unsigned int local_ip_address;
    int remote_port;
    unsigned int remote_ip_address;
};

#endif // _UDP_CONNECTION_H_
