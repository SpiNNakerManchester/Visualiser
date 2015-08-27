#include "UDPConnection.h"
#include "Threadable.h"
#include "EIEIOMessage.h"

#ifndef _CONNECTION_LISTENER_H_
#define _CONNECTION_LISTENER_H_

class PacketReceiveCallbackInterface {
public:
    virtual void receive_packet_callback(EIEIOMessage *message) = 0;
    virtual ~PacketReceiveCallbackInterface();
};

class ConnectionListener : public Threadable {
public:
    ConnectionListener(UDPConnection *connection);
    void add_receive_packet_callback(
        PacketReceiveCallbackInterface *packet_callback);

protected:
    void run();
};

#endif // _CONNECTION_LISTENER_H_
