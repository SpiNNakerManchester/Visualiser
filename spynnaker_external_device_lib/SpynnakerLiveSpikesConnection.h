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
#include "SpynnakerDatabaseConnection.h"
#include "DatabaseReader.h"
#include "Threadable.h"
#include "ConnectionListener.h"
#include "EIEIOMessage.h"
#include "SDPMessage.h"

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

typedef struct send_details {
    struct sockaddr *address;
    placement *plmnt;

    send_details(struct sockaddr *address, placement *placement) {
        this->address = address;
        this->plmnt = placement;
    }
} send_details;

typedef struct rate_details {
    int neuron_id;
    float rate;
} rate_details;

typedef struct payload_details {
    int neuron_id;
    int payload;
} payload_details;

// The maximum number of 32-bit keys that will fit in a packet
const int _MAX_FULL_KEYS_PER_PACKET = 63;

// The maximum number of 16-bit keys that will fit in a packet
const int _MAX_HALF_KEYS_PER_PACKET = 127;

// The maximum number of 32-bit keys and payloads that will fit in a packet
const int _MAX_FULL_KEYS_PAYLOADS_PER_PACKET = 31;


class SpikeInitializeCallbackInterface {
public:
    virtual void init_population(
        char *label, int n_neurons, float run_time_ms,
        float machine_time_step_ms) = 0;
    virtual ~SpikeInitializeCallbackInterface() {};
};


class SpikeReceiveCallbackInterface {
public:
    virtual void receive_spikes(
        char *label, int time, int n_spikes, int* spikes) = 0;
    virtual ~SpikeReceiveCallbackInterface() {};
};


class SpikesStartCallbackInterface {
public:
    virtual void spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection) = 0;
    virtual ~SpikesStartCallbackInterface() {};
};

class SpikesPauseStopCallbackInterface {
public:
    virtual void spikes_stop(
        char *label, SpynnakerLiveSpikesConnection *connection) = 0;
    virtual ~SpikesPauseStopCallbackInterface() {};
};

class PayloadReceiveCallbackInterface {
public:
    virtual void receive_payloads(
        char *label, int n_payloads, payload_details *payloads) = 0;
    virtual ~PayloadReceiveCallbackInterface() {};
};


class SpynnakerLiveSpikesConnection :
        public SpynnakerDatabaseConnection, public DatabaseCallbackInterface,
        public StartCallbackInterface, public PacketReceiveCallbackInterface,
        public PauseStopCallbackInterface {
public:
    SpynnakerLiveSpikesConnection(
        int n_receive_labels=0, char **receive_labels=NULL,
        int n_send_labels=0, char **send_labels=NULL,
        char *local_host=NULL, int local_port=0,
        bool wait_for_start=false);
    void add_initialize_callback(
        char *label, SpikeInitializeCallbackInterface *init_callback);
    void add_receive_callback(
        char *label, SpikeReceiveCallbackInterface *receive_callback);
    void add_receive_callback(
        char *label, PayloadReceiveCallbackInterface *receive_callback);
    void add_start_callback(
        char *label, SpikesStartCallbackInterface *start_callback);
    void add_wait_for_start(char *label);
    void add_pause_stop_callback(
        char *label, SpikesPauseStopCallbackInterface *pause_stop_callback);
    void send_spike(
        char *label, int neuron_id, bool send_full_keys=false);
    void send_spikes(
        char *label, std::vector<int> n_neuron_ids, bool send_full_keys=false);
    void send_rate(char *label, int neuron_id, float rate);
    void send_rates(char *label, std::vector<rate_details> details);
    void send_start(char *label=NULL);
    void continue_run();
    ~SpynnakerLiveSpikesConnection();
    virtual void read_database_callback(DatabaseReader *reader);
    virtual void start_callback();
    virtual void pause_stop_callback();
    virtual void receive_packet_callback(EIEIOMessage *message);

private:
    static void *_call_start_callback(void *start_callback_info);
    static void *_call_pause_stop_callback(void *pause_stop_info);
    void handle_time_packet(EIEIOMessage *message);
    void handle_no_time_packet(EIEIOMessage *message);

    std::vector<char *> receive_labels;
    std::vector<char *> send_labels;
    std::map<std::string, send_details *> send_address_details;
    std::map<std::string, std::map<int, int> *> neuron_id_to_key_maps;
    std::map<int, struct label_and_neuron_id *> key_to_neuron_id_and_label_map;
    std::map<std::string, std::vector<
             SpikeInitializeCallbackInterface *> > init_callbacks;
    std::map<std::string, std::vector<
             SpikeReceiveCallbackInterface *> > live_spike_callbacks;
    std::map<std::string, std::vector<
             PayloadReceiveCallbackInterface *> > live_payload_callbacks;
    std::map<std::string, std::vector<
             SpikesStartCallbackInterface *> > start_callbacks;
    std::map<std::string, std::vector<
             SpikesPauseStopCallbackInterface *> > pause_stop_callbacks;

    pthread_cond_t start_condition;
    pthread_mutex_t start_mutex;
    std::map<std::string, bool> waiting_for_start;
    int n_waiting_for_start;
    UDPConnection *receiver_connection;
    ConnectionListener *listener;
    struct sockaddr *root_chip_address;
    unsigned char app_id;
    sync_type next_sync;
};

#endif /* _SPYNNAKER_LIVE_SPIKES_CONNECTION_H_ */
