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
#include "SpynnakerDatabaseConnection.h"

#define MAX_PACKET_SIZE 300

SpynnakerDatabaseConnection::SpynnakerDatabaseConnection(
        StartCallbackInterface *start_callback,
        PauseStopCallbackInterface *pause_stop_callback,
        char *local_host, int local_port)
        :UDPConnection(local_port, local_host) {
    this->database_path_received = false;
    this->start_callback = start_callback;
    this->pause_stop_callback = pause_stop_callback;
    this->running = false;
    this->start();
}

void SpynnakerDatabaseConnection::add_database_callback(
        DatabaseCallbackInterface *callback){
    this->database_callbacks.push_back(callback);
}

void SpynnakerDatabaseConnection::run() {

    this->running = true;
    while (this->running) {

        // Wait for notification that the database has been written
        unsigned char data[MAX_PACKET_SIZE];
        struct sockaddr_in address;
        int n_bytes = this->receive_data_with_address(
            data, MAX_PACKET_SIZE - 1, (struct sockaddr *) &address);
        if (!database_path_received) {
            data[n_bytes] = '\0';
            char *database_path = (char *) &data[2];

            this->set_database(database_path);

            // Send the notification back that the database has been read
            unsigned char eieio_response[2];
            eieio_response[1] = (1 << 6);
            eieio_response[0] = 1;
            this->send_data_to(
                eieio_response, 2, (struct sockaddr *) &address);
            database_path_received = true;
        }

        // Wait for the start notification
        unsigned char unused_data[MAX_PACKET_SIZE];
        this->receive_data(unused_data, MAX_PACKET_SIZE);
        if (this->start_callback != NULL) {
            // Call the start callback
            this->start_callback->start_callback();
        }

        // Wait for the stop / pause notification
        unsigned char unused_data2[MAX_PACKET_SIZE];
        this->receive_data(unused_data2, MAX_PACKET_SIZE);
        if (this->pause_stop_callback != NULL){
            // Call the pause stop callback
            this->pause_stop_callback->pause_stop_callback();
        }
    }
}

void SpynnakerDatabaseConnection::set_database(char *database_path) {

    database_path_received = true;

    // Call the callback function with the reader
    DatabaseReader reader(database_path);
    for (int i = 0; i < this->database_callbacks.size(); i++) {
        this->database_callbacks[i]->read_database_callback(&reader);
    }
    reader.close_database_connection();
}

SpynnakerDatabaseConnection::~SpynnakerDatabaseConnection() {
    this->running = false;
}
