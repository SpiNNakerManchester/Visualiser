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
#include "../../SpynnakerLiveSpikesConnection.h"
#include "../../WaitForStop.h"
#include "sender_interface_forward.h"
#include "sender_interface_backward.h"
#include "receiver_interface.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
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
        fprintf(stderr, "Listening on %u\n", connection.get_local_port());
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
        fprintf(stderr, "Received %u spikes", receiver_callback->get_n_spikes());
        (void) pthread_mutex_unlock(&count_mutex);
    }
    catch (char const* msg){
        printf("%s \n", msg);
    }
}
