#include "../../SpynnakerLiveSpikesConnection.h"
#include "receiver_interface.h"
#include <stdio.h>
#include <unistd.h>
#ifdef WIN32
#define sleep(n) Sleep(n)
#endif

int main(int argc, char **argv){
    try{
        // set up basic stuff
        char const* receive_label1 = "pop_forward";
        char const* receive_label2 = "pop_backward";
        char* receive_labels[2] = {(char *) receive_label1, (char*) receive_label2};
        char* send_labels[0];
        char const* local_host = NULL;
        SpynnakerLiveSpikesConnection connection = SpynnakerLiveSpikesConnection(
            2, receive_labels, 0, send_labels, (char*) local_host, 19996);
        // build the SpikeReceiveCallbackInterface
        ReceiverInterface* receiver_callback = new ReceiverInterface();
        // register the callback with the SpynnakerLiveSpikesConnection
        connection.add_receive_callback((char*) receive_label1, receiver_callback);
        connection.add_receive_callback((char*) receive_label2, receiver_callback);

        while(true){
            sleep(1);
        }
    }
    catch (char const* msg){
        printf("%s \n", msg);
    }
}
