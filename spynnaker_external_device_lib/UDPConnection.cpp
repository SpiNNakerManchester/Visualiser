#include "UDPConnection.h"
#include <exception>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

UDPConnection::UDPConnection(
        int local_port, char *local_host, int remote_port, char *remote_host)
{
#ifdef WIN32
    WSADATA wsaData; // if this doesn't work
    //WSAData wsaData; // then try this instead

    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }
#endif

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == 0) {
        throw "Socket could not be created";
    }

    local_ip_address = htonl(INADDR_ANY);
    if (local_host != NULL) {
        auto lookup_address = gethostbyname(local_host);
        if (lookup_address == NULL) {
            throw "local_host address not found";
        }

        memcpy(&local_ip_address, lookup_address->h_addr,
               lookup_address->h_length);
    }


    struct sockaddr_in local_address;
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = local_ip_address;
    local_address.sin_port = htons(local_port);

    if (bind(sock, (struct sockaddr *) &local_address, sizeof local_address) < 0) {
        throw "Error binding to local address";
    }

    this->can_send = false;
    this->remote_ip_address = 0;
    this->remote_port = 0;

    if (remote_host != NULL && remote_port != 0) {
        can_send = true;
        this->remote_port = remote_port;

        auto lookup_address = gethostbyname(remote_host);
        if (lookup_address == NULL) {
            throw "remote_host address not found";
        }

        memcpy(&remote_ip_address, lookup_address->h_addr,
               lookup_address->h_length);

        struct sockaddr_in remote_address;
        remote_address.sin_family = AF_INET;
        remote_address.sin_addr.s_addr = remote_ip_address;
        remote_address.sin_port = htons(remote_port);

        if (connect(sock, (struct sockaddr *) &remote_address,
                    sizeof remote_address) < 0) {
            throw "Error connecting to remote address";
        }
    }

    socklen_t local_address_length = sizeof local_address;
    if (getsockname(sock, (struct sockaddr *) &local_address,
                    &local_address_length) < 0) {
        throw "Error getting local socket address";
    }

    this->local_ip_address = local_address.sin_addr.s_addr;
    this->local_port = ntohs(local_address.sin_port);
}

int UDPConnection::receive_data(unsigned char *data, int length) {
    int received_length = recv(sock, (char *) data, length, 0);
    if (received_length < 0) {
        throw "Error receiving data";
    }
    return received_length;
}

static inline int receive_from(
	int sock, unsigned char *data, int dlen, void *addr, int alen) {
    socklen_t address_length = alen;
    int received_length = recvfrom(sock, data, dlen, 0,
	    (sockaddr *) addr, (socklen_t *) &address_length);
    if (received_length < 0) {
        throw "Error receiving data";
    }
    return received_length;
}

int UDPConnection::receive_data_with_address(
	unsigned char *data, int length, sockaddr *address) {
    return receive_from(sock, data, length, address, sizeof *address);
}

int UDPConnection::receive_data_with_address(
	unsigned char *data, int length, sockaddr_in *address) {
    return receive_from(sock, data, length, address, sizeof *address);
}

void UDPConnection::send_data(unsigned char *data, int length) {
    if (send(sock, (const char *) data, length, 0) < 0) {
        throw "Error sending data";
    }
}

static inline void send_to(
	int sock, void *data, int dlen, void *addr, int alen) {
    if (sendto(sock, data, dlen, 0, (struct sockaddr *) addr, alen) < 0) {
        throw "Error sending data";
    }
}

void UDPConnection::send_data_to(
	unsigned char *data, int length, sockaddr* address) {
    send_to(sock, data, length, address, sizeof *address);
}

void UDPConnection::send_data_to(
	unsigned char *data, int length, sockaddr_in* address) {
    send_to(sock, data, length, address, sizeof *address);
}

UDPConnection::~UDPConnection() {
    shutdown(sock, SHUT_RDWR);
    close(sock);
}
