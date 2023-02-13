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
#include "../../SpynnakerLiveSpikesConnection.h"
#include "../../WaitForStop.h"
#include "receiver_interface.h"
#include <stdio.h>
#include <unistd.h>
#include <mutex>
#include <iostream>
#ifdef WIN32
#define sleep(n) Sleep(n)
#endif

int main(int argc, char **argv){
    try{
        // set up basic stuff
        char const* receive_label1 = "pop_forward";
        char const* receive_label2 = "pop_backward";
        char* receive_labels[2] = {(char *) receive_label1, (char*) receive_label2};
        char* send_labels[0];
        char const* local_host = NULL;
        std::mutex mtx;
        SpynnakerLiveSpikesConnection connection = SpynnakerLiveSpikesConnection(
            2, receive_labels, 0, send_labels);
        std::cerr << "Listening on " << connection.get_local_port() << "\n";
        // Register SpikeReceiveCallbackInterface
        ReceiverInterface* receiver_callback = new ReceiverInterface(&mtx);
        connection.add_receive_callback((char*) receive_label1, receiver_callback);
        connection.add_receive_callback((char*) receive_label2, receiver_callback);

        // Stop interface
        WaitForStop *wait_for_stop = new WaitForStop();
        connection.add_pause_stop_callback((char*) receive_label1, wait_for_stop);

        wait_for_stop->wait_for_stop();
        std::lock_guard<std::mutex> lock(mtx);
        std::cerr << "Received " << receiver_callback->get_n_spikes() << " spikes\n";
    }
    catch (std::exception &e){
        std::cout << "Exception caught: " << e.what() << "\n";
    }
    catch (...) {
        std::cout << "Unknown exception caught\n";
    }
}
