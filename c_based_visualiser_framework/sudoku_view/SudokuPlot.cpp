#include <GL/glut.h>
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "SudokuPlot.h"
#include "../utilities/colour.h"
#include "../glut_framework/GlutFramework.h"


SudokuPlot::SudokuPlot(
        int argc, char **argv, int neurons_per_number,
        float ms_per_bin, bool wait_for_start) {
    this->window_width = INIT_WINDOW_WIDTH;
    this->window_height = INIT_WINDOW_HEIGHT;
    this->cell_id = 0;
    this->user_pressed_start = !wait_for_start;
    this->simulation_started = false;
    this->database_read = false;
    this->n_neurons = 0;
    this->timestep_ms = 0;
    this->plot_time_ms = 0;
    this->ms_per_bin = ms_per_bin;
    this->latest_time = 0.0;
    this->neurons_per_number = neurons_per_number;

    this->n_populations_to_read = 1;

    this->argc = argc;
    this->argv = argv;

    if (pthread_mutex_init(&this->start_mutex, NULL) == -1) {
        fprintf(stderr, "Error initializing start mutex!\n");
        exit(-1);
    }
    if (pthread_cond_init(&this->start_condition, NULL) == -1) {
        fprintf(stderr, "Error initializing start condition!\n");
        exit(-1);
    }

    if (pthread_mutex_init(&this->point_mutex, NULL) == -1) {
        fprintf(stderr, "Error initializing point mutex!\n");
        exit(-1);
    }
}

void SudokuPlot::main_loop() {
    startFramework(argc, argv, "Sudoku", window_width, window_height,
	    INIT_WINDOW_X, INIT_WINDOW_Y, double(FRAMES_PER_SECOND));
}

void SudokuPlot::init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glColor3f(1.0, 1.0, 1.0);
    glShadeModel(GL_SMOOTH);
}

void SudokuPlot::init_population(
        char *label, int n_neurons, float run_time_ms,
        float machine_time_step_ms) {
    std::string label_str = std::string(label);
    this->plot_time_ms = run_time_ms;
    this->timestep_ms = machine_time_step_ms;

    char *cell_label = (char *) malloc(sizeof(char) * strlen(label));
    strcpy(cell_label, label);
    this->cell_labels[this->cell_id] = cell_label;
    this->cell_size_map[this->cell_id] = n_neurons;
    this->label_to_cell_map[label_str] = this->cell_id;
    this->n_neurons += n_neurons;
    this->cell_id += 1;

    pthread_mutex_lock(&this->start_mutex);
    this->n_populations_to_read -= 1;
    if (this->n_populations_to_read <= 0) {
        this->database_read = true;
        while (!this->user_pressed_start) {
            pthread_cond_wait(&this->start_condition, &this->start_mutex);
        }
    }
    pthread_mutex_unlock(&this->start_mutex);
}

void SudokuPlot::spikes_start(
        char *label, SpynnakerLiveSpikesConnection *connection) {
    pthread_mutex_lock(&this->start_mutex);
    this->simulation_started = true;
    pthread_mutex_unlock(&this->start_mutex);
}

void SudokuPlot::receive_spikes(
        char *label, int time, int n_spikes, int *spikes) {
    pthread_mutex_lock(&this->point_mutex);
    std::string label_str = std::string(label);
    for (int i = 0; i < n_spikes; i++) {
        int cell_id = spikes[i] / (this->neurons_per_number * 9);
        int neuron_id = spikes[i] % (this->neurons_per_number * 9);
        std::pair<int, int> point(time, neuron_id);
        this->points_to_draw[cell_id].push_back(point);
    }
    float time_ms = time * this->timestep_ms;
    if (time_ms > this->latest_time) {
        this->latest_time = time_ms;
    }
    pthread_mutex_unlock(&this->point_mutex);
}

//-------------------------------------------------------------------------
//  Draws a string at the specified coordinates.
//-------------------------------------------------------------------------
void SudokuPlot::printgl(float x, float y, void *font_style,
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

void SudokuPlot::printglstroke(float x, float y, float size, float rotate,
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
    for (i = 0; str[i] != '\0'; i++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
    }
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
    glPopMatrix();
}

void check_cell(
        int cell_values[], bool cell_correct[], int x, int y,
        int row, int column) {
    int value = cell_values[(y * 9) + x];
    if (value == cell_values[(row * 9) + column]) {
        cell_correct[(y * 9) + x] = false;
    }
}

static void line(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void SudokuPlot::display(float time) {
    if (glutGetWindow() == this->window) {

        glPointSize(1.0);

        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glColor4f(0.0, 0.0, 0.0, 1.0);

        float cell_width = (window_width - (2 * WINDOW_BORDER)) / 9.0;
        float cell_height = (window_height - (2 * WINDOW_BORDER)) / 9.0;

        float end = this->latest_time;
        float start = end - this->ms_per_bin;
        if (start < 0.0) {
            start = 0.0;
            end = start + this->ms_per_bin;
        }

        float x_spacing = (float) cell_width /
            ((end - start) / this->timestep_ms);

        pthread_mutex_lock(&this->start_mutex);
        if (!this->database_read) {
            char prompt[] = "Waiting for database to be ready...";
            printgl((window_width / 2) - 120, window_height - 50,
                    GLUT_BITMAP_TIMES_ROMAN_24, prompt);
        } else if (!this->user_pressed_start) {
            char prompt[] = "Press space bar to start...";
            printgl((window_width / 2) - 100, window_height - 50,
                    GLUT_BITMAP_TIMES_ROMAN_24, prompt);
        } else if (!this->simulation_started) {
            char prompt[] = "Waiting for simulation to start...";
            printgl((window_width / 2) - 120, window_height - 50,
                    GLUT_BITMAP_TIMES_ROMAN_24, prompt);
        } else {
            char title[] = "Sudoku";
            printgl((window_width / 2) - 75, window_height - 50,
                    GLUT_BITMAP_TIMES_ROMAN_24, title);
        }
        pthread_mutex_unlock(&this->start_mutex);

        // Draw the cells
        glColor4f(0.0, 0.0, 0.0, 1.0);
        for (uint32_t i = 0; i <= 9; i++) {
            if (i % 3 == 0) {
                glLineWidth(3.0);
            } else {
                glLineWidth(1.0);
            }
            uint32_t y_pos = WINDOW_BORDER + (i * cell_height);
            line(window_width - WINDOW_BORDER, y_pos, WINDOW_BORDER, y_pos);
        }

        for (uint32_t i = 0; i <= 9; i++) {
            if (i % 3 == 0) {
                glLineWidth(3.0);
            } else {
                glLineWidth(1.0);
            }
            uint32_t x_pos = WINDOW_BORDER + (i * cell_width);
            line(x_pos, window_height - WINDOW_BORDER, x_pos, WINDOW_BORDER);
        }

        int start_tick = start / this->timestep_ms;
        int end_tick = end / this->timestep_ms;

        pthread_mutex_lock(&this->point_mutex);

        // Work out the cell values
        int cell_value[81];
        float cell_prob[81];
        for (uint32_t cell = 0; cell < 81; cell++) {

            // Strip off items that are no longer needed
            while (!points_to_draw[cell].empty() &&
                    points_to_draw[cell].front().first < start_tick) {
                points_to_draw[cell].pop_front();
            }

            // Count the spikes per number
            int count[9];
            for (uint32_t i = 0; i < 9; i++) {
                count[i] = 0;
            }
            int total = 0;
            for (auto iter = points_to_draw[cell].begin();
                    iter != points_to_draw[cell].end(); ++iter) {
                int number = iter->second / this->neurons_per_number;
                if (number < 9) {
                    count[number] += 1;
                    total += 1;
                } else {
                    fprintf(
                        stderr, "Neuron id %d out of range\n", iter->second);
                }
            }

            // Work out the probability of a given number in a given cell
            int max_prob_number = 0;
            float max_prob = 0;
            for (uint32_t i = 0; i < 9; i++) {
                if (count[i] > 0) {
                    float prob = (float) count[i] / (float) total;
                    if (prob > max_prob) {
                        max_prob = prob;
                        max_prob_number = i + 1;
                    }
                }
            }
            cell_value[cell] = max_prob_number;
            cell_prob[cell] = max_prob;
        }

        // Work out the correctness of each cell
        bool cell_valid[81];
        for (uint32_t cell = 0; cell < 81; cell++) {
            uint32_t x = cell % 9;
            uint32_t y = cell / 9;
            cell_valid[cell] = true;
            for (int row = 0; row < 9; row++) {
                if (row != y) {
                    check_cell(cell_value, cell_valid, x, y, row, x);
                }
            }
            for (int col = 0; col < 9; col++) {
                if (col != x) {
                    check_cell(cell_value, cell_valid, x, y, y, col);
                }
            }
            for (int row = 3 * (y / 3); row < (3 * ((y / 3) + 1)); row++) {
                for (int col = 3 * (x / 3); col < (3 * ((x / 3) + 1)); col++) {
                    if (x != col && y != row) {
                        check_cell(cell_value, cell_valid, x, y, row, col);
                    }
                }
            }
        }

        // Print the spikes
        for (uint32_t cell = 0; cell < 81; cell++) {
            uint32_t cell_x = cell % 9;
            uint32_t cell_y = cell / 9;

            uint32_t x_start = WINDOW_BORDER + (cell_x * cell_width) + 1;
            uint32_t y_start = WINDOW_BORDER + (cell_y * cell_height) + 1;
            float y_spacing = (float) cell_height
                    / (float) (this->neurons_per_number * 9);

            // Work out how probable the number is and use this for colouring
            float cell_sat = 1 - cell_prob[cell];

            glPointSize(2.0);
            glBegin(GL_POINTS);

            if (cell_valid[cell]) {
                glColor4f(cell_sat, 1.0, cell_sat, 1.0);
            } else {
                glColor4f(1.0, cell_sat, cell_sat, 1.0);
            }

            for (auto iter = points_to_draw[cell].begin();
                    iter != points_to_draw[cell].end(); ++iter) {
                float x_value =
                	(iter->first - start_tick) * x_spacing + x_start;
                float y_value = iter->second * y_spacing + y_start;
                glVertex2f(x_value, y_value);
            }

            glEnd();

            // Print the number
            if (cell_value[cell] != 0) {
                glColor4f(0, 0, 0, 1 - cell_sat);
                char label[] = "%d";
                float size = 0.005 * cell_height;
                printglstroke(
                    x_start + (cell_width / 2.0) - (size * 50.0),
                    y_start + (cell_height / 2.0) - (size * 50.0),
                    size, 0, label, cell_value[cell]);
            }
        }

        pthread_mutex_unlock(&this->point_mutex);

        glutSwapBuffers();
    }
}

void SudokuPlot::reshape(int width, int height) {
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

void SudokuPlot::keyboardUp(unsigned char key, int x, int y) {
    if ((int) key == 32) {

        // create and send the eieio command message confirming database
        // read
        pthread_mutex_lock(&this->start_mutex);
        if (!this->user_pressed_start) {
            printf("Starting the simulation\n");
            this->user_pressed_start = true;
            pthread_cond_signal(&this->start_condition);
        }
        pthread_mutex_unlock(&this->start_mutex);
    }
}

void SudokuPlot::safelyshut(void) {
    exit(0);                // kill program dead
}

SudokuPlot::~SudokuPlot() {
    // TODO Auto-generated destructor stub
}
