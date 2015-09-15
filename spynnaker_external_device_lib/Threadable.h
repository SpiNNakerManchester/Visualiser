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
