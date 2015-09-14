#include "sender_interface_forward.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#ifdef WIN32
#define sleep(n) Sleep(n)
#endif


SenderInterfaceForward::SenderInterfaceForward(pthread_mutex_t *cond){
    this->cond = cond;
}

void SenderInterfaceForward::spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection){
    srand(time(NULL));
    for (int neuron_id = 0; neuron_id < 100; neuron_id += 20){
        float time =  (((float)(rand() % 100)) / 100) + 0.5;
        sleep(time);
        fprintf(stderr, "waiting for %f seconds \n", time);
        (void) pthread_mutex_lock(this->cond);
        printf("Sending forward spike %d", neuron_id);
        (void) pthread_mutex_unlock(this->cond);
        connection->send_spike(label, neuron_id);
    }
}
