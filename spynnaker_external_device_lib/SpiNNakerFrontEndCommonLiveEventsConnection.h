#include "SpynnakerDatabaseConnection.h"
#include "DatabaseReader.h"
#include "Threadable.h"
#include "ConnectionListener.h"
#include "EIEIOMessage.h"

#include <string>
#include <vector>
#include <map>

#ifndef _SPINNAKER_FRONT_END_COMMON_LIVE_EVENTS_CONNECTION_H_
#define _SPINNAKER_FRONT_END_COMMON_LIVE_EVENTS_CONNECTION_H_

class SpiNNakerFrontEndCommonLiveEventsConnection;

struct label_and_atom_id {
    char *label;
    int atom_id;

    label_and_atom_id(char *label, int atom_id) {
        this->label = label;
        this->atom_id = atom_id;
    }
};

// The maximum number of 32-bit keys that will fit in a packet
const int _MAX_FULL_KEYS_PER_PACKET = 63;

// The maximum number of 32-bit keys and payloads that will fit in a packet
const int _MAX_FULL_KEYS_PAYLOADS_PER_PACKET = 31;

// The maximum number of 16-bit keys and payloads that will fit in a packet
const int _MAX_HALF_KEYS_PAYLOADS_PER_PACKET = 42;

// The maximum number of 16-bit keys that will fit in a packet
const int _MAX_HALF_KEYS_PER_PACKET = 127;



class EventInitializeCallbackInterface {
public:
    virtual void init_population(
        char *label, int n_neurons, float run_time_ms,
        float machine_time_step_ms) = 0;
    virtual ~EventInitializeCallbackInterface() {};
};


class EventReceiveCallbackInterface {
public:
    virtual void receive_events(
        char *label, int time, int n_events, int* events) = 0;
    virtual ~EventReceiveCallbackInterface() {};
};


class EventReceiveCallbackPayloadInterface {
public:
    virtual void receive_events(
        char *label, int event, int payload) = 0;
    virtual ~EventReceiveCallbackPayloadInterface() {};
};


class EventsStartCallbackInterface {
public:
    virtual void events_start(
        char *label, SpiNNakerFrontEndCommonLiveEventsConnection *connection) = 0;
    virtual ~EventsStartCallbackInterface() {};
};


class SpiNNakerFrontEndCommonLiveEventsConnection :
        public SpynnakerDatabaseConnection, public DatabaseCallbackInterface,
        public StartCallbackInterface, public PacketReceiveCallbackInterface {
public:
    SpiNNakerFrontEndCommonLiveEventsConnection(
        int n_receive_labels=0, char **receive_labels=NULL,
        int n_send_labels=0, char **send_labels=NULL,
        char *local_host=NULL, int local_port=0,
        bool wait_for_start=false);
    void add_initialize_callback(
        char *label, EventInitializeCallbackInterface *init_callback);
    void add_receive_callback(
        char *label, EventReceiveCallbackInterface *receive_callback);
    void add_receive_callback_payload(
        char *label, EventReceiveCallbackPayloadInterface *receive_callback);
    void add_start_callback(
        char *label, EventsStartCallbackInterface *start_callback);
    void add_wait_for_start(char *label);
    void send_event(
        char *label, int atom_id, bool send_full_keys=false);
    void send_events(
        char *label, std::vector<int> n_atom_ids, bool send_full_keys=false);
    void send_event_with_payload(
        char *label, int atom_id, int payload, bool send_full_keys=false);
    void send_events_with_payloads(
        char *label, std::vector<int> n_atom_ids, std::vector<int> n_payloads,
        bool send_full_keys=false);
    void send_start(char *label=NULL);
    ~SpiNNakerFrontEndCommonLiveEventsConnection();
    virtual void read_database_callback(DatabaseReader *reader);
    virtual void start_callback();
    virtual void receive_packet_callback(EIEIOMessage *message);

private:
    static void *_call_start_callback(void *start_callback_info);
    void _process_payload_message(EIEIOMessage *message);
    void _process_time_message(EIEIOMessage *message);

    std::vector<char *> receive_labels;
    std::vector<char *> send_labels;
    std::map<std::string, struct sockaddr *> send_address_details;
    std::map<std::string, std::map<int, int> *> atom_id_to_key_maps;
    std::map<int, struct label_and_atom_id *> key_to_atom_id_and_label_map;
    std::map<std::string, std::vector<
             EventInitializeCallbackInterface *> > init_callbacks;
    std::map<std::string, std::vector<
             EventReceiveCallbackInterface *> > live_event_callbacks;
    std::map<std::string, std::vector<
            EventReceiveCallbackPayloadInterface *> > 
            live_event_payload_callbacks;
    std::map<std::string, std::vector<
             EventsStartCallbackInterface *> > start_callbacks;

    pthread_cond_t start_condition;
    pthread_mutex_t start_mutex;
    std::map<std::string, bool> waiting_for_start;
    int n_waiting_for_start;
};

#endif /* _SPINNAKER_FRONT_END_COMMON_LIVE_EVENTS_CONNECTION_H_ */
