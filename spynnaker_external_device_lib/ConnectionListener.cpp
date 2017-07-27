#include "ConnectionListener.h"
#include <iostream>
#include <stdio.h>

ConnectionListener::ConnectionListener(UDPConnection* connection) {
    _connection = connection;
    if (pthread_mutex_init(&free_data_mutex, NULL) == -1) {
        fprintf(stderr, "Error initializing free data mutex\n");
        exit(-1);
    }
    if (pthread_mutex_init(&data_to_process_mutex, NULL) == -1) {
        fprintf(stderr, "Error initializing data to process mutex\n");
        exit(-1);
    }
    if (pthread_cond_init(&free_data_condition, NULL) == -1) {
        fprintf(stderr, "Error initializing free data condition\n");
        exit(-1);
    }
    if (pthread_cond_init(&data_to_process_condition, NULL) == -1) {
        fprintf(stderr, "Error initializing data to process condition\n");
        exit(-1);
    }
    for (int i = 0; i < 256; i++) {
        free_data.push(
            (unsigned char *) malloc(EIEIOMessage::get_max_size()));
    }
    _done = false;
    reader = new Reader(this);
    reader->start();
}

ConnectionListener::~ConnectionListener() {
    // TODO Full teardown required here
}

unsigned char *ConnectionListener::getFreeData() {
    unsigned char *data = nullptr;
    pthread_mutex_lock(&free_data_mutex);
    try{
        while (free_data.empty()) {
            pthread_cond_wait(&free_data_condition, &free_data_mutex);
        }
        data = free_data.front();
        free_data.pop();
    } catch (std::exception& e) {
        if (!_done) {
            printf("thrown a error \n");
            std::cerr << "exception caught: " << e.what() << '\n';
        }
    }
    pthread_mutex_unlock(&free_data_mutex);
    return data;
}

void ConnectionListener::postReceivedData(unsigned char *data) {
    pthread_mutex_lock(&data_to_process_mutex);
    try {
	data_to_process.push(data);
	pthread_cond_signal(&data_to_process_condition);
    } catch (std::exception& e) {
        if (!_done) {
            printf("thrown a error \n");
            std::cerr << "exception caught: " << e.what() << '\n';
        }
    }
    pthread_mutex_unlock(&data_to_process_mutex);
}

void ConnectionListener::run() {
    while (!_done) {
	unsigned char *data = getFreeData();
	if (data == nullptr)
	    continue;
        try{
            int length = _connection->receive_data(
                data, EIEIOMessage::get_max_size());
            // TODO Check if the length of the received data is sensible
            postReceivedData(data);
        } catch (std::exception& e) {
            if (!_done) {
                printf("thrown a error \n");
                std::cerr << "exception caught: " << e.what() << '\n';
            }
        }
    }
}

ConnectionListener::Reader::Reader(ConnectionListener *listener) {
    this->listener = listener;
}

unsigned char *ConnectionListener::Reader::getDataToProcess() {
    pthread_mutex_lock(&listener->data_to_process_mutex);
    while (listener->data_to_process.empty()) {
        pthread_cond_wait(&listener->data_to_process_condition,
                          &listener->data_to_process_mutex);
    }

    unsigned char *data = listener->data_to_process.front();
    listener->data_to_process.pop();

    pthread_mutex_unlock(&listener->data_to_process_mutex);
    return data;
}

void ConnectionListener::Reader::releaseProcessedData(unsigned char *data) {
    pthread_mutex_lock(&listener->free_data_mutex);
    listener->free_data.push(data);
    pthread_cond_signal(&listener->free_data_condition);
    pthread_mutex_unlock(&listener->free_data_mutex);
}

void ConnectionListener::Reader::run() {
    while (!listener->_done) {
        unsigned char *data = getDataToProcess();

        EIEIOMessage* message = new EIEIOMessage(data, 0);
        // send message to receiver
        for (auto iterator = listener->_callbacks.begin();
                iterator != listener->_callbacks.end();
                ++iterator) {
             (*iterator)->receive_packet_callback(message);
        }

        releaseProcessedData(data);
    }
}

void ConnectionListener::add_receive_packet_callback(
        PacketReceiveCallbackInterface *packet_callback) {
    _callbacks.insert(packet_callback);
}

void ConnectionListener::finish() {
    _done = true;
}
