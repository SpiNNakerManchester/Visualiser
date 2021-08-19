#include "../../SpynnakerLiveSpikesConnection.h"

class ReceiverInterface : public SpikeReceiveCallbackInterface{
public:
    void receive_spikes(char *label, int time, int n_spikes, int* spikes);
    ReceiverInterface(pthread_mutex_t *cond);

private:
    pthread_mutex_t *cond;
};
