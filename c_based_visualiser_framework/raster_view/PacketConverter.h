#include <deque>
#include <pthread.h>
#include <map>
#include <list>
#include "../utilities/Threadable.h"
#include "../utilities/EIEIOMessage.h"
#include "../utilities/SocketQueuer.h"

using namespace std;

#ifndef C_VIS_DATABASE_RASTER_VIEW_PACKETCONVERTER_H_
#define C_VIS_DATABASE_RASTER_VIEW_PACKETCONVERTER_H_

class PacketConverter : public Threadable {
public:
    PacketConverter(SocketQueuer *, deque< pair<int, int> >*, pthread_mutex_t*,
                    map<int, int>*);
    virtual ~PacketConverter();
protected:
    virtual void InternalThreadEntry();
private:
    deque<pair<int, int> > *points_to_plot;
    SocketQueuer *packet_queue;
    pthread_mutex_t* point_mutex;
    map<int, int> *neuron_id_mapping;
    const static int SIZE_OF_KEY = 4;
    void translate_eieio_message_to_points(eieio_message &,
                                        list<pair<int, int> > &);

};

#endif /* C_VIS_DATABASE_RASTER_VIEW_PACKETCONVERTER_H_ */
