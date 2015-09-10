#include "sender_interface_backward.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>


SenderInterfaceBackward::SenderInterfaceBackward(pthread_mutex_t *cond){
    this->cond = cond;
}

void SenderInterfaceBackward::spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection){
    srand(time(NULL));
    for (int neuron_id = 0; neuron_id < 100; neuron_id += 20){
        int real_id = 100 - neuron_id - 1;
        sleep(((float)((rand() % 100) / 100)) + 0.5);
        (void) pthread_mutex_lock(this->cond);
        fprintf(stderr, "Sending backward spike %d \n", real_id);
        (void) pthread_mutex_unlock(this->cond);
        connection->send_spike(label, real_id);
    }
}