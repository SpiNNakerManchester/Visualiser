#include "../../SpynnakerLiveSpikesConnection.h"
#include "receiver_interface.h"
#include "continue_interface.h"
#include <stdio.h>
#include <unistd.h>
#ifdef WIN32
#define sleep(n) Sleep(n)
#endif

int main(int argc, char **argv){
    try{
        // set up basic stuff
        char const* receive_label = "ssa";
        char* receive_labels[1] = {(char *) receive_label};
        char* send_labels[0];
        char const* local_host = NULL;
        SpynnakerLiveSpikesConnection connection = SpynnakerLiveSpikesConnection(
            1, receive_labels, 0, send_labels, (char*) local_host, 19999);
        // build the SpikeReceiveCallbackInterface
        ReceiverInterface* receiver_callback = new ReceiverInterface();
        // register the callback with the SpynnakerLiveSpikesConnection
        connection.add_receive_callback((char*) receive_label, receiver_callback);

        ContinueInterface *continue_callback = new ContinueInterface();
        connection.add_start_callback((char *) receive_label, continue_callback);

        while(true){
            sleep(1);
        }
    }
    catch (char const* msg){
        printf("%s \n", msg);
    }
}
