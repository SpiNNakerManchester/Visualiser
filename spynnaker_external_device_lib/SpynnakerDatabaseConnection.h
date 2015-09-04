#include "UDPConnection.h"
#include "DatabaseReader.h"
#include "Threadable.h"

#ifndef _SPYNNAKER_DATABASE_CONNECTION_H_
#define _SPYNNAKER_DATABASE_CONNECTION_H_

class DatabaseCallbackInterface {
public:
    virtual void read_database_callback(DatabaseReader *reader) = 0;
};

class StartCallbackInterface {
public:
    virtual void start_callback() = 0;
};

class SpynnakerDatabaseConnection : public UDPConnection, private Threadable {
public:
    SpynnakerDatabaseConnection(
        DatabaseCallbackInterface *database_callback,
        StartCallbackInterface *start_callback=NULL,
        char *local_host=NULL, int local_port=0);
    ~SpynnakerDatabaseConnection();

protected:
    void run();

private:
    DatabaseCallbackInterface *database_callback;
    StartCallbackInterface *start_callback;
};

#endif /* _SPYNNAKER_DATABASE_CONNECTION_H_ */
