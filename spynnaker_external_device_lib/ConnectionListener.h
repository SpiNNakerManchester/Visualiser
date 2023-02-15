/*
 * Copyright (c) 2015 The University of Manchester
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
#include "Threadable.h"
#include "EIEIOMessage.h"
#include <set>
#include <queue>

#ifndef _CONNECTION_LISTENER_H_
#define _CONNECTION_LISTENER_H_

class PacketReceiveCallbackInterface {
public:
    virtual void receive_packet_callback(EIEIOMessage *message) = 0;
    virtual ~PacketReceiveCallbackInterface() {};
};

class ConnectionListener : public Threadable {
public:
    ConnectionListener(UDPConnection *connection);
    void add_receive_packet_callback(
        PacketReceiveCallbackInterface *packet_callback);
    void finish();

protected:
    void run();

private:
    class Reader : public Threadable {
    public:
        Reader(ConnectionListener *listener);
        void run();
        ConnectionListener *listener;

    };

    Reader *reader;
    UDPConnection* _connection;
    bool _done;
    pthread_mutex_t free_data_mutex;
    pthread_mutex_t data_to_process_mutex;
    pthread_cond_t free_data_condition;
    pthread_cond_t data_to_process_condition;
    std::queue<unsigned char *> free_data;
    std::queue<unsigned char *> data_to_process;
    std::set<PacketReceiveCallbackInterface*> _callbacks;
};

#endif // _CONNECTION_LISTENER_H_
