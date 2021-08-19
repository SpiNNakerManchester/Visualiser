#include "../../SpynnakerLiveSpikesConnection.h"
#include "sender_interface_forward.h"
#include "sender_interface_backward.h"
#include "receiver_interface.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#ifdef WIN32
#define sleep(n) Sleep(n)
#endif

int main(int argc, char **argv){
    try{
        // set up basic stuff
        char const* send_label1 = "spike_injector_forward";
        char const* send_label2 = "spike_injector_backward";
        char* send_labels[2] = {(char *) send_label1, (char*) send_label2};
        char const* label1 = "pop_forward";
        char const* label2 = "pop_backward";
        char* receive_labels[2] = {(char *) label1, (char*) label2};
        char const* local_host = NULL;
        SpynnakerLiveSpikesConnection connection =
            SpynnakerLiveSpikesConnection(
                2, receive_labels, 2, send_labels, (char*) local_host, 19999);
        // build the SpikeReceiveCallbackInterface
        pthread_mutex_t count_mutex;
        pthread_mutex_init(&count_mutex, NULL);

        SenderInterfaceForward* sender_callback_forward =
            new SenderInterfaceForward(&count_mutex);
        SenderInterfaceBackward* sender_callback_backward =
            new SenderInterfaceBackward(&count_mutex);

        ReceiverInterface* receiver_callback =
            new ReceiverInterface(&count_mutex);
        // register the callback with the SpynnakerLiveSpikesConnection
        connection.add_receive_callback(
            (char*) label1, receiver_callback);
        connection.add_receive_callback(
            (char*) label2, receiver_callback);

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
