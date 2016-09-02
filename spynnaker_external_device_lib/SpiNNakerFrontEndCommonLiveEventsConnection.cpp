#include "SpiNNakerFrontEndCommonLiveEventsConnection.h"
#include "ConnectionListener.h"
#include "EIEIOMessage.h"
#include <stddef.h>
#include <set>
#include <stdlib.h>
#include <stdio.h>

SpiNNakerFrontEndCommonLiveEventsConnection::
    SpiNNakerFrontEndCommonLiveEventsConnection(
        int n_receive_labels, char **receive_labels,
        int n_send_labels, char **send_labels,
        char *local_host, int local_port, bool wait_for_start)
        : SpynnakerDatabaseConnection(this, local_host, local_port),
          StartCallbackInterface() {
    this->add_database_callback(this);
    for (int i = 0; i < n_receive_labels; i++) {
        std::string receive_label(receive_labels[i]);
        this->receive_labels.push_back(receive_labels[i]);
        this->live_event_callbacks[receive_label] =
            std::vector<EventReceiveCallbackInterface *>();
        this->start_callbacks[receive_label] =
            std::vector<EventsStartCallbackInterface *>();
        this->waiting_for_start[receive_label] = wait_for_start;
    }
    for (int i = 0; i < n_send_labels; i++) {
        std::string send_label(send_labels[i]);
        this->send_labels.push_back(send_labels[i]);
        this->start_callbacks[send_label] =
            std::vector<EventsStartCallbackInterface *>();
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
}

void SpiNNakerFrontEndCommonLiveEventsConnection::add_initialize_callback(
        char *label, EventInitializeCallbackInterface *init_callback) {
    this->init_callbacks[std::string(label)].push_back(init_callback);
}

void SpiNNakerFrontEndCommonLiveEventsConnection::add_receive_callback(
        char *label, EventReceiveCallbackInterface *receive_callback) {
    this->live_event_callbacks[std::string(label)].push_back(
        receive_callback);
}

void SpiNNakerFrontEndCommonLiveEventsConnection::add_receive_callback_payload(
        char *label, EventReceiveCallbackPayloadInterface *receive_callback) {
    this->live_event_payload_callbacks[std::string(label)].push_back(
        receive_callback);
}

void SpiNNakerFrontEndCommonLiveEventsConnection::add_start_callback(
        char *label, EventsStartCallbackInterface *start_callback) {
    this->start_callbacks[std::string(label)].push_back(
        start_callback);
}

void SpiNNakerFrontEndCommonLiveEventsConnection::
        add_wait_for_start(char *label) {
    pthread_mutex_lock(&(this->start_mutex));
    this->waiting_for_start[std::string(label)] = true;
    this->n_waiting_for_start += 1;
    pthread_mutex_unlock(&(this->start_mutex));
}

void SpiNNakerFrontEndCommonLiveEventsConnection::read_database_callback(
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

        reverse_ip_tag_info *receive_info =
            reader->get_live_input_details(send_label);
        this->send_address_details[send_label_str] = get_address(
            receive_info->board_address, receive_info->port);
        free(receive_info);
        this->atom_id_to_key_maps[send_label_str] =
            reader->get_atom_id_to_key_mapping(send_label);
        population_sizes[send_label_str] =
            this->atom_id_to_key_maps[send_label_str]->size();
    }

    // get output data
    std::set<int> ports_in_use;
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

        // get key to atom mapping for receiver translation
        std::map<int, int> *key_map = reader->get_key_to_atom_id_mapping(
            receive_label);
        for (std::map<int, int>::iterator iter = key_map->begin();
                iter != key_map->end(); iter++) {

            struct label_and_atom_id* item =
                new label_and_atom_id(receive_label, iter->second);
            this->key_to_atom_id_and_label_map[iter->first] = item;
        }

        population_sizes[receive_label_str] = key_map->size();
    }

    for (std::map<std::string, int>::iterator iter =
            population_sizes.begin(); iter != population_sizes.end();
            iter++) {
        std::string pop_label = iter->first;
        int n_atoms = iter->second;
        std::vector<EventInitializeCallbackInterface *> init_callbacks =
            this->init_callbacks[pop_label];
        for (int i = 0; i < init_callbacks.size(); i++) {
            init_callbacks[i]->init_population(
                (char *) pop_label.c_str(), n_atoms, run_time_ms,
                machine_time_step_ms);
        }
    }

    pthread_mutex_lock(&(this->start_mutex));
    while (this->n_waiting_for_start > 0) {
        pthread_cond_wait(&(this->start_condition), &(this->start_mutex));
    }
    pthread_mutex_unlock(&(this->start_mutex));
}

struct start_callback_info {
    EventsStartCallbackInterface *start_callback;
    char *label;
    SpiNNakerFrontEndCommonLiveEventsConnection *connection;

    start_callback_info(
            EventsStartCallbackInterface *start_callback,
            char *label,
            SpiNNakerFrontEndCommonLiveEventsConnection *connection) {
        this->start_callback = start_callback;
        this->label = label;
        this->connection = connection;
    }
};

void SpiNNakerFrontEndCommonLiveEventsConnection::start_callback() {
    for (std::map<
                std::string,
                std::vector<EventsStartCallbackInterface *> >::iterator iter =
                    this->start_callbacks.begin();
            iter != this->start_callbacks.end(); iter++) {
        for (int i = 0; i < iter->second.size(); i++) {
            pthread_t callback_thread;
            char *label = (char *) iter->first.c_str();
            EventsStartCallbackInterface *callback = iter->second[i];
            struct start_callback_info *start_info =
                new struct start_callback_info(callback, label, this);
            pthread_create(
                &callback_thread, NULL, _call_start_callback,
                (void *) start_info);
        }
    }
}

void *SpiNNakerFrontEndCommonLiveEventsConnection::_call_start_callback(
         void *start_info) {
    struct start_callback_info *start_callback_info =
        (struct start_callback_info *) start_info;
    start_callback_info->start_callback->events_start(
        start_callback_info->label, start_callback_info->connection);
    return NULL;
}

void SpiNNakerFrontEndCommonLiveEventsConnection::_process_payload_message(
        EIEIOMessage *message){
    while (message->is_next_element()) {
        EIEIOElement* element = message->get_next_element();
        int payload = element->get_payload();
        int key = element->get_key();
        std::map<int, struct label_and_atom_id *>::iterator value =
            this->key_to_atom_id_and_label_map.find(key);
        if (value != this->key_to_atom_id_and_label_map.end()) {
            struct label_and_atom_id *item = value->second;
            int atom_id = item->atom_id;
            std::vector<EventReceiveCallbackPayloadInterface *> callbacks =
                this->live_event_payload_callbacks[item->label];
            std::string label = item->label;
            for (int i = 0; i < callbacks.size(); i++) {
                callbacks[i]->receive_events(
                    (char *) label.c_str(), atom_id, payload);
            }
        }
    }
}

void SpiNNakerFrontEndCommonLiveEventsConnection::_process_time_message(
        EIEIOMessage *message){
    std::map<std::pair<int, std::string>, std::vector<int> *> key_times_labels;
    while (message->is_next_element()) {
        EIEIOElement* element = message->get_next_element();
        int time = element->get_payload();
        int key = element->get_key();

        std::map<int, struct label_and_atom_id *>::iterator value =
            this->key_to_atom_id_and_label_map.find(key);
        if (value != this->key_to_atom_id_and_label_map.end()) {
            struct label_and_atom_id *item = value->second;
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
            ids->push_back(item->atom_id);
        }
    }
    for (std::map<std::pair<int, std::string>,
                  std::vector<int> *>::iterator iter =
                      key_times_labels.begin();
            iter != key_times_labels.end(); iter++) {
        int time = iter->first.first;
        std::string label = iter->first.second;
        std::vector<EventReceiveCallbackInterface *> callbacks =
            this->live_event_callbacks[label];
        for (int i = 0; i < callbacks.size(); i++) {
            std::vector<int> *events = iter->second;
            callbacks[i]->receive_events(
                (char *) label.c_str(), time, events->size(),
                &((*events)[0]));
        }
    }
}


void SpiNNakerFrontEndCommonLiveEventsConnection::receive_packet_callback(
        EIEIOMessage *message) {
    if (!message->has_timestamps()) {
        this->_process_payload_message(message);
    }
    else{
       this->_process_time_message(message);
    }
}


void SpiNNakerFrontEndCommonLiveEventsConnection::send_events(
        char *label, std::vector<int> atom_ids, bool send_full_keys){

    // figure max events size for each message
    int max_keys = _MAX_HALF_KEYS_PAYLOADS_PER_PACKET;
    if (send_full_keys) {
        max_keys = _MAX_FULL_KEYS_PAYLOADS_PER_PACKET;
    }

    // iterate till all events have been consumed
    int pos = 0;
    while (pos < atom_ids.size()) {

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

        int events_in_packet = 0;

        // iterate till packet has been filled, or all events are consumed
        while (pos < atom_ids.size() && events_in_packet < max_keys) {
            int key;
            if (send_full_keys) {
                std::map<int, int> * map = this->atom_id_to_key_maps[label];
                key = (*map)[atom_ids[pos]];
            }
            else{
                key = atom_ids[pos];
            }
            message.add_key(key);
            message.increment_count();
            pos += 1;
            events_in_packet += 1;
        }

        //locate socket details for where to send this to
        unsigned char * data = (unsigned char *) malloc(message.get_max_size());
        int size = message.get_data(data);
        this->send_data_to(data, size,
                           this->send_address_details[std::string(label)]);
        free(data);
    }
}

void SpiNNakerFrontEndCommonLiveEventsConnection::send_event(
        char* label, int atom_id, bool send_full_keys) {
    // build a vector for the atom_id, then call send events.
    std::vector<int> atom_ids;
    atom_ids.push_back(atom_id);
    send_events(label, atom_ids, send_full_keys);
}


void SpiNNakerFrontEndCommonLiveEventsConnection::send_events_with_payloads(
        char *label, std::vector<int> atom_ids, std::vector<int> payloads,
        bool send_full_keys){

    // figure max events size for each message
    int max_keys = _MAX_HALF_KEYS_PER_PACKET;
    if (send_full_keys) {
        max_keys = _MAX_FULL_KEYS_PAYLOADS_PER_PACKET;
    }

    // iterate till all events have been consumed
    int pos = 0;
    while (pos < atom_ids.size()) {

        EIEIOHeader * header;
        if (send_full_keys) {
            EIEIOType type = KEY_PAYLOAD_32_BIT;
            EIEIOHeader* eieio_header =
                new EIEIOHeader(1, 0, 0, 0, type, 0, 0, 0);
            header = eieio_header;
        }
        else {
            EIEIOType type = KEY_PAYLOAD_16_BIT;
            EIEIOHeader* eieio_header =
                new EIEIOHeader(1, 0, 0, 0, type, 0, 0, 0);
            header = eieio_header;
        }
        EIEIOMessage message = EIEIOMessage(header);

        int events_in_packet = 0;

        // iterate till packet has been filled, or all events are consumed
        while (pos < atom_ids.size() && events_in_packet < max_keys) {
            int key;
            int payload = payloads[pos];
            if (send_full_keys) {
                std::map<int, int> * map = this->atom_id_to_key_maps[label];
                key = (*map)[atom_ids[pos]];
            }
            else{
                key = atom_ids[pos];
            }
            message.add_key_and_payload(key, payload);
            message.increment_count();
            pos += 1;
            events_in_packet += 1;
        }

        //locate socket details for where to send this to
        unsigned char * data = (unsigned char *) malloc(message.get_max_size());
        int size = message.get_data(data);
        this->send_data_to(data, size,
                           this->send_address_details[std::string(label)]);
        free(data);
    }
}


void SpiNNakerFrontEndCommonLiveEventsConnection::send_event_with_payload(
        char *label, int atom_id, int payload, bool send_full_keys){
    // build a vector for the atom_id, then call send events.
    std::vector<int> atom_ids;
    atom_ids.push_back(atom_id);

    // build a vector for the payloads, then call send events.
    std::vector<int> payloads;
    payloads.push_back(payload);

    // send all events with payloads
    send_events_with_payloads(label, atom_ids, payloads, send_full_keys);
}

void SpiNNakerFrontEndCommonLiveEventsConnection::send_start(char *label) {
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

SpiNNakerFrontEndCommonLiveEventsConnection::
        ~SpiNNakerFrontEndCommonLiveEventsConnection(){
    pthread_mutex_destroy(&(this->start_mutex));
    pthread_cond_destroy(&(this->start_condition));
}
