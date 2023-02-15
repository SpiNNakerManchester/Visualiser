/*
 * Copyright (c) 2015 The University of Manchester
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RASTERPLOT_H_
#define RASTERPLOT_H_

#include "../glut_framework/GlutFramework.h"
#include "../utilities/ColourReader.h"
#include <SpynnakerLiveSpikesConnection.h>
#include <map>
#include <deque>
#include <set>
#include <pthread.h>

class RasterPlot : public GlutFramework, public SpikeReceiveCallbackInterface,
                   public SpikesStartCallbackInterface,
                   public SpikeInitializeCallbackInterface {

public:
    RasterPlot(
        int argc, char **argv, ColourReader *colour_reader, float ms_per_pixel,
        bool wait_for_start=true);
    void init();
    void main_loop();
    virtual void init_population(
        char *label, int n_neurons, float run_time_ms,
        float machine_time_step_ms);
    virtual void spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection);
    virtual void receive_spikes(
        char *label, int time, int n_spikes, int* spikes);
    void display(float time);
    void reshape(int width, int height);
    void keyboardUp(unsigned char key, int x, int y);
    virtual ~RasterPlot();

private:
    void printgl(float x, float y, void *font_style, char* format, ...);
    void printglstroke(float x, float y, float size, float rotate,
        char* format, ...);
    void safelyshut();

    constexpr static int WINDOW_BORDER = 110;
    constexpr static int INIT_WINDOW_WIDTH = 800;
    constexpr static int INIT_WINDOW_HEIGHT = 600;
    constexpr static double FRAMES_PER_SECOND = 60.0;
    constexpr static int INIT_WINDOW_X = 100;
    constexpr static int INIT_WINDOW_Y = 100;

    ColourReader *colour_reader;
    int argc;
    char **argv;

    float plot_time_ms;
    float timestep_ms;
    int n_neurons;
    int window_width;
    int window_height;
    float ms_per_pixel;
    float latest_time;

    std::deque<std::pair<int, int> > points_to_draw;
    pthread_mutex_t point_mutex;

    int base_pos;
    std::map<std::string, int> label_to_base_pos_map;
    std::map<int, struct colour> y_pos_to_colour_map;
    std::map<int, char*> y_axis_labels;

    pthread_mutex_t start_mutex;
    pthread_cond_t start_condition;
    int n_populations_to_read;
    bool database_read;
    bool user_pressed_start;
    bool simulation_started;
};

#endif /* RASTERPLOT_H_ */
