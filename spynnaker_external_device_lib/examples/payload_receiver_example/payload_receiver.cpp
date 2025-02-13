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
#include <stdio.h>
#include <unistd.h>
#ifdef WIN32
#define sleep(n) Sleep(n)
#endif

int main(int argc, char **argv){
    try{
        // set up basic stuff
        char const* receive_label1 = "left_wheel_motor_voltage";
        char const* receive_label2 = "right_wheel_motor_voltage";
        char* receive_labels[2] = {(char *) receive_label1, (char*) receive_label2};
        char* send_labels[0];
        char const* local_host = NULL;
        SpynnakerLiveSpikesConnection connection = SpynnakerLiveSpikesConnection(
            2, receive_labels, 0, send_labels);
        fprintf(stderr, "Listening on %u\n", connection.get_local_port());

        // Register SpikeReceiveCallbackInterface
        ReceiverInterface* receiver_callback = new ReceiverInterface();
        connection.add_receive_callback((char*) receive_label1, receiver_callback);
        connection.add_receive_callback((char*) receive_label2, receiver_callback);

        // Stop interface
        WaitForStop *wait_for_stop = new WaitForStop();
        connection.add_pause_stop_callback((char*) receive_label1, wait_for_stop);

        wait_for_stop->wait_for_stop();
    }
    catch (char const* msg){
        printf("%s \n", msg);
    }
}
