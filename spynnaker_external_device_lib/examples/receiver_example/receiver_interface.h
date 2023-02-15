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
#include "../../SpynnakerLiveSpikesConnection.h"
#include <mutex>

class ReceiverInterface : public SpikeReceiveCallbackInterface {
public:
    ReceiverInterface(std::mutex *mtx) : mtx(mtx){};
    void receive_spikes(char *label, int time, int n_spikes, int* spikes);
    int get_n_spikes() {
        return this->n_spikes;
    }
private:
    int n_spikes = 0;
    std::mutex *mtx;

};
