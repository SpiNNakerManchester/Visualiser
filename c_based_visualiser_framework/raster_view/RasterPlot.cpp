/*
 * rasterplot.cpp
 *
 *  Created on: 2 Dec 2014
 *      Author: alan and Rowley
 */

#include <GL/glut.h>
#include <GL/freeglut.h>
#include <map>
#include <deque>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "RasterPlot.h"
#include "PacketConverter.h"

using namespace std;
RasterPlot *instance = NULL;

RasterPlot::RasterPlot(int argc, char **argv) {
	if (pthread_mutex_init(&this->point_mutex, NULL) == -1) {
        fprintf(stderr, "Error initializing mutex!\n");
        exit(-1);
    }
	instance = this;
}

void RasterPlot::start(SocketQueuer *queuer,
                       std::map<int, char*> *y_axis_labels,
                       std::map<int, int> *key_to_neuron_id_map) {
    this->y_axis_labels = y_axis_labels;

    PacketConverter translater(queuer, &this->points_to_draw,
                               &this->point_mutex,
                               key_to_neuron_id_map);
    translater.start();
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

    glPointSize(1.0);
    float x_spacing = (float) (windowWidth - (2 * windowBorder))
            / ((float) plotTime / timeStep);
    float y_spacing = (float) (windowHeight - (2 * windowBorder))
            / (float) plotNeurons;

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear (GL_COLOR_BUFFER_BIT);
    glColor4f(0.0, 0.0, 0.0, 1.0);                      // Black Text for Labels

    char title[] = "Raster Plot";
    printgl((windowWidth / 2) - 75, windowHeight - 50,
            GLUT_BITMAP_TIMES_ROMAN_24, title);

    char x_axis[] = "Simulation Time (ms)";
    printglstroke((windowWidth / 2) - 100, 20, 0.12, 0, x_axis);
    char label_0[] = "0";
    printglstroke(windowBorder - 15, windowBorder - 20, 0.10, 0, label_0);
    char label_max[] = "%d";
    printglstroke(windowWidth - windowBorder - 20, windowBorder - 20, 0.10, 0,
            label_max, plotTime);

    if (y_axis_labels != NULL) {
        for (map<int, char*>::iterator iter = y_axis_labels->begin();
                iter != y_axis_labels->end(); ++iter) {
            float y_value = ((iter->first * y_spacing) + windowBorder) - 10;
            char y_label[] = "%s";
            printglstroke(60, y_value, 0.10, 0, y_label, iter->second);
        }
    }

    glColor4f(0.0, 0.0, 0.0, 1.0);
    glLineWidth(1.0);
    glBegin (GL_LINES);
    glVertex2f(windowWidth - windowBorder, windowBorder); // rhs
    glVertex2f(windowBorder - 10, windowBorder); // inside
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(windowBorder - 10, windowBorder);
    glVertex2f(windowBorder - 10, windowHeight - windowBorder);
    glEnd();

    glPointSize(2.0);
    glBegin(GL_POINTS);
    for (deque<pair<int, int> >::iterator iter =
            points_to_draw.begin(); iter != points_to_draw.end(); ++iter) {
        struct colour colour = neuron_id_to_colour_map[iter->second];
        if (neuron_id_to_colour_map.find(iter->second)
                == neuron_id_to_colour_map.end()) {
            fprintf(stderr, "Missing colour for neuron %d\n", iter->second);
            continue;
        }

        glColor4f(colour.r, colour.g, colour.b, 1.0);
        float x_value = (iter->first * x_spacing) + windowBorder;
        float y_value = (iter->second * y_spacing) + windowBorder;

        glVertex2f(x_value, y_value);
    }
    do_update = false;
    glEnd();

    glutSwapBuffers();
}

void RasterPlot::reshape(int width, int height) {
    fprintf(stderr, "Reshape to %d, %d\n", width, height);
    windowWidth = width;
    windowHeight = height;

    //printf("Wid: %d, Hei: %d.\n",width,height);
    glViewport(0, 0, (GLsizei) width, (GLsizei) height); // viewport dimensions
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // an orthographic projection. Should probably look into OpenGL
    //perspective projections for 3D if that's your thing
    glOrtho(0.0, width, 0.0, height, -50.0, 50.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void RasterPlot::safelyshut(void) {
    exit(0);                // kill program dead
}

RasterPlot::~RasterPlot() {
	// TODO Auto-generated destructor stub
}
