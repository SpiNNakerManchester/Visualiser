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
    virtual ~ConnectionListener();
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
    private:
        unsigned char *getDataToProcess(void);
        void releaseProcessedData(unsigned char *data);
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

    unsigned char *getFreeData(void);
    void postReceivedData(unsigned char *data);
};

#endif // _CONNECTION_LISTENER_H_
