/*
 * Copyright (c) 2018-2023 The University of Manchester
 * based on work Copyright (c) 2010 Paul Solt, paulsolt@gmail.com
 * Originally released under the MIt License
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

#include "Keyboard.h"

Keyboard::Keyboard() {
    for (int i = 0; i < NUMBER_KEYS; i++) {
        keys[i] = UP;
    }
}

void Keyboard::keyDown( int key ) {
    keys[key] = DOWN;
}

void Keyboard::keyUp( int key ) {
    keys[key] = UP;
}

bool Keyboard::isKeyDown( int key ) {
    if (keys[key] == DOWN) {
        return true;
    } else {
        return false;
    }
}
