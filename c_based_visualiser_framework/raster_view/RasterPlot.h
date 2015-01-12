/*
 * rasterplot.h
 *
 *  Created on: 2 Dec 2014
 *      Author: alan
 */

#ifndef RASTERPLOT_H_
#define RASTERPLOT_H_
#include "../utilities/SocketQueuer.h"
#include "../utilities/Threadable.h"
#include "../utilities/colour.h"
#include <pthread.h>
#include <deque>
#include <map>

class RasterPlot : public Threadable {
public:
	RasterPlot(int , char **);
    void start(SocketQueuer *, std::map<int, char*> *, std::map<int, int> *);
	void display(float);
	void reshape(int, int);
	virtual ~RasterPlot();

protected:
    void InternalThreadEntry();

private:
    static const int windowBorder = 110;

	std::map<int, struct colour> neuron_id_to_colour_map;
    std::deque< std::pair<int, int> > points_to_draw;
    bool do_update = false;
    pthread_mutex_t point_mutex;
    std::map<int, int> *key_to_neuron_id_map;
    std::map<int, char*> *y_axis_labels;
    int window;
    int windowWidth;
    int windowHeight;

	int plotTime = 1000;
	float timeStep = 0.1;
	int plotNeurons = 8000;

	static void displayWrapper();
	static void reshapeWrapper(int width, int height);
	static void idleWrapper();
	static void shutdownWrapper();

    void printgl(float, float, void *, char*, ...);
    void printglstroke(float, float, float, float, char*, ...);
	void safelyshut(void);
};

#endif /* RASTERPLOT_H_ */
