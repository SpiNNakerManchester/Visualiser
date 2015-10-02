#include "UDPConnection.h"
#include "DatabaseReader.h"
#include "Threadable.h"
#include <vector>

#ifndef _SPYNNAKER_DATABASE_CONNECTION_H_
#define _SPYNNAKER_DATABASE_CONNECTION_H_

class DatabaseCallbackInterface {
public:
    virtual void read_database_callback(DatabaseReader *reader) = 0;
    virtual ~DatabaseCallbackInterface() {};
};

class StartCallbackInterface {
public:
    virtual void start_callback() = 0;
    virtual ~StartCallbackInterface() {};
};

class SpynnakerDatabaseConnection : public UDPConnection, private Threadable {
public:
    SpynnakerDatabaseConnection(
        StartCallbackInterface *start_callback=NULL,
        char *local_host=NULL, int local_port=0);
    void add_database_callback(DatabaseCallbackInterface *callback);
    void set_database(char *database_path);
    ~SpynnakerDatabaseConnection();

protected:
    void run();

private:
    std::vector<DatabaseCallbackInterface *> database_callbacks;
    StartCallbackInterface *start_callback;
    bool database_path_received;
};

#endif /* _SPYNNAKER_DATABASE_CONNECTION_H_ */
