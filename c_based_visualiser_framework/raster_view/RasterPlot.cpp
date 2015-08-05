#include <GL/glut.h>
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "RasterPlot.h"
#include "PacketConverter.h"
#include "../utilities/SocketQueuer.h"
#include "../utilities/colour.h"
#include "../glut_framework/GlutFramework.h"

using namespace glutFramework;
using namespace std;

RasterPlot::RasterPlot(int argc, char **argv, char *remote_host,
                    std::set<int> *ports,
                    map<int, char*> *y_axis_labels,
                    map<int, int> *key_to_neuronid_map,
                    map<int, colour> *neuron_id_to_colour_map,
                    float plot_time_ms, float timestep_ms, int n_neurons,
                    DatabaseMessageConnection *database_message_connection) {
    this->window_width = INIT_WINDOW_WIDTH;
    this->window_height = INIT_WINDOW_HEIGHT;

    this->y_axis_labels = y_axis_labels;
    this->key_to_neuronid_map = key_to_neuronid_map;
    this->neuron_id_to_colour_map = neuron_id_to_colour_map;
    this->plot_time_ms = plot_time_ms;
    this->timestep_ms = timestep_ms;
    this->n_neurons = n_neurons;
    this->database_message_connection = database_message_connection;

    fprintf(stderr, "n_neurons = %i\n", this->n_neurons);
    fprintf(stderr, "plot time = %f\n", this->plot_time_ms);
    fprintf(stderr, "timestep = %f\n", this->timestep_ms);


    if (pthread_mutex_init(&(this->point_mutex), NULL) == -1) {
        fprintf(stderr, "Error initializing mutex!\n");
        exit(-1);
    }

    for (std::set<int>::iterator iter = ports->begin();
            iter != ports->end(); iter++) {
        int port = *iter;
        printf("listening to port %d", port);
        SocketQueuer *queuer = new SocketQueuer(port, remote_host);
        queuer->start();
        PacketConverter *translater = new PacketConverter(
            queuer, &(this->points_to_draw), &(this->point_mutex),
            this->key_to_neuronid_map);
        translater->start();
    }
    startFramework(argc, argv, "Raster Plot", window_width, window_height,
                INIT_WINDOW_X, INIT_WINDOW_Y, FRAMES_PER_SECOND);
}

void RasterPlot::init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glColor3f(1.0, 1.0, 1.0);
    glShadeModel(GL_SMOOTH);
}

//-------------------------------------------------------------------------
//  Draws a string at the specified coordinates.
//-------------------------------------------------------------------------
void RasterPlot::printgl(float x, float y, void *font_style,
        char* format, ...) {
    va_list arg_list;
    char str[256];
    int i;

    // font options:  GLUT_BITMAP_8_BY_13 GLUT_BITMAP_9_BY_15
    //GLUT_BITMAP_TIMES_ROMAN_10 GLUT_BITMAP_HELVETICA_10
    //GLUT_BITMAP_HELVETICA_12 GLUT_BITMAP_HELVETICA_18
    //GLUT_BITMAP_TIMES_ROMAN_24

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
    for (i = 0; str[i] != '\0'; i++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
    }
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
    glPopMatrix();
}

void RasterPlot::display(float time) {
    if (glutGetWindow() == this->window) {
        glPointSize(1.0);
        float x_spacing = (float) (window_width - (2 * WINDOW_BORDER))
                / ((float) plot_time_ms / timestep_ms);
        float y_spacing = (float) (window_height - (2 * WINDOW_BORDER))
                / (float) n_neurons;

        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glColor4f(0.0, 0.0, 0.0, 1.0);

        char title[] = "Raster Plot";
        printgl((window_width / 2) - 75, window_height - 50,
                GLUT_BITMAP_TIMES_ROMAN_24, title);

        char x_axis[] = "Simulation Time (ms)";
        printglstroke((window_width / 2) - 100, 20, 0.12, 0, x_axis);
        char label_0[] = "0";
        printglstroke(WINDOW_BORDER - 15, WINDOW_BORDER - 20, 0.10, 0, label_0);
        char label_max[] = "%.1f";
        printglstroke(window_width - WINDOW_BORDER - 20, WINDOW_BORDER - 20,
                0.10, 0, label_max, plot_time_ms);

        for (map<int, char*>::iterator iter = y_axis_labels->begin();
                iter != y_axis_labels->end(); ++iter) {
            float y_value = ((iter->first * y_spacing) + WINDOW_BORDER) - 10;
            float width =
                    glutStrokeLength(GLUT_STROKE_ROMAN,
                                    reinterpret_cast<const unsigned char*>
                                     (iter->second))
                    * 0.1;
            char y_label[] = "%s";
            printglstroke((WINDOW_BORDER - width) - 20, y_value, 0.10, 0,
                    iter->second);
        }

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
        pthread_mutex_lock(&(this->point_mutex));
        for (deque<pair<int, int> >::iterator iter =
                points_to_draw.begin(); iter != points_to_draw.end(); ++iter) {

            if (neuron_id_to_colour_map->find(iter->second)
                    == neuron_id_to_colour_map->end()) {
                fprintf(stderr, "Missing colour for neuron %d\n", iter->second);
                continue;
            }
            struct colour colour = (*neuron_id_to_colour_map)[iter->second];

            glColor4f(colour.r, colour.g, colour.b, 1.0);
            float x_value = (iter->first * x_spacing) + WINDOW_BORDER;
            float y_value = (iter->second * y_spacing) + WINDOW_BORDER;

            glVertex2f(x_value, y_value);
        }
        pthread_mutex_unlock(&(this->point_mutex));
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
    if ((int) key == 13) {
        if (database_message_connection != NULL) {

            // create and send the eieio command message confirming database
            // read
            printf("Starting the simulation\n");
            database_message_connection->send_ready_notification();
            database_message_connection->close_connection();
        }
    }
}

void RasterPlot::safelyshut(void) {
    exit(0);                // kill program dead
}

RasterPlot::~RasterPlot() {
    // TODO Auto-generated destructor stub
}
