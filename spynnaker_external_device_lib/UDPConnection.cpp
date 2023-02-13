/*
 * Copyright (c) 2015-2023 The University of Manchester
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "UDPConnection.h"
#include <exception>
#include <stdexcept>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

UDPConnection::UDPConnection(
        int local_port, char *local_host, int remote_port, char *remote_host) {

#ifdef WIN32
    WSADATA wsaData; // if this doesn't work
    //WSAData wsaData; // then try this instead

    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }
#endif

    this->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->sock == 0) {
        throw std::runtime_error("Socket could not be created");
    }

    this->local_ip_address = htonl(INADDR_ANY);
    if (local_host != NULL) {
        struct hostent *lookup_address = gethostbyname(local_host);
        if (lookup_address == NULL) {
            throw std::invalid_argument("local_host address not found");
        }

        memcpy(&this->local_ip_address, lookup_address->h_addr,
               lookup_address->h_length);
    }


    struct sockaddr_in local_address;
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = this->local_ip_address;
    local_address.sin_port = htons(local_port);

    if (bind(this->sock, (struct sockaddr *) &local_address,
             sizeof(local_address)) < 0) {
        throw std::runtime_error("Error binding to local address");
    }

    this->can_send = false;
    this->remote_ip_address = 0;
    this->remote_port = 0;

    if (remote_host != NULL && remote_port != 0) {
        this->can_send = true;
        this->remote_port = remote_port;

        struct hostent *lookup_address = gethostbyname(remote_host);
        if (lookup_address == NULL) {
            throw std::invalid_argument("remote_host address not found");
        }

        memcpy(&this->remote_ip_address, lookup_address->h_addr,
               lookup_address->h_length);

        struct sockaddr_in remote_address;
        remote_address.sin_family = AF_INET;
        remote_address.sin_addr.s_addr = this->remote_ip_address;
        remote_address.sin_port = htons(remote_port);

        if (connect(this->sock, (struct sockaddr *) &remote_address,
                    sizeof(remote_address)) < 0) {
            throw std::runtime_error("Error connecting to remote address");
        }
    }

    socklen_t local_address_length = sizeof(local_address);
    if (getsockname(this->sock, (struct sockaddr *) &local_address,
                    &local_address_length) < 0) {
        throw std::invalid_argument("Error getting local socket address");
    }

    this->local_ip_address = local_address.sin_addr.s_addr;
    this->local_port = ntohs(local_address.sin_port);
}

int UDPConnection::receive_data(unsigned char *data, int length) {
    int received_length = recv(this->sock, (char *) data, length, 0);
    if (received_length < 0) {
        throw std::runtime_error("Error receiving data");
    }
    return received_length;
}

int UDPConnection::receive_data_with_address(unsigned char *data, int length,
                                             struct sockaddr *address) {
    int address_length = sizeof(*address);
    int received_length = recvfrom(this->sock, (char *) data, length, 0,
                                   address, (socklen_t *) &address_length);
    if (received_length < 0) {
        throw std::runtime_error("Error receiving data with address");
    }
    return received_length;
}

void UDPConnection::send_data(unsigned char *data, int length) {
    if (send(this->sock, (const char *) data, length, 0) < 0) {
        throw std::runtime_error("Error sending data");
    }
}

void UDPConnection::send_data_to(unsigned char *data, int length,
                                 sockaddr* address) {
    if (sendto(this->sock, (const char *) data, length, 0,
               (const struct sockaddr *) address, sizeof(*address)) < 0) {
        throw std::runtime_error("Error sending data to address");
    }
}

UDPConnection::~UDPConnection() {
    shutdown(this->sock, SHUT_RDWR);
    close(this->sock);
}
