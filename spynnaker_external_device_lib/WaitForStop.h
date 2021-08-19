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
#include <pthread.h>
#include "SpynnakerLiveSpikesConnection.h"

class WaitForStop : public SpikesPauseStopCallbackInterface {
public:

    WaitForStop() {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
        stopped = false;
    }

    void spikes_stop(
            char *label, SpynnakerLiveSpikesConnection *connection) {
        (void) pthread_mutex_lock(&mutex);
        stopped = true;
        (void) pthread_cond_signal(&cond);
        (void) pthread_mutex_unlock(&mutex);
    }

    void wait_for_stop() {
        (void) pthread_mutex_lock(&mutex);
        while (!stopped) {
            (void) pthread_cond_wait(&cond, &mutex);
        }
        (void) pthread_mutex_unlock(&mutex);
    }

private:
    bool stopped;
    pthread_cond_t cond;
    pthread_mutex_t mutex;

};
