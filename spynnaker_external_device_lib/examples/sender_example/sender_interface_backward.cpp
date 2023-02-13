/*
 * Copyright (c) 2015-2023 The University of Manchester
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
#include "sender_interface_backward.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#ifdef WIN32
#define sleep(n) Sleep(n)
#endif


SenderInterfaceBackward::SenderInterfaceBackward(pthread_mutex_t *cond){
    this->cond = cond;
}

void SenderInterfaceBackward::spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection){
    srand(time(NULL));
    for (int neuron_id = 0; neuron_id < 100; neuron_id += 20){
        int real_id = 100 - neuron_id - 1;
        float time =  (((float)(rand() % 100)) / 100) + 0.5;
        printf("waiting for %f seconds \n", time);
        sleep(time);
        (void) pthread_mutex_lock(this->cond);
        printf("Sending backward spike %d \n", real_id);
        (void) pthread_mutex_unlock(this->cond);
        connection->send_spike(label, real_id);
    }
}
