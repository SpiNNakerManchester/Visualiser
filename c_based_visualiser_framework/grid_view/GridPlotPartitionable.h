#ifndef RASTERPLOT_H_
#define RASTERPLOT_H_

#include "../glut_framework/GlutFramework.h"
#include "../utilities/GridColourReader.h"
#include <SpiNNakerFrontEndCommonLiveEventsConnection.h>
#include <map>
#include <deque>
#include <set>
#include <pthread.h>
#include <string.h>

//! state_to_draw_struct
typedef struct state_to_draw_struct {
    uint32_t cell_id;
    uint32_t state;
} state_to_draw_struct;


class GridPlotPartitionable : public GlutFramework,
        public EventReceiveCallbackPayloadInterface,
        public EventsStartCallbackInterface,
        public EventInitializeCallbackInterface {

public:
    GridPlotPartitionable(
        int argc, char **argv, int grid_size_x, int grid_size_y,
         GridColourReader *colour_reader, bool wait_for_start=true);
    void init();
    void main_loop();
    virtual void init_population(
        char *label, int n_neurons, float run_time_ms,
        float machine_time_step_ms);
    virtual void events_start(
        char *label, SpiNNakerFrontEndCommonLiveEventsConnection *connection);
    virtual void receive_events(char *label, int event, int payload);
    void display(float time);
    void reshape(int width, int height);
    void keyboardUp(unsigned char key, int x, int y);
    virtual ~GridPlotPartitionable();

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

    GridColourReader *colour_reader;

    int argc;
    char **argv;

    float plot_time_ms;
    float timestep_ms;
    int n_cells;
    int grid_size_x;
    int grid_size_y;
    int window_width;
    int window_height;
    std::string cell_label;
    int points_recieved;

    std::deque<state_to_draw_struct> *states_to_draw;
    pthread_mutex_t point_mutex;

    pthread_mutex_t start_mutex;
    pthread_cond_t start_condition;
    bool database_read;
    bool user_pressed_start;
    bool simulation_started;
};

#endif /* RASTERPLOT_H_ */
