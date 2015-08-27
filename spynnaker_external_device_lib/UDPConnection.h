#ifndef _UDP_CONNECTION_H_
#define _UDP_CONNECTION_H_

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

#include <stdlib.h>

static inline struct sockaddr *get_address(char *ip_address, int port) {
    struct hostent *lookup_address = gethostbyname(ip_address);
    if (lookup_address == NULL) {
        throw "local_host address not found";
    }
    struct sockaddr_in *local_address =
        (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    local_address->sin_family = AF_INET;
    local_address->sin_addr.s_addr = (u_long) lookup_address->h_addr;
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

private:
    int sock;
    bool can_send;
    int local_port;
    unsigned char *local_ip_address;
    int remote_port;
    unsigned char *remote_ip_address;
};

#endif // _UDP_CONNECTION_H_
