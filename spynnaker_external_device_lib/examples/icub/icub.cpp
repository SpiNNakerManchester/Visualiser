#include "../../SpynnakerLiveSpikesConnection.h"
#include <chrono>
#include <thread>

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

ICubInterface::ICubInterface(
        char *send_label, char *receive_label) {
    if (pthread_mutex_init(&this->recv_mutex, NULL) == -1) {
        fprintf(stderr, "Error initializing recv mutex!\n");
        exit(-1);
    }
    pthread_cond_init(&this->recv_cond, 0);
    if (pthread_mutex_init(&this->send_mutex, NULL) == -1) {
        fprintf(stderr, "Error initializing send mutex!\n");
        exit(-1);
    }
    pthread_cond_init(&this->send_cond, 0);
    this->send_queue_enabled = false;
    this->running = true;
    this->send_label = NULL;

    this->send_connection = new SpynnakerLiveSpikesConnection(
         1, &receive_label, 1, &send_label, (char*) NULL, 19999);
    send_connection->add_initialize_callback(receive_label, this);
    send_connection->add_receive_callback(receive_label, this);
    send_connection->add_start_callback(send_label, this);
    send_connection->add_pause_stop_callback(send_label, this);

    this->start();
}

void ICubInterface::init_population(
        char *label, int n_neurons, float run_time_ms,
        float machine_time_step_ms) {
    // TODO: Do anything that needs to be done before the simulation starts
    fprintf(stderr, "Population %s has %d neurons\n", label, n_neurons);
}


void ICubInterface::receive_spikes(
        char *label, int time, int n_spikes, int *spikes) {
    pthread_mutex_lock(&this->recv_mutex);
    for (int i = 0; i < n_spikes; i++) {
        recv_queue.push_back(std::pair<int, int>(time, spikes[i]));
    }
    pthread_cond_signal(&this->recv_cond);
    pthread_mutex_unlock(&this->recv_mutex);
}

void ICubInterface::spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection) {
    pthread_mutex_lock(&this->send_mutex);
    this->send_label = (char *) malloc(sizeof(char) * strlen(label));
    strcpy(this->send_label, label);
    pthread_cond_signal(&this->send_cond);
    pthread_mutex_unlock(&this->send_mutex);
}

void ICubInterface::spikes_stop(
        char *label, SpynnakerLiveSpikesConnection *connection) {
    this->send_label = NULL;
    fprintf(stderr, "Exiting\n");
    exit(0);
}

int ICubInterface::getRecvQueueSize() {
    int size = 0;
    pthread_mutex_lock(&this->recv_mutex);
    size = this->recv_queue.size();
    pthread_cond_signal(&this->recv_cond);
    pthread_mutex_unlock(&this->recv_mutex);
    return size;
}

std::pair<int, int> ICubInterface::getNextSpike(bool wait) {
    std::pair<int, int> spike;
    pthread_mutex_lock(&this->recv_mutex);
    while (wait && this->recv_queue.size() == 0) {
        pthread_cond_wait(&this->recv_cond, &this->recv_mutex);
    }
    if (this->recv_queue.size() < 0) {
        return std::pair<int, int>(-1, -1);
    }
    spike = this->recv_queue.front();
    this->recv_queue.pop_front();
    pthread_mutex_unlock(&this->recv_mutex);
    return spike;
}

void ICubInterface::addSpikeToSendQueue(int neuron_id) {
    pthread_mutex_lock(&this->send_mutex);
    this->send_queue.push_back(neuron_id);
    pthread_cond_signal(&this->send_cond);
    pthread_mutex_unlock(&this->send_mutex);
}

void ICubInterface::enableSendQueue() {
    pthread_mutex_lock(&this->send_mutex);
    this->send_queue_enabled = true;
    pthread_cond_signal(&this->send_cond);
    pthread_mutex_unlock(&this->send_mutex);
}

void ICubInterface::run() {

    pthread_mutex_lock(&this->send_mutex);
    while (this->running) {
        while (this->running
                && ((!this->send_queue_enabled)
                    || (this->send_queue.size() <= 0)
                    || (this->send_label == NULL))) {
            pthread_cond_wait(&this->send_cond, &this->send_mutex);
        }

        int id = this->send_queue.front();
        this->send_queue.pop_front();

        pthread_mutex_unlock(&this->send_mutex);
        this->send_connection->send_spike(this->send_label, id);
        pthread_mutex_lock(&this->send_mutex);
    }
    pthread_mutex_unlock(&this->send_mutex);
}

void ICubInterface::end() {
    pthread_mutex_lock(&this->send_mutex);
    this->running = false;
    pthread_cond_signal(&this->send_cond);
    pthread_mutex_unlock(&this->send_mutex);
}

int main(int argc, char **argv) {
    ICubInterface *icub = new ICubInterface(
        (char *) "from_icub", (char *) "to_icub");
    icub->addSpikeToSendQueue(0);
    icub->enableSendQueue();

    fprintf(stderr, "Listening for database\n");
    while (true) {
        std::pair<int, int> spike = icub->getNextSpike(true);
        fprintf(stderr, "Time %d: Received %d\n", spike.first, spike.second);
    }
    fprintf(stderr, "Exiting end of main\n");
}
