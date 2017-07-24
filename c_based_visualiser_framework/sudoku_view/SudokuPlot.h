#ifndef RASTERPLOT_H_
#define RASTERPLOT_H_

#include "../glut_framework/GlutFramework.h"
#include <SpynnakerLiveSpikesConnection.h>
#include <map>
#include <deque>
#include <set>
#include <pthread.h>

class SudokuPlot : public GlutFramework, public SpikeReceiveCallbackInterface,
                   public SpikesStartCallbackInterface,
                   public SpikeInitializeCallbackInterface {

public:
    SudokuPlot(
        int argc, char **argv, int neurons_per_number, float ms_per_bin,
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
    virtual ~SudokuPlot();

private:
    void printgl(float x, float y, void *font_style, char* format, ...);
    void printglstroke(float x, float y, float size, float rotate,
        char* format, ...);
    void safelyshut();

    const static int WINDOW_BORDER = 110;
    const static int INIT_WINDOW_WIDTH = 800;
    const static int INIT_WINDOW_HEIGHT = 600;
    const static int FRAMES_PER_SECOND = 60;
    const static int INIT_WINDOW_X = 100;
    const static int INIT_WINDOW_Y = 100;

    int argc;
    char **argv;

    float plot_time_ms;
    float timestep_ms;
    int n_neurons;
    int window_width;
    int window_height;
    float ms_per_bin;
    float latest_time;
    int neurons_per_number;

    std::deque<std::pair<int, int> > points_to_draw[81];
    pthread_mutex_t point_mutex;

    int cell_id;
    std::map<std::string, int> label_to_cell_map;
    std::map<int, int> cell_size_map;
    std::map<int, char*> cell_labels;

    pthread_mutex_t start_mutex;
    pthread_cond_t start_condition;
    int n_populations_to_read;
    bool database_read;
    bool user_pressed_start;
    bool simulation_started;
};

#endif /* RASTERPLOT_H_ */
