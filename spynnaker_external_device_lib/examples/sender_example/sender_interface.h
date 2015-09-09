#include "../SpynnakerLiveSpikesConnection.h"

class SenderInterface : public SpikesStartCallbackInterface{
public:
    void spikes_start(char *label, SpynnakerLiveSpikesConnection *connection);
    SenderInterface();
};