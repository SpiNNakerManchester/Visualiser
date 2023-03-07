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

#ifndef PERFORMANCE_TIMER_H
#define PERFORMANCE_TIMER_H

#ifdef WIN32
    #include <windows.h>	// Windows
#else
    #include <sys/time.h>	// Mac/Unix
#endif

/** Performance_Timer.h
 * A cross-platform timer wrapped around a high performance Windows/Unix time function.
 *
 * Authors: Paul Solt 8-21-10
 */

class PerformanceTimer {
public:
    PerformanceTimer();
    ~PerformanceTimer();

    /** Starts the timer */
    void start();

    /** Stops the timer */
    void stop();

    /** Checks to see if the timer is stopped */
    bool isStopped() const;

    /** Returns the elapsed time since the timer was started, or the time interval
     between calls to start() and stop().
     */
    double getElapsedMicroseconds();
    double getElapsedMilliseconds();
    double getElapsedSeconds();

private:
    bool _isStopped;

#ifdef WIN32
    LARGE_INTEGER _start;
    LARGE_INTEGER _end;
    LARGE_INTEGER _freq;
#else
    timeval _start;
    timeval _end;
#endif

};

#endif
