/*
 * Copyright (c) 2015 The University of Manchester
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
