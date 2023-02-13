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
#include "sender_interface_forward.h"
#include "sender_interface_backward.h"
#include "receiver_interface.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#ifdef WIN32
#define sleep(n) Sleep(n)
#endif

int main(int argc, char **argv){
    try{
        // set up basic stuff
        char const* send_label1 = "spike_injector_forward";
        char const* send_label2 = "spike_injector_backward";
        char* send_labels[2] = {(char *) send_label1, (char*) send_label2};
        char const* label1 = "pop_forward";
        char const* label2 = "pop_backward";
        char* receive_labels[2] = {(char *) label1, (char*) label2};
        char const* local_host = NULL;
        SpynnakerLiveSpikesConnection connection =
            SpynnakerLiveSpikesConnection(
                2, receive_labels, 2, send_labels, (char*) local_host);
        std::cerr << "Listening on " << connection.get_local_port() << "\n";
        // build the SpikeReceiveCallbackInterface
        pthread_mutex_t count_mutex;
        pthread_mutex_init(&count_mutex, NULL);

        SenderInterfaceForward* sender_callback_forward =
            new SenderInterfaceForward(&count_mutex);
        SenderInterfaceBackward* sender_callback_backward =
            new SenderInterfaceBackward(&count_mutex);

        ReceiverInterface* receiver_callback =
            new ReceiverInterface(&count_mutex);
        // register the callback with the SpynnakerLiveSpikesConnection
        connection.add_receive_callback(
            (char*) label1, receiver_callback);
        connection.add_receive_callback(
            (char*) label2, receiver_callback);

        // register the callback with the SpynnakerLiveSpikesConnection
        connection.add_start_callback((char *) send_label1,
                                      sender_callback_forward);
        connection.add_start_callback((char *) send_label2,
                                      sender_callback_backward);

        // Deal with end of simulation
        WaitForStop *wait_for_stop = new WaitForStop();
        connection.add_pause_stop_callback((char *) label1, wait_for_stop);
        wait_for_stop->wait_for_stop();
        (void) pthread_mutex_lock(&count_mutex);
        std::cerr << "Received " << receiver_callback->get_n_spikes() << " spikes\n";
        (void) pthread_mutex_unlock(&count_mutex);
    }
    catch (std::exception &e){
        std::cout << "Exception caught: " << e.what() << "\n";
    }
    catch (...) {
        std::cout << "Unknown exception caught\n";
    }
}
