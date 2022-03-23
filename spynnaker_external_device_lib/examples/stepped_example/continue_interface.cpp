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
