#include "icub.h"
#include <chrono>
#include <thread>

int main(int argc, char **argv) {

    // Set up the ICUB interface
    ICubInterface *icub = new ICubInterface(
        (char *) "from_icub", (char *) "to_icub");

    int numSpikes = 0;
    int neuronID = 0;
    int stepsToRun = 20;
    int count = 0;

    // Pre-load some spikes to send queue
    for (int s = 0; s < 146; s++) {
       icub->addSpikeToSendQueue(neuronID);
       if (neuronID == 0) {
           neuronID = 1;
       } else{
           neuronID = 0;
       }
    }

    // enable the queue for processing
    icub->enableSendQueue();

    // wait for start of simulation
    fprintf(stderr, "Waiting for simulation to start...\n");
    icub->waitForStart();
    fprintf(stderr, "Running...\n");

    // run for the number of steps defined above
    while (count < stepsToRun) {
        int recvSize = icub->getRecvQueueSize();

        // If there are incoming packets in the receiver queue
        // then process them and increment count of spikes

        if (recvSize > 0) {
            std::pair<int, int> spike = icub->getNextSpike();
            numSpikes++;
            fprintf(stderr, "Time %d, ID %d, total %d recv %d\n",
                    spike.first, spike.second, numSpikes, recvSize);
        }

        // continue to add new spikes to the send queue
        icub->addSpikeToSendQueue(neuronID);
        if (neuronID == 0) {
            neuronID = 1;
        }else{
            neuronID = 0;
        }

        // wait here
        std::this_thread::sleep_for(std::chrono::seconds(1));

        count++;

    } // main processing loop
}
