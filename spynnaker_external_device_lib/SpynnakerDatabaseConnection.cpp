#include "SpynnakerDatabaseConnection.h"

#define MAX_PACKET_SIZE 300

SpynnakerDatabaseConnection::SpynnakerDatabaseConnection(
        DatabaseCallbackInterface *database_callback,
        StartCallbackInterface *start_callback,
        char *local_host, int local_port)
        :UDPConnection(local_port, local_host) {
    this->database_callback = database_callback;
    this->start_callback = start_callback;
    this->start();
}

void SpynnakerDatabaseConnection::run() {

    // Wait for notification that the database has been written
    unsigned char data[MAX_PACKET_SIZE];
    struct sockaddr_in address;
    int n_bytes = this->receive_data_with_address(
        data, MAX_PACKET_SIZE - 1, (struct sockaddr *) &address);
    data[n_bytes] = '\0';
    char *database_path = (char *) &data[2];

    // Call the callback function with the reader
    DatabaseReader reader(database_path);
    this->database_callback->read_database_callback(&reader);
    reader.close_database_connection();

    // Send the notification back that the database has been read
    unsigned char eieio_response[2];
    eieio_response[1] = (1 << 6);
    eieio_response[0] = 1;
    this->send_data_to(eieio_response, 2, (struct sockaddr *) &address);

    if (this->start_callback != NULL) {

        // Wait for the start notification
        unsigned char unused_data[MAX_PACKET_SIZE];
        this->receive_data(unused_data, MAX_PACKET_SIZE);

        // Call the start callback
        this->start_callback->start_callback();
    }
}

SpynnakerDatabaseConnection::~SpynnakerDatabaseConnection() {
}
