
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
#ifdef WIN32
    #include <windows.h>
    #include <GL/glut.h>
#elif defined(__APPLE__)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
    #include <GL/freeglut.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "PendulumPlot.h"
#include "../utilities/colour.h"
#include "../glut_framework/GlutFramework.h"


PendulumPlot::PendulumPlot(
        int argc, char **argv, bool wait_for_start,
        int input_width, int input_height,
        int output_width, int output_height, int output_offset,
        int output_per_core_width, int output_per_core_height) {
    this->window_width = input_width + (WINDOW_BORDER * 2);
    this->window_height = input_height + (WINDOW_BORDER * 2);
    this->user_pressed_start = !wait_for_start;
    this->simulation_started = false;
    this->database_read = false;
    this->timestep_ms = 0;
    this->input_width = input_width;
    this->input_height = input_height;
    this->output_width = output_width;
    this->output_height = output_height;
    this->output_offset = output_offset;
    this->output_per_core_width = output_per_core_width;
    this->output_per_core_height = output_per_core_height;

    this->argc = argc;
    this->argv = argv;

    this->points = (uint8_t *) malloc(input_width * input_height * sizeof(uint8_t));

    if (pthread_mutex_init(&(this->start_mutex), NULL) == -1) {
        fprintf(stderr, "Error initializing start mutex!\n");
        exit(-1);
    }
    if (pthread_cond_init(&(this->start_condition), NULL) == -1) {
        fprintf(stderr, "Error initializing start condition!\n");
        exit(-1);
    }

    if (pthread_mutex_init(&(this->point_mutex), NULL) == -1) {
        fprintf(stderr, "Error initializing point mutex!\n");
        exit(-1);
    }
}

void PendulumPlot::main_loop() {
    startFramework(argc, argv, "Pendulum Plot", INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT,
                   INIT_WINDOW_X, INIT_WINDOW_Y, FRAMES_PER_SECOND);
}

void PendulumPlot::init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glColor3f(1.0, 1.0, 1.0);
    glShadeModel(GL_SMOOTH);
}

void PendulumPlot::init_population(
        char *label, int n_neurons, float run_time_ms,
        float machine_time_step_ms) {
    fprintf(stderr, "Receiving population %s\n", label);
    std::string label_str = std::string(label);
    this->timestep_ms = machine_time_step_ms;

    pthread_mutex_lock(&(this->start_mutex));
    this->database_read = true;
    while (!this->user_pressed_start) {
        pthread_cond_wait(&(this->start_condition), &(this->start_mutex));
    }
    pthread_mutex_unlock(&(this->start_mutex));
}

void PendulumPlot::spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection) {
    pthread_mutex_lock(&(this->start_mutex));
    this->simulation_started = true;
    pthread_mutex_unlock(&(this->start_mutex));
}

void PendulumPlot::receive_spikes(
        char *label, int time, int n_spikes, int *spikes) {
    pthread_mutex_lock(&(this->point_mutex));
    for (int i = 0; i < n_spikes; i++) {
        this->points[spikes[i]] += 200;
    }
    pthread_mutex_unlock(&(this->point_mutex));
}

void PendulumPlot::receive_target_spikes(int time, int n_spikes, int *spikes) {
    pthread_mutex_lock(&(this->point_mutex));
    min_conv_x = input_height;
    min_conv_y = input_width;
    max_conv_x = 0;
    max_conv_y = 0;
    fprintf(stderr, "%u: [", time);
    for (int i = 0; i < n_spikes; i++) {
        int square_row = spikes[i] / (output_width * output_per_core_width);
        int rem = spikes[i] % (output_width * output_per_core_width);
        int square_col = rem / (output_per_core_width * output_per_core_height);
        rem = rem % (output_per_core_width * output_per_core_height);
        int in_square_y = rem / output_per_core_width;
        int in_square_x = rem % output_per_core_width;
        // Invert x and y as the covolution rotates
        int y = output_offset + square_row * output_per_core_height + in_square_y;
        int x = output_offset + square_col * output_per_core_width + in_square_x;
        min_conv_x = x < min_conv_x? x: min_conv_x;
        min_conv_y = y < min_conv_y? y: min_conv_y;
        max_conv_x = x > max_conv_x? x: max_conv_x;
        max_conv_y = y > max_conv_y? y: max_conv_y;
        fprintf(stderr, "%i, ", spikes[i]);
    }
    fprintf(stderr, "]\n");
    conv_decay = 4;
    pthread_mutex_unlock(&(this->point_mutex));
}

//-------------------------------------------------------------------------
//  Draws a string at the specified coordinates.
//-------------------------------------------------------------------------
void PendulumPlot::printgl(float x, float y, void *font_style,
        char* format, ...) {
    va_list arg_list;
    char str[256];
    int i;

    va_start(arg_list, format);
    vsprintf(str, format, arg_list);
    va_end(arg_list);

    const int width = glutBitmapLength(font_style, (const unsigned char *) str);
    float left = x - ((float) (width * window_ratio) / 2.0);
    if (left < 0.0) {
        left = 0.0;
    }

    glRasterPos2f(left + WINDOW_BORDER, y);
    glutBitmapString(font_style, (const unsigned char *) str);
}

static void line(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void PendulumPlot::display(float time) {
    if (glutGetWindow() == this->window) {

        glPointSize(1.0);

        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glColor4f(0.0, 0.0, 0.0, 1.0);

        int left = WINDOW_BORDER - 6;
        int right = window_width - (WINDOW_BORDER - 6);
        int top = window_height - (WINDOW_BORDER - 6);
        int bottom = WINDOW_BORDER - 6;

        pthread_mutex_lock(&(this->start_mutex));
        if (!this->database_read) {
            char prompt[] = "Waiting for database to be ready...";
            printgl((window_width / 2), top + 3,
                    GLUT_BITMAP_TIMES_ROMAN_24, prompt);
        } else if (!this->user_pressed_start) {
            char prompt[] = "Press space bar to start...";
            printgl((window_width / 2), top + 3,
                    GLUT_BITMAP_TIMES_ROMAN_24, prompt);
        } else if (!this->simulation_started) {
            char prompt[] = "Waiting for simulation to start...";
            printgl((window_width / 2), top + 3,
                    GLUT_BITMAP_TIMES_ROMAN_24, prompt);
        } else {
            char title[] = "Detection";
            printgl((window_width / 2), top + 3,
                    GLUT_BITMAP_TIMES_ROMAN_24, title);
        }
        pthread_mutex_unlock(&(this->start_mutex));

        glColor4f(0.0, 0.0, 0.0, 1.0);
        glLineWidth(1.0);
        line(left, bottom, right, bottom);
        line(left, top, right, top);
        line(left, bottom, left, top);
        line(right, bottom, right, top);

        pthread_mutex_lock(&(this->point_mutex));
        glPointSize(5.0);
        glBegin(GL_POINTS);
        for (int x = 0; x < input_width; x++) {
            for (int y = 0; y < input_height; y++) {
                float colour = 1.0 - ((float) points[y * input_width + x]) / 255.0f;
                glColor4f(colour, 1.0, colour, 1.0);
                float x_value = x + WINDOW_BORDER;
                float y_value = y + WINDOW_BORDER;
                glVertex2f(x_value, y_value);
                points[y * input_width + x] *= 0.5;
            }
        }
        glEnd();

        if (conv_decay > 0) {
            conv_decay -= 1;
        }
        if (conv_decay > 0) {
            glPointSize(1.0);
            glLineWidth(1.0);
            glBegin(GL_QUADS);
            glColor4f(1.0, 0.2, 0.2, 1.0);
            int min_x = min_conv_x + WINDOW_BORDER;
            int max_x = max_conv_x + WINDOW_BORDER;
            int min_y = min_conv_y + WINDOW_BORDER;
            int max_y = max_conv_y + WINDOW_BORDER;
            glVertex2f(min_x, min_y);
            glVertex2f(min_x, max_y);
            glVertex2f(max_x, max_y);
            glVertex2f(max_x, min_y);
            glEnd();
        }
        pthread_mutex_unlock(&(this->point_mutex));

        glutSwapBuffers();
    }
}

void PendulumPlot::reshape(int width, int height) {
    if (glutGetWindow() == this->window) {
        fprintf(stderr, "Reshape to %d, %d\n", width, height);

        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, window_width, 0.0, window_height, 1.0, -1.0);

        float ratioX = (float) width / window_width;
        float ratioY = (float) height / window_height;
        window_ratio = ratioX < ratioY ? ratioX: ratioY;

        int viewWidth = (int) (window_width * window_ratio);
        int viewHeight = (int) (window_height * window_ratio);
        int viewX = (int) ((width - window_width * window_ratio) / 2);
        int viewY = (int) ((height - window_height * window_ratio) / 2);

        glViewport(viewX, viewY, viewWidth, viewHeight);

        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity();
    }
}

void PendulumPlot::keyboardUp(unsigned char key, int x, int y) {
    if ((int) key == 32) {

        // create and send the eieio command message confirming database
        // read
        pthread_mutex_lock(&(this->start_mutex));
        if (!this->user_pressed_start) {
            printf("Starting the simulation\n");
            this->user_pressed_start = true;
            pthread_cond_signal(&(this->start_condition));
        }
        pthread_mutex_unlock(&(this->start_mutex));
    }
}

void PendulumPlot::safelyshut(void) {
    exit(0);                // kill program dead
}

PendulumPlot::~PendulumPlot() {
    // TODO Auto-generated destructor stub
}

PendulumPlot::TargetReceiver::TargetReceiver(PendulumPlot *plotter) {
    this->plotter = plotter;
}

void PendulumPlot::TargetReceiver::receive_spikes(char *label, int time, int n_spikes, int *spikes) {
    plotter->receive_target_spikes(time, n_spikes, spikes);
}
