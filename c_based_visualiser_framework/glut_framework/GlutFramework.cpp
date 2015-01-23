/*
 * The MIT License
 *
 * Copyright (c) 2010 Paul Solt, PaulSolt@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "GlutFramework.h"

namespace glutFramework {

	GlutFramework *GlutFramework::instance = NULL;

	GlutFramework::GlutFramework() {
		elapsedTimeInSeconds = 0;
		frameTimeElapsed = 0;
	}

	GlutFramework::~GlutFramework() {
	}

	void GlutFramework::startFramework(
	        int argc, char *argv[], std::string title, int width, int height,
	        int posx, int posy, double fps) {

	    // Sets the instance to this, used in the callback wrapper functions
		setInstance();
		this->frameTime = 1.0 / fps * 1000.0;

		// Initialize GLUT
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
		glutInitWindowSize(width, height);
		glutInitWindowPosition(0, 100);
		this->window = glutCreateWindow(title.c_str());

        init();                     // Initialize

		// Function callbacks with wrapper functions
        glutDisplayFunc(displayWrapper);
		glutReshapeFunc(reshapeWrapper);
        glutIdleFunc(runWrapper);
		glutMouseFunc(mouseButtonPressWrapper);
		glutMotionFunc(mouseMoveWrapper);
		glutKeyboardFunc(keyboardDownWrapper);
		glutKeyboardUpFunc(keyboardUpWrapper);
		glutSpecialFunc(specialKeyboardDownWrapper);
		glutSpecialUpFunc(specialKeyboardUpWrapper);
		glutMainLoop();
	}

	void GlutFramework::display(float dTime) {
	}

	void GlutFramework::reshape(int width, int height) {
		glViewport(0,0,(GLsizei)width,(GLsizei)height);
	}

	void GlutFramework::mouseButtonPress(int button, int state, int x, int y) {
	}

	void GlutFramework::mouseMove(int x, int y) {
	}

	void GlutFramework::keyboardDown( unsigned char key, int x, int y ) {
	}

	void GlutFramework::keyboardUp( unsigned char key, int x, int y ) {
	}

	void GlutFramework::specialKeyboardDown( int key, int x, int y ) {
	}

	void GlutFramework::specialKeyboardUp( int key, int x, int y ) {
	}

	// **************************
	// ** GLUT Setup functions **
	// **************************
	void GlutFramework::init() {
	}

	void GlutFramework::setInstance() {
		instance = this;
	}

	void GlutFramework::run() {
		if(frameRateTimer.isStopped()) {
			frameRateTimer.start();
		}

		// stop the timer and calculate time since last frame
		frameRateTimer.stop();
		double milliseconds = frameRateTimer.getElapsedMilliseconds();
		frameTimeElapsed += milliseconds;

		if( frameTimeElapsed >= this->frameTime ) {

		    // If the time exceeds a certain "frame rate" then show the next
		    // frame
			glutPostRedisplay();

			// remove a "frame" and start counting up again
			frameTimeElapsed -= this->frameTime;
		}
		frameRateTimer.start();
	}

	void GlutFramework::displayFramework() {
	    if (displayTimer.isStopped()) {
            displayTimer.start();
        }
        displayTimer.stop();
        elapsedTimeInSeconds = displayTimer.getElapsedSeconds();
        display(elapsedTimeInSeconds);
        displayTimer.start();
	}

	// ******************************************************************
	// ** Static functions which are passed to Glut function callbacks **
	// ******************************************************************

	void GlutFramework::displayWrapper() {
	    instance->displayFramework();
	}

	void GlutFramework::reshapeWrapper(int width, int height) {
		instance->reshape(width, height);
	}

	void GlutFramework::runWrapper() {
		instance->run();
	}

	void GlutFramework::mouseButtonPressWrapper(int button, int state, int x,
	                                            int y) {
		instance->mouseButtonPress(button, state, x, y);
	}

	void GlutFramework::mouseMoveWrapper(int x, int y) {
		instance->mouseMove(x, y);
	}

	void GlutFramework::keyboardDownWrapper(unsigned char key, int x, int y) {
		instance->keyboardDown(key,x,y);
	}

	void GlutFramework::keyboardUpWrapper(unsigned char key, int x, int y) {
		instance->keyboardUp(key,x,y);
	}

	void GlutFramework::specialKeyboardDownWrapper(int key, int x, int y) {
		instance->specialKeyboardDown(key,x,y);
	}

	void GlutFramework::specialKeyboardUpWrapper(int key, int x, int y) {
		instance->specialKeyboardUp(key,x,y);
	}

} // namespace
