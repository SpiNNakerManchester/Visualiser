#include "../../SpynnakerLiveSpikesConnection.h"
#include <pthread.h>

class ContinueInterface : public SpikesStartCallbackInterface{
public:
    void spikes_start(char *label, SpynnakerLiveSpikesConnection *connection);
};
