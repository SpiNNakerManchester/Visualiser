#include "../../SpynnakerLiveSpikesConnection.h"

class ICubInterface :
        public SpikeReceiveCallbackInterface,
        public SpikesStartCallbackInterface,
        public SpikesPauseStopCallbackInterface,
        public SpikeInitializeCallbackInterface,
        private Threadable {
public:
    ICubInterface(char *send_labels, char *receive_labels);
    void init_population(
        char *label, int n_neurons, float run_time_ms,
        float machine_time_step_ms);
    void receive_spikes(char *label, int time, int n_spikes, int* spikes);
    void spikes_start(char *label, SpynnakerLiveSpikesConnection *connection);
    void spikes_stop(char *label, SpynnakerLiveSpikesConnection *connection);

    int getRecvQueueSize();
    std::pair<int, int> getNextSpike(bool wait=false);
    void addSpikeToSendQueue(int neuron_id);
    void enableSendQueue();
    void waitForStart();
    void waitForEnd();
    void end();

protected:
    void run();

private:
    pthread_mutex_t recv_mutex;
    pthread_cond_t recv_cond;
    std::deque< std::pair<int, int> > recv_queue;
    pthread_mutex_t send_mutex;
    pthread_cond_t send_cond;
    std::deque< int > send_queue;
    bool send_queue_enabled;
    SpynnakerLiveSpikesConnection *send_connection;
    char *send_label;
    bool running;
};
