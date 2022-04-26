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
#include "receiver_interface.h"
#include "continue_interface.h"
#include <stdio.h>
#include <unistd.h>
#include <mutex>
#ifdef WIN32
#define sleep(n) Sleep(n)
#endif

int main(int argc, char **argv){
    try{
        // set up basic stuff
        char const* receive_label = "ssa";
        char* receive_labels[1] = {(char *) receive_label};
        char* send_labels[0];
        char const* local_host = NULL;
        std::mutex mtx;
        SpynnakerLiveSpikesConnection connection = SpynnakerLiveSpikesConnection(
            1, receive_labels, 0, send_labels, (char*) local_host);
        fprintf(stderr, "Listening on %u\n", connection.get_local_port());
        // build the SpikeReceiveCallbackInterface
        ReceiverInterface* receiver_callback = new ReceiverInterface(&mtx);
        // register the callback with the SpynnakerLiveSpikesConnection
        connection.add_receive_callback((char*) receive_label, receiver_callback);

        ContinueInterface *continue_callback = new ContinueInterface();
        connection.add_start_callback((char *) receive_label, continue_callback);

        WaitForStop *wait_for_stop = new WaitForStop();
        connection.add_pause_stop_callback((char *) receive_label, wait_for_stop);

        wait_for_stop->wait_for_stop();
        std::lock_guard<std::mutex> lock(mtx);
        fprintf(stderr, "Received %u spikes", receiver_callback->get_n_spikes());
    }
    catch (const char* msg){
        printf("Caught: %s \n", msg);
    }
}
