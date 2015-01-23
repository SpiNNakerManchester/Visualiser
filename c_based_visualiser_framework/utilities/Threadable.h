/*
 * Threadable.h
 *
 *  Created on: 3 Dec 2014
 *      Author: stokesa6
 */

#include <pthread.h>

#ifndef C_VIS_DATABASE_THREADABLE_H_
#define C_VIS_DATABASE_THREADABLE_H_

/*
 * socketqueuer.h
 *
 *  Created on: 3 Dec 2014
 *      Author: stokesa6
 */
class Threadable {
public:
	Threadable();
	virtual ~Threadable();
	bool start(){
		return (pthread_create(&_thread, NULL, InternalThreadEntryFunc, this) == 0);
    }
	void exit_thread(){
		(void) pthread_join(_thread, NULL);
	}
protected:
   /** Implement this method in your subclass with the code you want your thread to run. */
   virtual void InternalThreadEntry() = 0;
private:
   static void * InternalThreadEntryFunc(void * This) {((Threadable *)This)->InternalThreadEntry(); return NULL;}
   pthread_t _thread;
};

#endif /* C_VIS_DATABASE_THREADABLE_H_ */
