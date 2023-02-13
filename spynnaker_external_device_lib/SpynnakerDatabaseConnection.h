/*
 * Copyright (c) 2015-2023 The University of Manchester
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

class PauseStopCallbackInterface {
public:
    virtual void pause_stop_callback() = 0;
    virtual ~PauseStopCallbackInterface() {};
};


class SpynnakerDatabaseConnection : public UDPConnection, private Threadable {
public:
    SpynnakerDatabaseConnection(
        StartCallbackInterface *start_callback=NULL,
        PauseStopCallbackInterface *pause_stop_callback=NULL,
        char *local_host=NULL, int local_port=0);
    void add_database_callback(DatabaseCallbackInterface *callback);
    void set_database(char *database_path);
    ~SpynnakerDatabaseConnection();

protected:
    void run();

private:
    std::vector<DatabaseCallbackInterface *> database_callbacks;
    StartCallbackInterface *start_callback;
    PauseStopCallbackInterface *pause_stop_callback;
    bool database_path_received;
    bool running;
};

#endif /* _SPYNNAKER_DATABASE_CONNECTION_H_ */
