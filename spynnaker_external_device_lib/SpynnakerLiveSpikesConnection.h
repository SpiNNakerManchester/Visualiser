#include "SpynnakerDatabaseConnection.h"
#include "DatabaseReader.h"
#include "Threadable.h"
#include "ConnectionListener.h"
#include "EIEIOMessage.h"

#include <string>
#include <vector>
#include <map>

#ifndef _SPYNNAKER_LIVE_SPIKES_CONNECTION_H_
#define _SPYNNAKER_LIVE_SPIKES_CONNECTION_H_

class SpynnakerLiveSpikesConnection;

struct label_and_neuron_id {
    char *label;
    int neuron_id;

    label_and_neuron_id(char *label, int neuron_id) {
        this->label = label;
        this->neuron_id = neuron_id;
    }
};

// The maximum number of 32-bit keys that will fit in a packet
const int MAX_FULL_KEYS_PER_PACKET = 63;

// The maximum number of 16-bit keys that will fit in a packet
const int MAX_HALF_KEYS_PER_PACKET = 127;

class SpikeReceiveCallbackInterface {
public:
    virtual void receive_spikes(
        char *label, int time, int n_spikes, int* spikes) = 0;
    virtual ~SpikeReceiveCallbackInterface();
};

class SpikesStartCallbackInterface {
public:
    virtual void spikes_start(char *label,
                              SpynnakerLiveSpikesConnection *connection);
    virtual ~SpikesStartCallbackInterface();
};

class SpynnakerLiveSpikesConnection :
        private SpynnakerDatabaseConnection, private DatabaseCallbackInterface,
        private StartCallbackInterface, private PacketReceiveCallbackInterface {
public:
    SpynnakerLiveSpikesConnection(
        int n_receive_labels=0, char **receive_labels=NULL,
        int n_send_labels=0, char **send_labels=NULL,
        char *local_host=NULL, int local_port=0);
    void add_receive_callback(
        char *label, SpikeReceiveCallbackInterface *receive_callback);
    void add_start_callback(
        char *label, SpikesStartCallbackInterface *start_callback);
    void send_spike(
        char *label, int neuron_id, bool send_full_keys=false);
    void send_spikes(
        char *label, std::vector<int> n_neuron_ids, int *neuron_ids,
        bool send_full_keys=false);
    ~SpynnakerLiveSpikesConnection();

protected:
    void read_database_callback(DatabaseReader *reader);
    void start_callback();
    void receive_packet_callback(EIEIOMessage *message);

private:
    static void *_call_start_callback(void *start_callback_info);

    std::vector<char *> receive_labels;
    std::vector<char *> send_labels;
    std::map<std::string, struct sockaddr *> send_address_details;
    std::map<std::string, std::map<int, int> *> neuron_id_to_key_maps;
    std::map<int, struct label_and_neuron_id *> key_to_neuron_id_and_label_map;
    std::map<std::string, std::vector<
             SpikeReceiveCallbackInterface *> > live_spike_callbacks;
    std::map<std::string, std::vector<
             SpikesStartCallbackInterface *> > start_callbacks;
};

#endif /* _SPYNNAKER_LIVE_SPIKES_CONNECTION_H_ */
