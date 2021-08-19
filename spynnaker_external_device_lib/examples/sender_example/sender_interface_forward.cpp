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
        fprintf(stderr, "waiting for %f seconds \n", time);
        sleep(time);
        (void) pthread_mutex_lock(this->cond);
        fprintf(stderr, "Sending forward spike %d \n", neuron_id);
        (void) pthread_mutex_unlock(this->cond);
        connection->send_spike(label, neuron_id);
    }
}
