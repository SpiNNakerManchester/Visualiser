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
#include "receiver_interface.h"
#include <stdio.h>
#include <mutex>

void ReceiverInterface::receive_spikes(
        char *label, int time, int n_spikes, int* spikes){
    std::lock_guard<std::mutex> lock(*mtx);
    for (int neuron_id_position = 0;  neuron_id_position < n_spikes;
            neuron_id_position++){
         printf("Received spike at time %d, from %s - %d \n",
                time, label, spikes[neuron_id_position]);
    }
    this->n_spikes += n_spikes;
}
