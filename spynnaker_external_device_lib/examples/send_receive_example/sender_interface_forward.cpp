/*
 * Copyright (c) 2015 The University of Manchester
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
        printf("waiting for %f seconds \n", time);
        (void) pthread_mutex_lock(this->cond);
        printf("Sending forward spike %d\n", neuron_id);
        (void) pthread_mutex_unlock(this->cond);
        connection->send_spike(label, neuron_id);
    }
}
