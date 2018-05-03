#include "../../SpynnakerLiveSpikesConnection.h"
#include <chrono>
#include <thread>

class ICubInterface :
        public SpikeReceiveCallbackInterface,
        public SpikesStartCallbackInterface,
        public SpikesPauseStopCallbackInterface,
        public SpikeInitializeCallbackInterface {
public:
    void init_population(
        char *label, int n_neurons, float run_time_ms,
        float machine_time_step_ms);
    void receive_spikes(char *label, int time, int n_spikes, int* spikes);
    void spikes_start(char *label, SpynnakerLiveSpikesConnection *connection);
    void spikes_stop(char *label, SpynnakerLiveSpikesConnection *connection);
};

void ICubInterface::init_population(
        char *label, int n_neurons, float run_time_ms,
        float machine_time_step_ms) {
    // TODO: Do anything that needs to be done before the simulation starts
    fprintf(stderr, "Population %s has %d neurons\n", label, n_neurons);
}


void ICubInterface::receive_spikes(
        char *label, int time, int n_spikes, int *spikes) {
    // TODO: Do something with spikes received
    for (int i = 0; i < n_spikes; i++) {
        fprintf(stderr, "Time = %dms: Received spike from %s: %d\n",
            time, label, spikes[i]);
    }
}

void ICubInterface::spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection) {
    // TODO: Do something when the simulation starts
    // NOTE: Some experimentation has shown that it is best to wait a
    //       short time before sending spikes (otherwise they just go missing,
    //       so not a major issue)
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    fprintf(stderr, "Sending spike %s: 0\n", label);
    connection->send_spike(label, 0);
}

void ICubInterface::spikes_stop(
        char *label, SpynnakerLiveSpikesConnection *connection) {
    // TODO: Do something when the simulation stops
    exit(0);
}

int main(int argc, char **argv) {
    char *send_labels[1] = {(char *) "from_icub"};
    char *receive_labels[1] = {(char *) "to_icub"};
    SpynnakerLiveSpikesConnection connection = SpynnakerLiveSpikesConnection(
        1, receive_labels, 1, send_labels, (char*) NULL, 19999);

    ICubInterface *icub = new ICubInterface();
    connection.add_initialize_callback(receive_labels[0], icub);
    connection.add_receive_callback(receive_labels[0], icub);
    connection.add_start_callback(send_labels[0], icub);
    connection.add_pause_stop_callback(send_labels[0], icub);

    fprintf(stderr, "Listening for database\n");
    while (true) {
       std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}
