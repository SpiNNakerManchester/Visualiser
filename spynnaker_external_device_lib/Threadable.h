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
#include <pthread.h>

#ifndef _THREADABLE_H_
#define _THREADABLE_H_


class Threadable {

public:

    Threadable() {}

    virtual ~Threadable() {}

    bool start(){
        return (pthread_create(&_thread, NULL, _run, this) == 0);
    }

    void join() {
        (void) pthread_join(_thread, NULL);
    }

protected:

    //Implement this method in your subclass with the code you want your
    // thread to run
    virtual void run() = 0;

private:

    static void *_run(void * This) {
        ((Threadable *)This)->run();
        return NULL;
    }

    pthread_t _thread;
};

#endif /* _THREADABLE_H_ */
