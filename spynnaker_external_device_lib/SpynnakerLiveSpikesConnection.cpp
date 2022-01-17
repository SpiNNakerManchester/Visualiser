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
#include "SpynnakerLiveSpikesConnection.h"
#include "ConnectionListener.h"
#include "EIEIOMessage.h"
#include "SDPMessage.h"
#include <stddef.h>
#include <set>
#include <stdlib.h>
#include <stdio.h>

SpynnakerLiveSpikesConnection::SpynnakerLiveSpikesConnection(
        int n_receive_labels, char **receive_labels,
        int n_send_labels, char **send_labels,
        char *local_host, int local_port, bool wait_for_start)
        : SpynnakerDatabaseConnection(this, this, local_host, local_port),
          StartCallbackInterface(), PauseStopCallbackInterface() {
    this->add_database_callback(this);
    for (int i = 0; i < n_receive_labels; i++) {
        std::string receive_label(receive_labels[i]);
        this->receive_labels.push_back(receive_labels[i]);
        this->live_spike_callbacks[receive_label] =
            std::vector<SpikeReceiveCallbackInterface *>();
        this->live_payload_callbacks[receive_label] =
            std::vector<PayloadReceiveCallbackInterface *>();
        this->start_callbacks[receive_label] =
            std::vector<SpikesStartCallbackInterface *>();
        this->waiting_for_start[receive_label] = wait_for_start;
        this->pause_stop_callbacks[receive_label] =
            std::vector<SpikesPauseStopCallbackInterface *>();
    }
    for (int i = 0; i < n_send_labels; i++) {
        std::string send_label(send_labels[i]);
        this->send_labels.push_back(send_labels[i]);
        this->start_callbacks[send_label] =
            std::vector<SpikesStartCallbackInterface *>();
        this->pause_stop_callbacks[send_label] =
            std::vector<SpikesPauseStopCallbackInterface *>();
        this->waiting_for_start[send_label] = wait_for_start;
    }
    this->n_waiting_for_start = 0;
    if (wait_for_start) {
        this->n_waiting_for_start = n_send_labels + n_receive_labels;
    }
    if (pthread_mutex_init(&(this->start_mutex), NULL) == -1) {
        fprintf(stderr, "Error initializing live start mutex\n");
        exit(-1);
    }
    if (pthread_cond_init(&(this->start_condition), NULL) == -1) {
        fprintf(stderr, "Error initializing live start condition\n");
        exit(-1);
    }
    this->receiver_connection = NULL;
    this->listener = NULL;
    this->next_sync = SYNC_1;
    this->root_chip_address = NULL;
    this->app_id = 0;
}

void SpynnakerLiveSpikesConnection::add_initialize_callback(
        char *label, SpikeInitializeCallbackInterface *init_callback) {
    this->init_callbacks[std::string(label)].push_back(init_callback);
}

void SpynnakerLiveSpikesConnection::add_receive_callback(
        char *label, SpikeReceiveCallbackInterface *receive_callback) {
    this->live_spike_callbacks[std::string(label)].push_back(
        receive_callback);
}

void SpynnakerLiveSpikesConnection::add_receive_callback(
        char *label, PayloadReceiveCallbackInterface *receive_callback) {
    this->live_payload_callbacks[std::string(label)].push_back(
        receive_callback);
}

void SpynnakerLiveSpikesConnection::add_start_callback(
        char *label, SpikesStartCallbackInterface *start_callback) {
    this->start_callbacks[std::string(label)].push_back(
        start_callback);
}

void SpynnakerLiveSpikesConnection::add_pause_stop_callback(
        char *label, SpikesPauseStopCallbackInterface *pause_stop_callback) {
    this->pause_stop_callbacks[std::string(label)].push_back(
        pause_stop_callback);
}

void SpynnakerLiveSpikesConnection::add_wait_for_start(char *label) {
    pthread_mutex_lock(&(this->start_mutex));
    this->waiting_for_start[std::string(label)] = true;
    this->n_waiting_for_start += 1;
    pthread_mutex_unlock(&(this->start_mutex));
}

void SpynnakerLiveSpikesConnection::read_database_callback(
        DatabaseReader *reader) {

    float run_time_ms = reader->get_configuration_parameter_value(
        (char *) "runtime");
    float machine_time_step_ms = reader->get_configuration_parameter_value(
        (char *) "machine_time_step") / 1000.0;

    std::map<std::string, int> population_sizes;

    // get input data
    for (int i = 0; i < this->send_labels.size(); i++) {
        char *send_label = this->send_labels[i];
        std::string send_label_str(send_label);

        std::vector<placement *> *placements = reader->get_placements(send_label);
        placement *plmnt = (*placements)[0];
        for (uint32_t i = 1; i < placements->size(); i++) {
            placement *p = (*placements)[i];
            free(p);
        }
        delete placements;
        char *ip_address = reader->get_ip_address(plmnt->x, plmnt->y);
        struct sockaddr *addr = get_address(ip_address, SCP_SCAMP_PORT);
        free(ip_address);
        this->send_address_details[send_label_str] = new send_details(addr, plmnt);
        this->neuron_id_to_key_maps[send_label_str] =
            reader->get_neuron_id_to_key_mapping(send_label);
        population_sizes[send_label_str] =
            this->neuron_id_to_key_maps[send_label_str]->size();
    }

    // get output data
    if (this->receive_labels.size() > 0) {
        this->receiver_connection = new UDPConnection();
        for (int i = 0; i < this->receive_labels.size(); i++) {
            char *receive_label = this->receive_labels[i];
            std::string receive_label_str(receive_label);

            // Update the IP Tag to ensure reception
            try {
                ip_tag_info *recv_info = reader->get_live_output_details(receive_label);
                SCPIPTagSetMessage msg = SCPIPTagSetMessage(
                        255, 255, recv_info->tag, recv_info->strip_sdp);
                struct sockaddr *s_addr = get_address(recv_info->board_address,
                        SCP_SCAMP_PORT);
                unsigned char data[sizeof(SCPIPTagSetMessage) + 2];
                memcpy(&data[2], &msg, sizeof(SCPIPTagSetMessage));
                data[0] = 0;
                data[1] = 0;
                this->receiver_connection->send_data_to(
                        data, sizeof(SCPIPTagSetMessage) + 2, s_addr);
                this->receiver_connection->receive_data(data, sizeof(SCPIPTagSetMessage));
                free(recv_info);
                free(s_addr);
            } catch (const char * str) {
                fprintf(stderr, "Error sending Tag update to SpiNNaker, continuing\n");
            }

            // get key to neuron mapping for reciever translation
            std::map<int, int> *key_map = reader->get_key_to_neuron_id_mapping(
                receive_label);
            for (std::map<int, int>::iterator iter = key_map->begin();
                    iter != key_map->end(); iter++) {

                struct label_and_neuron_id* item =
                    new label_and_neuron_id(receive_label, iter->second);
                this->key_to_neuron_id_and_label_map[iter->first] = item;
            }

            population_sizes[receive_label_str] = key_map->size();
        }

        this->listener = new ConnectionListener(this->receiver_connection);
        listener->add_receive_packet_callback(this);
        listener->start();
    }

    for (std::map<std::string, int>::iterator iter =
            population_sizes.begin(); iter != population_sizes.end();
            iter++) {
        std::string pop_label = iter->first;
        int n_neurons = iter->second;
        std::vector<SpikeInitializeCallbackInterface *> init_callbacks =
            this->init_callbacks[pop_label];
        for (int i = 0; i < init_callbacks.size(); i++) {
            init_callbacks[i]->init_population(
                (char *) pop_label.c_str(), n_neurons, run_time_ms,
                machine_time_step_ms);
        }
    }

    // Get the address of the root chip
    char *root_ip = reader->get_ip_address(0, 0);
    this->root_chip_address = get_address(root_ip, SCP_SCAMP_PORT);
    this->app_id = (int) reader->get_configuration_parameter_value(
            (char *) std::string("app_id").c_str());
    free(root_ip);

    pthread_mutex_lock(&(this->start_mutex));
    while (this->n_waiting_for_start > 0) {
        pthread_cond_wait(&(this->start_condition), &(this->start_mutex));
    }
    pthread_mutex_unlock(&(this->start_mutex));
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
    for (std::map<
                std::string,
                std::vector<SpikesStartCallbackInterface *> >::iterator iter =
                    this->start_callbacks.begin();
            iter != this->start_callbacks.end(); iter++) {
        for (int i = 0; i < iter->second.size(); i++) {
            pthread_t callback_thread;
            char *label = (char *) iter->first.c_str();
            SpikesStartCallbackInterface *callback = iter->second[i];
            struct start_callback_info *start_info =
                new struct start_callback_info(callback, label, this);
            pthread_create(
                &callback_thread, NULL, _call_start_callback,
                (void *) start_info);
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


void SpynnakerLiveSpikesConnection::pause_stop_callback() {
    for (std::map<
                std::string,
                std::vector<SpikesPauseStopCallbackInterface *> >::iterator
                    iter = this->pause_stop_callbacks.begin();
            iter != this->pause_stop_callbacks.end(); iter++) {
        for (int i = 0; i < iter->second.size(); i++) {
            pthread_t callback_thread;
            char *label = (char *) iter->first.c_str();
            SpikesPauseStopCallbackInterface *callback = iter->second[i];
            struct pause_stop_callback_info *pause_stop_info =
                new struct pause_stop_callback_info(callback, label, this);
            pthread_create(
                &callback_thread, NULL, _call_pause_stop_callback,
                (void *) pause_stop_info);
        }
    }
}


void *SpynnakerLiveSpikesConnection::_call_pause_stop_callback(
        void *pause_stop_info) {
    struct pause_stop_callback_info *pause_stop_callback_info =
        (struct pause_stop_callback_info *) pause_stop_info;
    pause_stop_callback_info->pause_stop_callback->spikes_stop(
        pause_stop_callback_info->label, pause_stop_callback_info->connection);
    return NULL;
}


void SpynnakerLiveSpikesConnection::receive_packet_callback(
        EIEIOMessage *message) {
    if (message->has_timestamps()) {
        handle_time_packet(message);
    } else {
        handle_no_time_packet(message);
    }
}

void SpynnakerLiveSpikesConnection::handle_time_packet(
        EIEIOMessage *message) {
    std::map<std::pair<int, std::string>, std::vector<int> *> key_times_labels;
    while (message->is_next_element()) {
        EIEIOElement* element = message->get_next_element();
        int time = element->get_payload();
        int key = element->get_key();
        std::map<int, struct label_and_neuron_id *>::iterator value =
            this->key_to_neuron_id_and_label_map.find(key);
        if (value != this->key_to_neuron_id_and_label_map.end()) {
            struct label_and_neuron_id *item = value->second;
            std::pair<int, std::string> time_label(
                time, std::string(item->label));
            std::map<std::pair<int, std::string>,
                     std::vector<int> *>::iterator key_times_labels_item =
                         key_times_labels.find(time_label);
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

void SpynnakerLiveSpikesConnection::handle_no_time_packet(
        EIEIOMessage *message) {
    while (message->is_next_element()) {
        EIEIOElement* element = message->get_next_element();
        int payload = element->get_payload();
        int key = element->get_key();
        std::map<int, struct label_and_neuron_id *>::iterator value =
            this->key_to_neuron_id_and_label_map.find(key);
        if (value != this->key_to_neuron_id_and_label_map.end()) {
            struct label_and_neuron_id *item = value->second;
            std::vector<PayloadReceiveCallbackInterface *> callbacks =
                        this->live_payload_callbacks[item->label];
            for (int i = 0; i < callbacks.size(); i++) {
                callbacks[i]->receive_payload(
                    item->label, item->neuron_id, payload);
            }
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
        while (pos < neuron_ids.size() && spikes_in_packet < max_keys) {
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

        // locate details for where to send this to
        send_details *details = this->send_address_details[std::string(label)];

        unsigned char * data = (unsigned char *) malloc(
                message.get_max_size() + sizeof(SDPHeader) + 2);
        int size = message.get_data(&data[sizeof(SDPHeader) + 2]) + sizeof(SDPHeader) + 2;
        SDPHeader *sdp_header = (SDPHeader *) &data[2];
        sdp_header->set_no_reply(
                0, details->plmnt->x, details->plmnt->y, details->plmnt->p, 1);
        this->send_data_to(data, size, details->address);
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

void SpynnakerLiveSpikesConnection::send_rates(
    char *label, std::vector<rate_details> details){

    // figure max spikes size for each message
    int max_keys = _MAX_FULL_KEYS_PAYLOADS_PER_PACKET;

    // iterate till all spikes have been consumed
    int pos = 0;
    while (pos < details.size()) {

        EIEIOHeader * header;
        EIEIOType type = KEY_PAYLOAD_32_BIT;
        EIEIOHeader* eieio_header =
            new EIEIOHeader(0, 0, 0, 0, type, 0, 0, 0);
        header = eieio_header;
        EIEIOMessage message = EIEIOMessage(header);

        int n_in_packet = 0;

        // iterate till packet has been filled, or all spikes are consumed
        while (pos < details.size() && n_in_packet < max_keys) {
            int key;
            std::map<int, int> * map = this->neuron_id_to_key_maps[label];
            key = (*map)[details[pos].neuron_id];
            int rate = (int) (details[pos].rate * 32768.0f);
            message.add_key_and_payload(key, rate);
            message.increment_count();
            pos += 1;
            n_in_packet += 1;
        }

        // locate details for where to send this to
        send_details *details = this->send_address_details[std::string(label)];

        unsigned char * data = (unsigned char *) malloc(
                message.get_max_size() + sizeof(SDPHeader) + 2);
        int size = message.get_data(&data[sizeof(SDPHeader) + 2]) + sizeof(SDPHeader) + 2;
        SDPHeader *sdp_header = (SDPHeader *) &data[2];
        sdp_header->set_no_reply(
                0, details->plmnt->x, details->plmnt->y, details->plmnt->p, 1);
        this->send_data_to(data, size, details->address);
        free(data);
    }
}

void SpynnakerLiveSpikesConnection::send_rate(
    char* label, int neuron_id, float rate) {
        // build a vector for the neuron_id, then call send spikes.
        std::vector<rate_details> details;
        details.push_back({.neuron_id=neuron_id, .rate=rate});
        send_rates(label, details);
}

void SpynnakerLiveSpikesConnection::send_start(char *label) {
    if (label != NULL) {
        std::string label_str = std::string(label);
        pthread_mutex_lock(&(this->start_mutex));
        if (this->waiting_for_start[label_str]) {
            this->waiting_for_start[label_str] = false;
            this->n_waiting_for_start -= 1;
            pthread_cond_signal(&(this->start_condition));
        }
        pthread_mutex_unlock(&(this->start_mutex));
    } else {
        pthread_mutex_lock(&(this->start_mutex));
        this->waiting_for_start.clear();
        this->n_waiting_for_start = 0;
        pthread_cond_signal(&(this->start_condition));
        pthread_mutex_unlock(&(this->start_mutex));
    }
}

void SpynnakerLiveSpikesConnection::continue_run() {
    SCPSyncSignalMessage msg = SCPSyncSignalMessage(this->app_id, this->next_sync);
    if (this->next_sync == SYNC_1) {
        this->next_sync = SYNC_0;
    } else {
        this->next_sync = SYNC_1;
    }
    unsigned char data[sizeof(SCPSyncSignalMessage) + 2];
    memcpy(&data[2], &msg, sizeof(SCPSyncSignalMessage));
    data[0] = 0;
    data[1] = 0;
    UDPConnection conn = UDPConnection();
    conn.send_data_to(
            data, sizeof(SCPSyncSignalMessage) + 2, this->root_chip_address);
    conn.receive_data(data, sizeof(SCPSyncSignalMessage));
}

SpynnakerLiveSpikesConnection::~SpynnakerLiveSpikesConnection(){
pthread_mutex_destroy(&(this->start_mutex));
    pthread_cond_destroy(&(this->start_condition));
}
