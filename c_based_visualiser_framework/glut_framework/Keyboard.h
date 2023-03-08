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
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

/** Keyboard.h
 *
 * Description: A basic utility class to keep track of what keys are currently
 *	kbeing pressed.
 *
 * Author: Paul Solt 8-21-10
 */


class Keyboard {
private:
    enum Key { UP, DOWN, RELEASED };
    static const int NUMBER_KEYS = 256;
    Key keys[ NUMBER_KEYS ];

public:

    /** Name: Keyboard()
     *
     * Description: Initialize all keys in the up state
     */
    Keyboard();

    /** Name: keyDown()
     *
     * Description: Set the key to the down state
     * Param: key - the key that is being pressed
     */
    void keyDown( int key );

    /** Name: keyDown()
     *
     * Description: Set the key to the up state
     * Param: key - the key that is being released
     */
    void keyUp( int key );

    /** Name: isKeyDown
     *
     * Description: Test to see if the key is being pressed
     */
    bool isKeyDown( int key );
};

#endif
