#include "receiver_interface.h"
#include <stdio.h>
#include <pthread.h>

ReceiverInterface::ReceiverInterface(pthread_mutex_t *cond){
    this->cond = cond;
}

void ReceiverInterface::receive_spikes(
        char *label, int time, int n_spikes, int* spikes){
    (void) pthread_mutex_lock(this->cond);
    for (int neuron_id_position = 0;  neuron_id_position < n_spikes;
            neuron_id_position++){
         fprintf(stderr, "Received spike at time %d, from %s - %d \n",
                time, label, spikes[neuron_id_position]);
    }
    (void) pthread_mutex_unlock(this->cond);
}
