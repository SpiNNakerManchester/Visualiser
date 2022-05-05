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
#include <map>
#include <string>
#include <vector>
#include <sqlite3.h>

#ifndef _DATABASE_READER_H_
#define _DATABASE_READER_H_

typedef struct ip_tag_info {
    char *ip_address;
    int port;
    bool strip_sdp;
    char *board_address;
    int tag;
} ip_tag_info;

typedef struct reverse_ip_tag_info {
    char *board_address;
    int port;
} reverse_ip_tag_info;

typedef struct placement {
    int x;
    int y;
    int p;
} placement;

class DatabaseReader {
    sqlite3 *db;
public:
    DatabaseReader(char *database_path);
    virtual ~DatabaseReader();
    sqlite3 *get_cursor();
    std::map<int, int> *get_key_to_neuron_id_mapping(char* label);
    std::map<int, int> *get_neuron_id_to_key_mapping(char* label);
    ip_tag_info *get_live_output_details(char *label);
    std::vector<placement *> *get_placements(char *label);
    char *get_ip_address(int x, int y);
    float get_configuration_parameter_value(char *parameter_name);
    void close_database_connection();
private:
    void db_error();
    sqlite3_stmt *db_prepare(const char *sql, ...);
};

#endif /* _DATABASE_READER_H_ */
