/*
 * Copyright (c) 2015 The University of Manchester
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
        printf("Sending continue\n");
        try {
            connection->continue_run();
        } catch (char const *msg) {
            printf("%s \n", msg);
        }
    }
}
