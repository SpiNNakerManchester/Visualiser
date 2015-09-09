#include "../SpynnakerLiveSpikesConnection.h"
#include "receiver_interface.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int main(int argc, char **argv){
    try{
        // set up basic stuff
        char const* send_label1 = "pop_forward";
        char const* send_label2 = "pop_backward";
        char* send_labels[2] = {(char *) send_label1, (char*) send_label2};
        char* recieve_labels[0];
        char const* local_host = NULL;
        SpynnakerLiveSpikesConnection connection = SpynnakerLiveSpikesConnection(
            2, receive_labels, 0, send_labels, (char*) local_host, 19996);
        // build the SpikeReceiveCallbackInterface
        pthread_cond_t *cond = new pthread_cond_t();
        SenderInterface* sender_callback = new SenderInterface(cond);

        // register the callback with the SpynnakerLiveSpikesConnection
        connection.add_start_callback((char *) send_label1, )

        while(true){
            sleep(1);
        }
    }
    catch (char const* msg){
        printf("%s \n", msg);
    }
}