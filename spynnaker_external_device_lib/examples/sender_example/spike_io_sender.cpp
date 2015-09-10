#include "../../SpynnakerLiveSpikesConnection.h"
#include "sender_interface_forward.h"
#include "sender_interface_backward.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int main(int argc, char **argv){
    try{
        // set up basic stuff
        char const* send_label1 = "spike_injector_forward";
        char const* send_label2 = "spike_injector_backward";
        char* send_labels[2] = {(char *) send_label1, (char*) send_label2};
        char* receive_labels[0];
        char const* local_host = NULL;
        SpynnakerLiveSpikesConnection connection =
            SpynnakerLiveSpikesConnection(
                0, receive_labels, 2, send_labels, (char*) local_host, 19999);
        // build the SpikeReceiveCallbackInterface
        pthread_mutex_t count_mutex;
        pthread_mutex_init(&count_mutex, NULL);
        SenderInterfaceForward* sender_callback_forward =
            new SenderInterfaceForward(&count_mutex);
        SenderInterfaceBackward* sender_callback_backward =
            new SenderInterfaceBackward(&count_mutex);

        // register the callback with the SpynnakerLiveSpikesConnection
        connection.add_start_callback((char *) send_label1,
                                      sender_callback_forward);
        connection.add_start_callback((char *) send_label2,
                                      sender_callback_backward);

        while(true){
            sleep(1);
        }
    }
    catch (char const* msg){
        printf("%s \n", msg);
    }
}