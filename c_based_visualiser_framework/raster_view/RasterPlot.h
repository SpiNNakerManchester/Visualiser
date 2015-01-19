/*
 * rasterplot.h
 *
 *  Created on: 2 Dec 2014
 *      Author: alan
 */

#ifndef RASTERPLOT_H_
#define RASTERPLOT_H_
#include "../utilities/SocketQueuer.h"
#include "../glut_framework/GlutFramework.h"
#include <map>
#include <deque>

using namespace glutFramework;

class RasterPlot : public GlutFramework {

public:
	RasterPlot(int , char **, SocketQueuer *, std::map<int, char*> *,
	           std::map<int, int> *, std::map<int, struct colour> *,
	           int, float);
	void init();
	void display(float);
	void reshape(int, int);
	virtual ~RasterPlot();

private:
    void printgl(float, float, void *, char*, ...);
    void printglstroke(float, float, float, float, char*, ...);
	void safelyshut(void);

	const static int WINDOW_BORDER = 110;
	const static int INIT_WINDOW_WIDTH = 800;
	const static int INIT_WINDOW_HEIGHT = 600;
	const static double FRAMES_PER_SECOND = 60.0;
	const static int INIT_WINDOW_X = 100;
	const static int INIT_WINDOW_Y = 100;

	int plot_time_ms;
	float timestep_ms;
	int n_neurons;
	int window_width;
	int window_height;

	std::deque<std::pair<int, int> > points_to_draw;
	pthread_mutex_t point_mutex;

    std::map<int, struct colour> *neuron_id_to_colour_map;
	std::map<int, int> *key_to_neuronid_map;
	std::map<int, char*> *y_axis_labels;
};

#endif /* RASTERPLOT_H_ */
