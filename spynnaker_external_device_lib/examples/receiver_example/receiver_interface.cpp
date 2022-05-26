/*
 * Copyright (c) 2015-2021 The University of Manchester
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
