#include <pthread.h>
#include "SpynnakerLiveSpikesConnection.h"

class WaitForStop : public SpikesPauseStopCallbackInterface {
public:

    WaitForStop() {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
        stopped = false;
    }

    void spikes_stop(
            char *label, SpynnakerLiveSpikesConnection *connection) {
        (void) pthread_mutex_lock(&mutex);
        stopped = true;
        (void) pthread_cond_signal(&cond);
        (void) pthread_mutex_unlock(&mutex);
    }

    void wait_for_stop() {
        (void) pthread_mutex_lock(&mutex);
        while (!stopped) {
            (void) pthread_cond_wait(&cond, &mutex);
        }
        (void) pthread_mutex_unlock(&mutex);
    }

private:
    bool stopped;
    pthread_cond_t cond;
    pthread_mutex_t mutex;

};
