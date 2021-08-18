#include "continue_interface.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#ifdef WIN32
#define sleep(n) Sleep(n)
#endif


void ContinueInterface::spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection){
    for (int neuron_id = 0; neuron_id < 100; neuron_id += 20){
        sleep(1);
        fprintf(stderr, "Sending continue\n");
        connection->continue_run();
    }
}
