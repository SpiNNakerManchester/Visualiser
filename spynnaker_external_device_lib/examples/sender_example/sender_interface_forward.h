#include "../../SpynnakerLiveSpikesConnection.h"
#include <pthread.h>

class SenderInterfaceForward : public SpikesStartCallbackInterface{
public:
    void spikes_start(char *label, SpynnakerLiveSpikesConnection *connection);
    SenderInterfaceForward(pthread_mutex_t *cond);

private:
    pthread_mutex_t *cond;
};
