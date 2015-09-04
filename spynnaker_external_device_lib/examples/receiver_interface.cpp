#include "receiver_interface.h"
#include <stdio.h>

ReceiverInterface::ReceiverInterface(){

}

void ReceiverInterface::receive_spikes(
        char *label, int time, int n_spikes, int* spikes){
    for (int neuron_id_position = 0;  neuron_id_position < n_spikes;
            neuron_id_position++){
         printf("Received spike at time %d, from %s - %d",
                time, label, spikes[neuron_id_position]);
    }
}