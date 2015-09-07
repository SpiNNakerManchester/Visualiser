#include "ConnectionListener.h"
#include <iostream>
#include <stdio.h>

ConnectionListener::ConnectionListener(UDPConnection* connection){
    this->_connection = connection;
    this->data = (unsigned char *) malloc(EIEIOMessage::get_max_size());
}

void ConnectionListener::run(){
    while (!this->_done){
        try{
            int length = this->_connection->receive_data(
                this->data, EIEIOMessage::get_max_size());
            EIEIOMessage* message = new EIEIOMessage(this->data, 0);
            // send message to receiver
            printf("FUCKED UP BEFORE sending to callback \n");
            std::set<PacketReceiveCallbackInterface*>::iterator iterator;
            for (iterator = this->_callbacks.begin();
                    iterator != this->_callbacks.end(); ++iterator){
                 (*iterator)->receive_packet_callback(message);
            }
            printf("FUCKED UP BEFORE ending receive \n");
        }
        catch (std::exception& e){
            if (!this->_done){
                printf("thrown a error \n");
                std::cerr << "exception caught: " << e.what() << '\n';
            }
        }
    }
}

void ConnectionListener::add_receive_packet_callback(
        PacketReceiveCallbackInterface* packet_callback){
    this->_callbacks.insert(packet_callback);
}

void ConnectionListener::close(){
    this->_done = true;
    free(this->data);
}