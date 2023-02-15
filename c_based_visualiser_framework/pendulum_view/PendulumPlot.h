/*
 * Copyright (c) 2015-2021 The University of Manchester
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef RASTERPLOT_H_
#define RASTERPLOT_H_

#include "../glut_framework/GlutFramework.h"
#include "../utilities/colour.h"
#include <SpynnakerLiveSpikesConnection.h>
#include <map>
#include <deque>
#include <set>
#include <pthread.h>

class PendulumPlot : public GlutFramework, public SpikeReceiveCallbackInterface,
                   public SpikesStartCallbackInterface,
                   public SpikeInitializeCallbackInterface {

public:
    PendulumPlot(
            int argc, char **argv,
            bool wait_for_start,
            int input_width, int input_height,
            int output_width, int output_height, int output_offset,
            int output_per_core_width, int output_per_core_height);
    void init();
    void main_loop();
    virtual void init_population(
        char *label, int n_neurons, float run_time_ms,
        float machine_time_step_ms);
    virtual void spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection);
    virtual void receive_spikes(
        char *label, int time, int n_spikes, int* spikes);
    void receive_target_spikes(int time, int n_spikes, int *spikes);
    void display(float time);
    void reshape(int width, int height);
    void keyboardUp(unsigned char key, int x, int y);
    SpikeReceiveCallbackInterface *get_target_receiver() {
        return new TargetReceiver(this);
    }
    virtual ~PendulumPlot();

private:

    class TargetReceiver : public SpikeReceiveCallbackInterface {
        public:
            TargetReceiver(PendulumPlot *plotter);
            virtual void receive_spikes(
                    char *label, int time, int n_spikes, int* spikes);
            PendulumPlot *plotter;
        };

    void printgl(float x, float y, void *font_style, char* format, ...);
    void safelyshut();

    constexpr static int WINDOW_BORDER = 20;
    constexpr static int INIT_WINDOW_WIDTH = 800;
    constexpr static int INIT_WINDOW_HEIGHT = 600;
    constexpr static double FRAMES_PER_SECOND = 60.0;
    constexpr static int INIT_WINDOW_X = 100;
    constexpr static int INIT_WINDOW_Y = 100;

    int argc;
    char **argv;

    float last_plot_time_ms = 0;
    float timestep_ms;
    int window_width;
    int window_height;
    float window_ratio = 1.0;

    int input_width;
    int input_height;
    int output_width;
    int output_height;
    int output_offset;
    int output_per_core_width;
    int output_per_core_height;

    int min_conv_x = -1;
    int max_conv_x = -1;
    int min_conv_y = -1;
    int max_conv_y = -1;
    int conv_decay = 0;

    uint8_t *points;
    pthread_mutex_t point_mutex;

    pthread_mutex_t start_mutex;
    pthread_cond_t start_condition;
    bool database_read;
    bool user_pressed_start;
    bool simulation_started;
};

#endif /* RASTERPLOT_H_ */
