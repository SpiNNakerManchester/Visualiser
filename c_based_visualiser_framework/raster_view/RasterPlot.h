/*
 * rasterplot.h
 *
 *  Created on: 2 Dec 2014
 *      Author: alan
 */

#ifndef RASTERPLOT_H_
#define RASTERPLOT_H_
#include "../utilities/DatabaseMessageConnection.h"
#include "../glut_framework/GlutFramework.h"
#include <map>
#include <deque>
#include <pthread.h>

using namespace glutFramework;

class RasterPlot : public GlutFramework {

public:
    RasterPlot(int argc, char **argv, char *remote_host,
            std::vector<int> *ports,
            std::map<int, char*> *y_axis_labels,
            std::map<int, int> *key_to_neuronid_map,
            std::map<int, struct colour> *neuron_id_to_colour_map,
            float plot_time_ms, float timestep_ms, int n_neurons,
            DatabaseMessageConnection *database_message_connection);
    void init();
    void display(float time);
    void reshape(int width, int height);
    void keyboardUp(unsigned char key, int x, int y);
    virtual ~RasterPlot();

private:
    void printgl(float x, float y, void *font_style, char* format, ...);
    void printglstroke(float x, float y, float size, float rotate,
        char* format, ...);
    void safelyshut();

    const static int WINDOW_BORDER = 110;
    const static int INIT_WINDOW_WIDTH = 800;
    const static int INIT_WINDOW_HEIGHT = 600;
    const static double FRAMES_PER_SECOND = 60.0;
    const static int INIT_WINDOW_X = 100;
    const static int INIT_WINDOW_Y = 100;

    float plot_time_ms;
    float timestep_ms;
    int n_neurons;
    int window_width;
    int window_height;

    std::deque<std::pair<int, int> > points_to_draw;
    pthread_mutex_t point_mutex;

    std::map<int, struct colour> *neuron_id_to_colour_map;
    std::map<int, int> *key_to_neuronid_map;
    std::map<int, char*> *y_axis_labels;

    DatabaseMessageConnection *database_message_connection;
};

#endif /* RASTERPLOT_H_ */
