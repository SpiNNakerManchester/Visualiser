#include "SpynnakerDatabaseConnection.h"

#define MAX_PACKET_SIZE 300

SpynnakerDatabaseConnection::SpynnakerDatabaseConnection(
        StartCallbackInterface *start_callback,
        PauseStopCallbackInterface *pause_stop_callback,
        char *local_host, int local_port)
        :UDPConnection(local_port, local_host) {
    database_path_received = false;
    this->start_callback = start_callback;
    this->pause_stop_callback = pause_stop_callback;
    running = false;
    start();
}

void SpynnakerDatabaseConnection::add_database_callback(
        DatabaseCallbackInterface *callback){
    database_callbacks.push_back(callback);
}

void SpynnakerDatabaseConnection::run() {

    running = true;
    while (running) {
    
        // Wait for notification that the database has been written
        unsigned char data[MAX_PACKET_SIZE];
        struct sockaddr_in address;
        int n_bytes = receive_data_with_address(
            data, MAX_PACKET_SIZE - 1, &address);
        if (!database_path_received) {
            data[n_bytes] = '\0';
            char *database_path = (char *) &data[2];

            set_database(database_path);

            // Send the notification back that the database has been read
            unsigned char eieio_response[2];
            eieio_response[1] = (1 << 6);
            eieio_response[0] = 1;
            send_data_to(eieio_response, 2, &address);
            database_path_received = true;
        }

        // Wait for the start notification
        unsigned char unused_data[MAX_PACKET_SIZE];
        receive_data(unused_data, MAX_PACKET_SIZE);
        if (start_callback != nullptr) {
            
            // Call the start callback
            start_callback->start_callback();
        }

        // Wait for the stop / pause notification
        unsigned char unused_data2[MAX_PACKET_SIZE];
        receive_data(unused_data2, MAX_PACKET_SIZE);
        if (pause_stop_callback != nullptr){
            
            // Call the pause stop callback
            pause_stop_callback->pause_stop_callback();
        }
    }
}

void SpynnakerDatabaseConnection::set_database(char *database_path) {
    database_path_received = true;

    // Call the callback function with the reader
    DatabaseReader reader(database_path);
    for (int i = 0; i < database_callbacks.size(); i++) {
        database_callbacks[i]->read_database_callback(&reader);
    }
    reader.close_database_connection();
}

SpynnakerDatabaseConnection::~SpynnakerDatabaseConnection() {
    running = false;
}
