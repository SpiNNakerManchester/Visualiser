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
