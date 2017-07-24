#include "ConnectionListener.h"
#include <iostream>
#include <stdio.h>

ConnectionListener::ConnectionListener(UDPConnection* connection) {
    this->_connection = connection;
    if (pthread_mutex_init(&this->free_data_mutex, NULL) == -1) {
        fprintf(stderr, "Error initializing free data mutex\n");
        exit(-1);
    }
    if (pthread_mutex_init(&this->data_to_process_mutex, NULL) == -1) {
        fprintf(stderr, "Error initializing data to process mutex\n");
        exit(-1);
    }
    if (pthread_cond_init(&this->free_data_condition, NULL) == -1) {
        fprintf(stderr, "Error initializing free data condition\n");
        exit(-1);
    }
    if (pthread_cond_init(&this->data_to_process_condition, NULL) == -1) {
        fprintf(stderr, "Error initializing data to process condition\n");
        exit(-1);
    }
    for (int i = 0; i < 256; i++) {
        this->free_data.push(
            (unsigned char *) malloc(EIEIOMessage::get_max_size()));
    }
    this->_done = false;
    this->reader = new Reader(this);
    this->reader->start();
}

ConnectionListener::~ConnectionListener() {
    // TODO Full teardown required here
}

unsigned char *ConnectionListener::getFreeData() {
    unsigned char *data = nullptr;
    pthread_mutex_lock(&this->free_data_mutex);
    try{
        while (this->free_data.empty()) {
            pthread_cond_wait(
                &this->free_data_condition, &this->free_data_mutex);
        }
        data = this->free_data.front();
        this->free_data.pop();
    } catch (std::exception& e) {
        if (!this->_done) {
            printf("thrown a error \n");
            std::cerr << "exception caught: " << e.what() << '\n';
        }
    }
    pthread_mutex_unlock(&this->free_data_mutex);
    return data;
}

void ConnectionListener::postReceivedData(unsigned char *data) {
    pthread_mutex_lock(&this->data_to_process_mutex);
    try {
	this->data_to_process.push(data);
	pthread_cond_signal(&this->data_to_process_condition);
    } catch (std::exception& e) {
        if (!this->_done) {
            printf("thrown a error \n");
            std::cerr << "exception caught: " << e.what() << '\n';
        }
    }
    pthread_mutex_unlock(&this->data_to_process_mutex);
}

void ConnectionListener::run() {
    while (!this->_done) {
	unsigned char *data = getFreeData();
	if (data == nullptr)
	    continue;
        try{
            int length = this->_connection->receive_data(
                data, EIEIOMessage::get_max_size());
            // TODO Check if the length of the received data is sensible
            postReceivedData(data);
        } catch (std::exception& e) {
            if (!this->_done) {
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
    pthread_mutex_lock(&this->listener->data_to_process_mutex);
    while (this->listener->data_to_process.empty()) {
        pthread_cond_wait(&this->listener->data_to_process_condition,
                          &this->listener->data_to_process_mutex);
    }

    unsigned char *data = this->listener->data_to_process.front();
    this->listener->data_to_process.pop();

    pthread_mutex_unlock(&this->listener->data_to_process_mutex);
    return data;
}

void ConnectionListener::Reader::releaseProcessedData(unsigned char *data) {
    pthread_mutex_lock(&this->listener->free_data_mutex);
    this->listener->free_data.push(data);
    pthread_cond_signal(&this->listener->free_data_condition);
    pthread_mutex_unlock(&this->listener->free_data_mutex);
}

void ConnectionListener::Reader::run() {
    while (!this->listener->_done) {
        unsigned char *data = getDataToProcess();

        EIEIOMessage* message = new EIEIOMessage(data, 0);
        // send message to receiver
        for (auto iterator = this->listener->_callbacks.begin();
                iterator != this->listener->_callbacks.end();
                ++iterator) {
             (*iterator)->receive_packet_callback(message);
        }

        releaseProcessedData(data);
    }
}

void ConnectionListener::add_receive_packet_callback(
        PacketReceiveCallbackInterface *packet_callback) {
    this->_callbacks.insert(packet_callback);
}

void ConnectionListener::finish() {
    this->_done = true;
}
