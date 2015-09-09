#include "SpynnakerLiveSpikesConnection.h"
#include "ConnectionListener.h"
#include "EIEIOMessage.h"
#include <stddef.h>
#include <set>
#include <stdlib.h>
#include <stdio.h>

SpynnakerLiveSpikesConnection::SpynnakerLiveSpikesConnection(
        int n_receive_labels, char **receive_labels,
        int n_send_labels, char **send_labels,
        char *local_host, int local_port)
        : SpynnakerDatabaseConnection(this, this, local_host, local_port),
          StartCallbackInterface(){
    for (int i = 0; i < n_receive_labels; i++) {
        std::string receive_label(receive_labels[i]);
        this->receive_labels.push_back(receive_labels[i]);
        this->live_spike_callbacks[receive_label] =
            std::vector<SpikeReceiveCallbackInterface *>();
        this->start_callbacks[receive_label] =
            std::vector<SpikesStartCallbackInterface *>();
    }
    for (int i = 0; i < n_send_labels; i++) {
        std::string send_label(send_labels[i]);
        this->send_labels.push_back(send_labels[i]);
        this->start_callbacks[send_label] =
            std::vector<SpikesStartCallbackInterface *>();
    }
}

void SpynnakerLiveSpikesConnection::add_receive_callback(
        char *label, SpikeReceiveCallbackInterface *receive_callback) {
    this->live_spike_callbacks[std::string(label)].push_back(
        receive_callback);
}

void SpynnakerLiveSpikesConnection::add_start_callback(
        char *label, SpikesStartCallbackInterface *start_callback) {
    this->start_callbacks[std::string(label)].push_back(
        start_callback);
}

void SpynnakerLiveSpikesConnection::read_database_callback(
        DatabaseReader *reader) {
    // get input data
    for (int i = 0; i < this->send_labels.size(); i++) {
        char *send_label = this->send_labels[i];
        std::string send_label_str(send_label);

        reverse_ip_tag_info *receive_info =
            reader->get_live_input_details(send_label);
        this->send_address_details[send_label_str] = get_address(
            receive_info->board_address, receive_info->port);
        free(receive_info);
        this->neuron_id_to_key_maps[send_label_str] =
            reader->get_neuron_id_to_key_mapping(send_label);
    }
    std::set<int> ports_in_use;
    // get output data
    for (int i = 0; i < this->receive_labels.size(); i++) {
        char *receive_label = this->receive_labels[i];
        std::string receive_label_str(receive_label);

        ip_tag_info *send_info = reader->get_live_output_details(receive_label);

        if (send_info->strip_sdp) {
            std::set<int>::iterator value = ports_in_use.find(send_info->port);
            if (value == ports_in_use.end()) {
                UDPConnection *connection = new UDPConnection(send_info->port);
                ConnectionListener *listener =
                    new ConnectionListener(connection);
                listener->add_receive_packet_callback(this);
                listener->start();
                ports_in_use.insert(send_info->port);
            }
        } else {
            throw "Only tags which strip the SDP are supported";
        }

        free(send_info);
        // get key to neuron mapping for reciever translation
        std::map<int, int> *key_map = reader->get_key_to_neuron_id_mapping(
            receive_label);
        for (std::map<int, int>::iterator iter = key_map->begin();
                iter != key_map->end(); iter++) {

            struct label_and_neuron_id* item =
                new label_and_neuron_id(receive_label, iter->second);
            this->key_to_neuron_id_and_label_map[iter->first] = item;
        }
    }
}

struct start_callback_info {
    SpikesStartCallbackInterface *start_callback;
    char *label;
    SpynnakerLiveSpikesConnection *connection;

    start_callback_info(
            SpikesStartCallbackInterface *start_callback, char *label,
            SpynnakerLiveSpikesConnection *connection) {
        this->start_callback = start_callback;
        this->label = label;
        this->connection = connection;
    }
};

void SpynnakerLiveSpikesConnection::start_callback() {
    for (std::map<std::string,
                  std::vector<SpikesStartCallbackInterface *> >::iterator iter =
                      this->start_callbacks.begin();
            iter != this->start_callbacks.end(); iter++) {
        for (int i = 0; i < iter->second.size(); i++) {
            pthread_t callback_thread;
            struct start_callback_info start_info(
                iter->second[i], (char *) iter->first.c_str(), this);
            pthread_create(
                &callback_thread, NULL, _call_start_callback,
                (void *) &start_info);
        }
    }
}

void *SpynnakerLiveSpikesConnection::_call_start_callback(void *start_info) {
    struct start_callback_info *start_callback_info =
        (struct start_callback_info *) start_info;
    start_callback_info->start_callback->spikes_start(
        start_callback_info->label, start_callback_info->connection);
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
        std::map<int, struct label_and_neuron_id *>::iterator value =
            this->key_to_neuron_id_and_label_map.find(key);
        if (value != this->key_to_neuron_id_and_label_map.end()) {
            struct label_and_neuron_id *item = value->second;
            std::pair<int, std::string> key_time(key, std::string(item->label));
            std::map<std::pair<int, std::string>,
                     std::vector<int> *>::iterator key_times_labels_item =
                         key_times_labels.find(key_time);
            std::vector<int> *ids = NULL;
            if (key_times_labels_item != key_times_labels.end()) {
                ids = key_times_labels_item->second;
            } else {
                ids = new std::vector<int>();
                key_times_labels[key_time] = ids;
            }
            ids->push_back(item->neuron_id);
        }
    }
    for (std::map<std::pair<int, std::string>,
                  std::vector<int> *>::iterator iter =
                      key_times_labels.begin();
            iter != key_times_labels.end(); iter++) {
        int time = iter->first.first;
        std::string label = iter->first.second;
        std::vector<SpikeReceiveCallbackInterface *> callbacks =
            this->live_spike_callbacks[label];
        for (int i = 0; i < callbacks.size(); i++) {
            std::vector<int> *spikes = iter->second;
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
            EIEIOType type = KEY_32_BIT;
            EIEIOHeader* eieio_header =
                new EIEIOHeader(0, 0, 0, 0, type, 0, 0, 0);
            header = eieio_header;
        }
        else {
            EIEIOType type = KEY_16_BIT;
            EIEIOHeader* eieio_header =
                new EIEIOHeader(0, 0, 0, 0, type, 0, 0, 0);
            header = eieio_header;
        }
        EIEIOMessage message = EIEIOMessage(header);

        int spikes_in_packet = 0;

        // iterate till packet has been filled, or all spikes are consumed
        while (pos < sizeof(neuron_ids) && spikes_in_packet < max_keys) {
            int key;
            if (send_full_keys) {
                std::map<int, int> * map = this->neuron_id_to_key_maps[label];
                key = (*map)[neuron_ids[pos]];
            }
            else{
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
        this->send_data_to(data, size,
                           this->send_address_details[std::string(label)]);
        free(data);
    }
}

void SpynnakerLiveSpikesConnection::send_spike(
    char* label, int neuron_id, bool send_full_keys) {
        // build a vector for the neuron_id, then call send spikes.
        std::vector<int> neuron_ids;
        neuron_ids.push_back(neuron_id);
        send_spikes(label, neuron_ids, send_full_keys);
}

SpynnakerLiveSpikesConnection::~SpynnakerLiveSpikesConnection(){
    //Do nowt
}