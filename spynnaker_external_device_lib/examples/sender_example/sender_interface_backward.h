#include "../../SpynnakerLiveSpikesConnection.h"
#include <pthread.h>

class SenderInterfaceBackward : public SpikesStartCallbackInterface{
public:
    void spikes_start(char *label, SpynnakerLiveSpikesConnection *connection);
    SenderInterfaceBackward(pthread_mutex_t *cond);

private:
    pthread_mutex_t *cond;
};