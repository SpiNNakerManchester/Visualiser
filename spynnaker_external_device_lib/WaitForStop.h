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
