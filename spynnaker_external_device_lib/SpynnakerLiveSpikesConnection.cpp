#include "SpynnakerLiveSpikesConnection.h"
#include "ConnectionListener.h"
#include "EIEIOMessage.h"
#include <stddef.h>
#include <set>
#include <stdlib.h>
#include <stdio.h>

static inline void lock(pthread_mutex_t &mutex) {
    pthread_mutex_lock(&mutex);
}

static inline void unlock(pthread_mutex_t &mutex) {
    pthread_mutex_unlock(&mutex);
}

static inline void wait(pthread_cond_t &cond, pthread_mutex_t &mutex) {
    pthread_cond_wait(&cond, &mutex);
}

static inline void signal(pthread_cond_t &cond) {
    pthread_cond_signal(&cond);
}

SpynnakerLiveSpikesConnection::SpynnakerLiveSpikesConnection(
        int n_receive_labels, char **receive_labels,
        int n_send_labels, char **send_labels,
        char *local_host, int local_port, bool wait_for_start)
        : SpynnakerDatabaseConnection(this, this, local_host, local_port),
          StartCallbackInterface(), PauseStopCallbackInterface() {
    add_database_callback(this);
    for (int i = 0; i < n_receive_labels; i++) {
        std::string receive_label(receive_labels[i]);
        this->receive_labels.push_back(receive_labels[i]);

        live_spike_callbacks[receive_label] =
            std::vector<SpikeReceiveCallbackInterface *>();
        start_callbacks[receive_label] =
            std::vector<SpikesStartCallbackInterface *>();
        waiting_for_start[receive_label] = wait_for_start;
        pause_stop_callbacks[receive_label] =
            std::vector<SpikesPauseStopCallbackInterface *>();
    }
    for (int i = 0; i < n_send_labels; i++) {
        std::string send_label(send_labels[i]);
        this->send_labels.push_back(send_labels[i]);

        start_callbacks[send_label] =
            std::vector<SpikesStartCallbackInterface *>();
        pause_stop_callbacks[send_label] =
            std::vector<SpikesPauseStopCallbackInterface *>();
        waiting_for_start[send_label] = wait_for_start;
    }
    n_waiting_for_start = 0;
    if (wait_for_start) {
        n_waiting_for_start = n_send_labels + n_receive_labels;
    }
    if (pthread_mutex_init(&start_mutex, NULL) == -1) {
        fprintf(stderr, "Error initializing live start mutex\n");
        exit(-1);
    }
    if (pthread_cond_init(&start_condition, NULL) == -1) {
        fprintf(stderr, "Error initializing live start condition\n");
        exit(-1);
    }
}

void SpynnakerLiveSpikesConnection::add_initialize_callback(
        char *label, SpikeInitializeCallbackInterface *init_callback) {
    init_callbacks[std::string(label)].push_back(init_callback);
}

void SpynnakerLiveSpikesConnection::add_receive_callback(
        char *label, SpikeReceiveCallbackInterface *receive_callback) {
    live_spike_callbacks[std::string(label)].push_back(receive_callback);
}

void SpynnakerLiveSpikesConnection::add_start_callback(
        char *label, SpikesStartCallbackInterface *start_callback) {
    start_callbacks[std::string(label)].push_back(start_callback);
}

void SpynnakerLiveSpikesConnection::add_pause_stop_callback(
        char *label, SpikesPauseStopCallbackInterface *pause_stop_callback) {
    pause_stop_callbacks[std::string(label)].push_back(pause_stop_callback);
}

void SpynnakerLiveSpikesConnection::add_wait_for_start(char *label) {
    lock(start_mutex);
    waiting_for_start[std::string(label)] = true;
    n_waiting_for_start += 1;
    unlock(start_mutex);
}

void SpynnakerLiveSpikesConnection::read_database_callback(
        DatabaseReader *reader) {
    float run_time_ms = reader->get_configuration_parameter_value(
        (char *) "runtime");
    float machine_time_step_ms = reader->get_configuration_parameter_value(
        (char *) "machine_time_step") / 1000.0;

    std::map<std::string, int> population_sizes;

    // get input data
    for (int i = 0; i < send_labels.size(); i++) {
        char *send_label = send_labels[i];
        std::string send_label_str(send_label);

        auto receive_info = reader->get_live_input_details(send_label);
        send_address_details[send_label_str] = get_address(
            receive_info->board_address, receive_info->port);
        free(receive_info);

        neuron_id_to_key_maps[send_label_str] =
            reader->get_neuron_id_to_key_mapping(send_label);
        population_sizes[send_label_str] =
            neuron_id_to_key_maps[send_label_str]->size();
    }

    // get output data
    std::set<int> ports_in_use;
    for (int i = 0; i < receive_labels.size(); i++) {
        char *receive_label = receive_labels[i];
        std::string receive_label_str(receive_label);

        auto send_info = reader->get_live_output_details(receive_label);
        if (!send_info->strip_sdp) {
            throw "Only tags which strip the SDP are supported";
        }
	auto value = ports_in_use.find(send_info->port);
	if (value == ports_in_use.end()) {
	    UDPConnection *connection = new UDPConnection(send_info->port);
	    auto listener = new ConnectionListener(connection);
	    listener->add_receive_packet_callback(this);
	    listener->start();
	    ports_in_use.insert(send_info->port);
	}
        free(send_info);

        // get key to neuron mapping for receiver translation
        auto key_map = reader->get_key_to_neuron_id_mapping(receive_label);
        for (auto iter = key_map->begin(); iter != key_map->end(); iter++) {
            auto item = new label_and_neuron_id(receive_label, iter->second);
            key_to_neuron_id_and_label_map[iter->first] = item;
        }

        population_sizes[receive_label_str] = key_map->size();
    }

    for (auto iter = population_sizes.begin();
	    iter != population_sizes.end(); iter++) {
        std::string pop_label = iter->first;
        int n_neurons = iter->second;
        auto init_callbacks = this->init_callbacks[pop_label];
        for (int i = 0; i < init_callbacks.size(); i++) {
            init_callbacks[i]->init_population(
                (char *) pop_label.c_str(), n_neurons, run_time_ms,
                machine_time_step_ms);
        }
    }

    lock(start_mutex);
    while (n_waiting_for_start > 0) {
        wait(start_condition, start_mutex);
    }
    unlock(start_mutex);
}

struct start_callback_info {
    SpikesStartCallbackInterface *start_callback;
    char *label;
    SpynnakerLiveSpikesConnection *connection;

    start_callback_info(
            SpikesStartCallbackInterface *start_callback,
            char *label, SpynnakerLiveSpikesConnection *connection) {
        this->start_callback = start_callback;
        this->label = label;
        this->connection = connection;
    }
};

struct pause_stop_callback_info {
    SpikesPauseStopCallbackInterface *pause_stop_callback;
    char *label;
    SpynnakerLiveSpikesConnection *connection;

    pause_stop_callback_info(
            SpikesPauseStopCallbackInterface *pause_stop_callback,
            char *label, SpynnakerLiveSpikesConnection *connection) {
        this->pause_stop_callback = pause_stop_callback;
        this->label = label;
        this->connection = connection;
    }
};


void SpynnakerLiveSpikesConnection::start_callback() {
    for (auto iter = start_callbacks.begin();
            iter != start_callbacks.end(); iter++) {
        for (int i = 0; i < iter->second.size(); i++) {
            pthread_t callback_thread;
            char *label = (char *) iter->first.c_str();
            SpikesStartCallbackInterface *callback = iter->second[i];
            auto start_info = new start_callback_info(
        	    callback, label, this);
            pthread_create(
                &callback_thread, NULL, _call_start_callback,
                (void *) start_info);
        }
    }
}


void *SpynnakerLiveSpikesConnection::_call_start_callback(void *start_info) {
    auto info = (start_callback_info *) start_info;
    info->start_callback->spikes_start(info->label, info->connection);
    return NULL;
}


void SpynnakerLiveSpikesConnection::pause_stop_callback() {
    for (auto iter = pause_stop_callbacks.begin();
	    iter != pause_stop_callbacks.end(); iter++) {
        for (int i = 0; i < iter->second.size(); i++) {
            pthread_t callback_thread;
            char *label = (char *) iter->first.c_str();
            auto callback = iter->second[i];
            auto pause_stop_info = new struct pause_stop_callback_info(
        	    callback, label, this);
            pthread_create(
                &callback_thread, NULL, _call_pause_stop_callback,
                (void *) pause_stop_info);
        }
    }
}


void *SpynnakerLiveSpikesConnection::_call_pause_stop_callback(
        void *pause_stop_info) {
    auto info = (pause_stop_callback_info *) pause_stop_info;
    info->pause_stop_callback->spikes_stop(info->label, info->connection);
    return NULL;
}


void SpynnakerLiveSpikesConnection::receive_packet_callback(
        EIEIOMessage *message) {
    if (!message->has_timestamps()) {
        throw "Only packets with a timestamp are considered";
    }
    std::map<std::pair<int, std::string>, std::vector<int> *> key_times_labels;
    while (message->is_next_element()) {
        EIEIOElement* element = message->get_next_element();
        int time = element->get_payload();
        int key = element->get_key();
        auto value = key_to_neuron_id_and_label_map.find(key);
        if (value != key_to_neuron_id_and_label_map.end()) {
            auto item = value->second;
            std::pair<int, std::string> time_label(
                time, std::string(item->label));
            auto key_times_labels_item = key_times_labels.find(time_label);
            std::vector<int> *ids = NULL;
            if (key_times_labels_item != key_times_labels.end()) {
                ids = key_times_labels_item->second;
            } else {
                ids = new std::vector<int>();
                key_times_labels[time_label] = ids;
            }
            ids->push_back(item->neuron_id);
        }
    }
    for (auto iter = key_times_labels.begin();
            iter != key_times_labels.end(); iter++) {
        int time = iter->first.first;
        std::string label = iter->first.second;
        auto callbacks = live_spike_callbacks[label];
        for (int i = 0; i < callbacks.size(); i++) {
            auto spikes = iter->second;
            callbacks[i]->receive_spikes(
                (char *) label.c_str(), time, spikes->size(), &((*spikes)[0]));
        }
    }
}

void SpynnakerLiveSpikesConnection::send_spikes(
    char *label, std::vector<int> neuron_ids, bool send_full_keys){

    // figure max spikes size for each message
    int max_keys = _MAX_HALF_KEYS_PER_PACKET;
    if (send_full_keys) {
        max_keys = _MAX_FULL_KEYS_PER_PACKET;
    }

    // iterate till all spikes have been consumed
    int pos = 0;
    while (pos < neuron_ids.size()) {

        EIEIOHeader * header;
        if (send_full_keys) {
            header = new EIEIOHeader(0, 0, 0, 0, KEY_32_BIT, 0, 0, 0);
        } else {
            header = new EIEIOHeader(0, 0, 0, 0, KEY_16_BIT, 0, 0, 0);
        }
        EIEIOMessage message = EIEIOMessage(header);

        int spikes_in_packet = 0;

        // iterate till packet has been filled, or all spikes are consumed
        while (pos < neuron_ids.size() && spikes_in_packet < max_keys) {
            int key;
            if (send_full_keys) {
                auto map = neuron_id_to_key_maps[label];
                key = (*map)[neuron_ids[pos]];
            } else {
                key = neuron_ids[pos];
            }
            message.add_key(key);
            message.increment_count();
            pos += 1;
            spikes_in_packet += 1;
        }

        //locate socket details for where to send this to
        unsigned char * data = (unsigned char *) malloc(message.get_max_size());
        int size = message.get_data(data);
        send_data_to(data, size, send_address_details[std::string(label)]);
        free(data);
    }
}

void SpynnakerLiveSpikesConnection::send_spike(
    char* label, int neuron_id, bool send_full_keys)
{
    // build a vector for the neuron_id, then call send spikes.
    std::vector<int> neuron_ids;
    neuron_ids.push_back(neuron_id);
    send_spikes(label, neuron_ids, send_full_keys);
}

void SpynnakerLiveSpikesConnection::send_start(char *label) {
    if (label != NULL) {
        std::string label_str = std::string(label);
        lock(start_mutex);
        if (waiting_for_start[label_str]) {
            waiting_for_start[label_str] = false;
            n_waiting_for_start -= 1;
            signal(start_condition);
        }
        unlock(start_mutex);
    } else {
        lock(start_mutex);
        waiting_for_start.clear();
        n_waiting_for_start = 0;
        signal(start_condition);
        unlock(start_mutex);
    }
}

SpynnakerLiveSpikesConnection::~SpynnakerLiveSpikesConnection(){
    pthread_mutex_destroy(&start_mutex);
    pthread_cond_destroy(&start_condition);
}
