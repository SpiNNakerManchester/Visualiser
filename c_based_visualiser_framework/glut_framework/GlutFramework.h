/*
 * Copyright (c) 2015 The University of Manchester
 * based on work Copyright (c) 2010 Paul Solt, paulsolt@gmail.com
 * https://github.com/PaulSolt/GLUT-Object-Oriented-Framework

 * Originally released under the MIT License
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

#ifndef GLUT_FRAMEWORK_H
#define GLUT_FRAMEWORK_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <string.h>

#ifdef WIN32
    #include <windows.h>
    #include <GL/glut.h>
#elif defined(__APPLE__)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
    #include <GL/freeglut.h>
#endif


#include "Keyboard.h"
#include "PerformanceTimer.h"
#include "Vector.h"


class GlutFramework {

private:
    double frameTimeElapsed;
    double frameTime;

protected:
    PerformanceTimer frameRateTimer;
    PerformanceTimer displayTimer;

    double elapsedTimeInSeconds;
    static GlutFramework *instance;
    int window;

public:
    GlutFramework();
    virtual ~GlutFramework();

    /**
     * startFramework will initialize framework and start the Glut run
     * loop. It must be called after the GlutFramework class is created to
     * start the application.
     */
    void startFramework(int argc, char *argv[], std::string title,
            int width, int height, int posx, int posy, double fps);

    // ****************************
    // ** Subclass and Implement **
    // ****************************

    /** Initializes GLUT and registers the callback functions. */
    virtual void init();

    /**
     The display function is called at a specified frames-per-second
     (FPS). Any animation drawing code can be run in the display method.
     @param dTime - the change in time (seconds)
     */
    virtual void display(float dTime);

    /** Called when the window dimensions change.
     @param width - the width of the window in pixels
     @param height - the height of the window in pixels
     */
    virtual void reshape(int width, int height);

    /** Called when the mouse buttons are pressed.
     @param button - the mouse buttons
     @param state - the state of the buttons
     @param x - the x coordinate
     @param y - the y coordinate
     */
    virtual void mouseButtonPress(int button, int state, int x, int y);

    /** Called when the mouse moves on the screen.
     @param x - the x coordinate
     @param y - the y coordinate
     */
    virtual void mouseMove(int x, int y);

    /**
     The keyboard function is called when a standard key is pressed
     down.
     @param key - the key press
     @param x - the x coordinate of the mouse
     @param y - the y coordinate of the mouse
     */
    virtual void keyboardDown( unsigned char key, int x, int y );

    /** The keyboard function is called when a standard key is "unpressed".
     @param key - the key press
     @param x - the x coordinate of the mouse
     @param y - the y coordinate of the mouse
     */
    virtual void keyboardUp( unsigned char key, int x, int y );

    /** The keyboard function is called when a special key is pressed down.
     (F1 keys, Home, Inser, Delete, Page Up/Down, End, arrow keys)
     http://www.opengl.org/resources/libraries/glut/spec3/node54.html
     @param key - the key press
     @param x - the x coordinate of the mouse
     @param y - the y coordinate of the mouse
     */
    virtual void specialKeyboardDown( int key, int x, int y );

    /** The keyboard function is called when a special key is "unpressed.
     (F1 keys, Home, Inser, Delete, Page Up/Down, End, arrow keys)
     @param key - the key press
     @param x - the x coordinate of the mouse
     @param y - the y coordinate of the mouse
     */
    virtual void specialKeyboardUp( int key, int x, int y );

    /**
     * The instance will be set when the framework is started, this step
     * is necessary to initialize the static instance that is used for the
     * object oriented wrapper calls
     */
    void setInstance();

    /**
     * The run method is called by GLUT and contains the logic to set the
     * framerate of the application.
     */
    void run();

    /** The displayFramework() function sets up initial GLUT state and
     *  calculates the change in time between each frame. It calls the
     *   display(float) function which can be subclassed.
     */
    void displayFramework();

    // ** Static functions which are passed to GLUT function callbacks **
    static void displayWrapper();
    static void reshapeWrapper(int width, int height);
    static void runWrapper();
    static void mouseButtonPressWrapper(int button, int state, int x,
                                        int y);
    static void mouseMoveWrapper(int x, int y);
    static void keyboardDownWrapper(unsigned char key, int x, int y);
    static void keyboardUpWrapper(unsigned char key, int x, int y);
    static void specialKeyboardDownWrapper(int key, int x, int y);
    static void specialKeyboardUpWrapper(int key, int x, int y);
};
#endif
