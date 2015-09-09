#include "sender_interface.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>


SenderInterface::SenderInterface(pthread_cond_t *cond){

}

void SenderInterface::spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection){
    srand(time(NULL));
    for (int neuron_id = 0; neuron_id < 100; neuron_id += 20){
        sleep(rand() + 0.5);
        print_condition.acquire()
        print "Sending forward spike", neuron_id
        print_condition.release()
        connection->send_spike(label, neuron_id, send_full_keys=True)
    }
}