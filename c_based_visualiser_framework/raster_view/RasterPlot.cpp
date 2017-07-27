#include <GL/glut.h>
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "RasterPlot.h"
#include "../utilities/colour.h"
#include "../glut_framework/GlutFramework.h"


RasterPlot::RasterPlot(
        int argc, char **argv, ColourReader *colour_reader,
        float ms_per_pixel, bool wait_for_start) {
    this->window_width = INIT_WINDOW_WIDTH;
    this->window_height = INIT_WINDOW_HEIGHT;
    this->base_pos = 0;
    this->user_pressed_start = !wait_for_start;
    this->simulation_started = false;
    this->database_read = false;
    this->n_neurons = 0;
    this->timestep_ms = 0;
    this->plot_time_ms = 0;
    this->ms_per_pixel = ms_per_pixel;
    this->latest_time = 0.0;

    this->n_populations_to_read = colour_reader->get_labels()->size();

    this->colour_reader = colour_reader;
    this->argc = argc;
    this->argv = argv;

    if (pthread_mutex_init(&start_mutex, NULL) == -1) {
        fprintf(stderr, "Error initializing start mutex!\n");
        exit(-1);
    }
    if (pthread_cond_init(&start_condition, NULL) == -1) {
        fprintf(stderr, "Error initializing start condition!\n");
        exit(-1);
    }
    if (pthread_mutex_init(&point_mutex, NULL) == -1) {
        fprintf(stderr, "Error initializing point mutex!\n");
        exit(-1);
    }
}

void RasterPlot::main_loop() {
    startFramework(argc, argv, "Raster Plot", window_width, window_height,
                   INIT_WINDOW_X, INIT_WINDOW_Y, double(FRAMES_PER_SECOND));
}

void RasterPlot::init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glColor3f(1.0, 1.0, 1.0);
    glShadeModel(GL_SMOOTH);
}

void RasterPlot::init_population(
        char *label, int n_neurons, float run_time_ms,
        float machine_time_step_ms) {
    std::string label_str = std::string(label);
    plot_time_ms = run_time_ms;
    timestep_ms = machine_time_step_ms;

    char *y_axis_label = (char *) malloc(sizeof(char) * strlen(label));
    strcpy(y_axis_label, label);
    y_axis_labels[base_pos + n_neurons / 2] = y_axis_label;
    label_to_base_pos_map[label_str] = base_pos;
    struct colour colour = colour_reader->get_colour(label);
    for (int i = 0; i < n_neurons; i++) {
        y_pos_to_colour_map[base_pos + i] = colour;
    }
    this->n_neurons += n_neurons;
    base_pos += n_neurons + 10;


    pthread_mutex_lock(&start_mutex);
    n_populations_to_read -= 1;
    if (n_populations_to_read <= 0) {
        database_read = true;
        while (!user_pressed_start) {
            pthread_cond_wait(&start_condition, &start_mutex);
        }
    }
    pthread_mutex_unlock(&start_mutex);
}

void RasterPlot::spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection) {
    pthread_mutex_lock(&start_mutex);
    simulation_started = true;
    pthread_mutex_unlock(&start_mutex);
}

void RasterPlot::receive_spikes(
        char *label, int time, int n_spikes, int *spikes) {
    pthread_mutex_lock(&point_mutex);
    std::string label_str = std::string(label);
    int base_pos = label_to_base_pos_map[label_str];
    for (int i = 0; i < n_spikes; i++) {
        std::pair<int, int> point(time, spikes[i] + base_pos);
        points_to_draw.push_back(point);
    }
    float time_ms = time * timestep_ms;
    if (time_ms > latest_time) {
        latest_time = time_ms;
    }
    pthread_mutex_unlock(&point_mutex);
}

//-------------------------------------------------------------------------
//  Draws a string at the specified coordinates.
//-------------------------------------------------------------------------
void RasterPlot::printgl(float x, float y, void *font_style,
        char* format, ...) {
    va_list arg_list;
    char str[256];
    int i;

    va_start(arg_list, format);
    vsprintf(str, format, arg_list);
    va_end(arg_list);

    glRasterPos2f(x, y);

    for (i = 0; str[i] != '\0'; i++) {
        glutBitmapCharacter(font_style, str[i]);
    }
}

void RasterPlot::printglstroke(float x, float y, float size, float rotate,
        char* format, ...) {
    va_list arg_list;
    char str[256];
    int i;
    GLvoid *font_style = GLUT_STROKE_ROMAN;

    va_start(arg_list, format);
    vsprintf(str, format, arg_list);
    va_end(arg_list);

    glPushMatrix();
    glEnable (GL_BLEND);   // antialias the font
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_LINE_SMOOTH);
    glLineWidth(1.5);   // end setup for antialiasing
    glTranslatef(x, y, 0);
    glScalef(size, size, size);
    glRotatef(rotate, 0.0, 0.0, 1.0);
    for (i = 0; str[i] != '\0'; i++)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
    glPopMatrix();
}

void RasterPlot::display(float time) {
    if (glutGetWindow() == window) {
        glPointSize(1.0);

        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glColor4f(0.0, 0.0, 0.0, 1.0);

        float start = 0.0;
        float end = plot_time_ms;
        if (ms_per_pixel != 0.0) {
            end = latest_time;
            start = end - (window_width - (2 * WINDOW_BORDER)) * ms_per_pixel;
            if (start < 0.0) {
                start = 0.0;
                end = start +
                    (window_width - (2 * WINDOW_BORDER)) * ms_per_pixel;
            }
        }
        float x_spacing = (float) (window_width - (2 * WINDOW_BORDER)) /
            ((end - start) / timestep_ms);
        float y_spacing = (float) (window_height - (2 * WINDOW_BORDER))
                / (float) base_pos;

        char x_axis[] = "Simulation Time (ms)";
        printglstroke((window_width / 2) - 100, 20, 0.12, 0, x_axis);
        char label_0[] = "%.1f";
        printglstroke(
            WINDOW_BORDER - 15, WINDOW_BORDER - 20, 0.10, 0, label_0, start);
        char label_max[] = "%.1f";
        printglstroke(
            window_width - WINDOW_BORDER - 20, WINDOW_BORDER - 20,
            0.10, 0, label_max, end);

        for (auto iter = y_axis_labels.begin();
                iter != y_axis_labels.end(); ++iter) {
            float y_value = ((iter->first * y_spacing) + WINDOW_BORDER) - 10;
            float width = glutStrokeLength(
                GLUT_STROKE_ROMAN,
                reinterpret_cast<const unsigned char*> (iter->second)) * 0.1;
            char y_label[] = "%s";
            printglstroke((WINDOW_BORDER - width) - 20, y_value, 0.10, 0,
                    iter->second);
        }

        pthread_mutex_lock(&start_mutex);
        if (!database_read) {
            char prompt[] = "Waiting for database to be ready...";
            printgl((window_width / 2) - 120, window_height - 50,
                    GLUT_BITMAP_TIMES_ROMAN_24, prompt);
        } else if (!user_pressed_start) {
            char prompt[] = "Press space bar to start...";
            printgl((window_width / 2) - 100, window_height - 50,
                    GLUT_BITMAP_TIMES_ROMAN_24, prompt);
        } else if (!simulation_started) {
            char prompt[] = "Waiting for simulation to start...";
            printgl((window_width / 2) - 120, window_height - 50,
                    GLUT_BITMAP_TIMES_ROMAN_24, prompt);
        } else {
            char title[] = "Raster Plot";
            printgl((window_width / 2) - 75, window_height - 50,
                    GLUT_BITMAP_TIMES_ROMAN_24, title);
        }
        pthread_mutex_unlock(&start_mutex);

        glColor4f(0.0, 0.0, 0.0, 1.0);
        glLineWidth(1.0);
        glBegin (GL_LINES);
        glVertex2f(window_width - WINDOW_BORDER, WINDOW_BORDER);
        glVertex2f(WINDOW_BORDER - 10, WINDOW_BORDER);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(WINDOW_BORDER - 10, WINDOW_BORDER);
        glVertex2f(WINDOW_BORDER - 10, window_height - WINDOW_BORDER);
        glEnd();

        glPointSize(2.0);
        glBegin(GL_POINTS);

        int start_tick = 0;
        int end_tick = this->plot_time_ms / this->timestep_ms;
        if (ms_per_pixel != 0.0) {
            start_tick = start / this->timestep_ms;
            end_tick = end / this->timestep_ms;
        }

        pthread_mutex_lock(&point_mutex);
        for (auto iter = points_to_draw.begin();
        	iter != points_to_draw.end(); ++iter) {
            auto colour_value = y_pos_to_colour_map.find(iter->second);
            if (colour_value == y_pos_to_colour_map.end()) {
                fprintf(stderr, "Missing colour for neuron %d\n", iter->second);
                continue;
            }
            struct colour colour = colour_value->second;
            glColor4f(colour.r, colour.g, colour.b, 1.0);
            if (iter->first >= start_tick && iter->first <= end_tick) {
                float x_value = ((iter->first - start_tick) * x_spacing)
                                 + WINDOW_BORDER;
                float y_value = (iter->second * y_spacing) + WINDOW_BORDER + 1;

                glVertex2f(x_value, y_value);
            }
        }
        pthread_mutex_unlock(&point_mutex);

        glEnd();
        glutSwapBuffers();
    }
}

void RasterPlot::reshape(int width, int height) {
    if (glutGetWindow() == this->window) {
        fprintf(stderr, "Reshape to %d, %d\n", width, height);
        this->window_width = width;
        this->window_height = height;

        // viewport dimensions
        glViewport(0, 0, (GLsizei) width, (GLsizei) height);
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        // an orthographic projection. Should probably look into OpenGL
        // perspective projections for 3D if that's your thing
        glOrtho(0.0, width, 0.0, height, -50.0, 50.0);
        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity();
    }
}

void RasterPlot::keyboardUp(unsigned char key, int x, int y) {
    if ((int) key == 32) {
        // create and send the eieio command message confirming database
        // read
        pthread_mutex_lock(&start_mutex);
        if (!user_pressed_start) {
            printf("Starting the simulation\n");
            user_pressed_start = true;
            pthread_cond_signal(&start_condition);
        }
        pthread_mutex_unlock(&start_mutex);
    }
}

void RasterPlot::safelyshut(void) {
    exit(0);                // kill program dead
}

RasterPlot::~RasterPlot() {
    // TODO Auto-generated destructor stub
}
