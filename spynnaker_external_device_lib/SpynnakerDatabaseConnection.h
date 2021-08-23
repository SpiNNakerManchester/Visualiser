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
