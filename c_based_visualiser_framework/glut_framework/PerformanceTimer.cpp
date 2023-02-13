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

#include "PerformanceTimer.h"
#include <cstddef>

PerformanceTimer::PerformanceTimer() {

#ifdef WIN32
    // Retrieves the frequency of the high-resolution performance counter
    QueryPerformanceFrequency(&_freq);
    _start.QuadPart = 0;
    _end.QuadPart = 0;
#else
    _start.tv_sec = 0;
    _start.tv_usec = 0;
    _end.tv_sec = 0;
    _end.tv_usec = 0;

#endif

    _isStopped = true;
}

PerformanceTimer::~PerformanceTimer() {
}

void PerformanceTimer::start() {
#ifdef WIN32
     // Retrieves the current value of the high-resolution performance counter
     QueryPerformanceCounter(&_start);
#else
    // Get the starting time
    gettimeofday(&_start, NULL);
#endif
    _isStopped = false;
}

void PerformanceTimer::stop() {
#ifdef WIN32
    QueryPerformanceCounter(&_end);
#else
    gettimeofday(&_end, NULL);
#endif

    _isStopped = true;
}

bool PerformanceTimer::isStopped() const {
    return _isStopped;
}

double PerformanceTimer::getElapsedMicroseconds() {
    double microSecond = 0;

    if (!_isStopped) {
#ifdef WIN32
        QueryPerformanceCounter(&_end);
#else
        gettimeofday(&_end, NULL);
#endif
    }

#ifdef WIN32
    if (_start.QuadPart != 0 && _end.QuadPart != 0) {
        microSecond = (_end.QuadPart - _start.QuadPart)
                * (1000000.0 / _freq.QuadPart);
    }
#else
    microSecond = (_end.tv_sec * 1000000.0 + _end.tv_usec)
            - (_start.tv_sec * 1000000.0 + _start.tv_usec);
#endif

    return microSecond;
}

double PerformanceTimer::getElapsedMilliseconds() {
    return getElapsedMicroseconds() / 1000.0;
}

double PerformanceTimer::getElapsedSeconds() {
    return getElapsedMicroseconds() / 1000000.0;
}
