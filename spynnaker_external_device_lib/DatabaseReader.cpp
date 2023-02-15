/*
 * Copyright (c) 2015 The University of Manchester
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
#include "DatabaseReader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdarg>

using namespace std;

static inline char *get_column_string_copy(sqlite3_stmt *compiled_statement,
                                           int column) {
    char *label = (char*) sqlite3_column_text(compiled_statement, column);
    char *labelcopy = (char *) malloc(strlen(label) + 1);
    strcpy(labelcopy, label);
    return labelcopy;
}

static void __attribute__((noreturn)) fail(const char *fmt, ...) {
    va_list arg_list;

    va_start(arg_list, fmt);
    vfprintf(stderr, fmt, arg_list);
    va_end(arg_list);
    exit(1);
}

DatabaseReader::DatabaseReader(char *database_path) {
    int rc;
    rc = sqlite3_open(database_path, &this->db);
    if (rc) {
        fail("Can't open database: %s\n", sqlite3_errmsg(this->db));
    }
}

DatabaseReader::~DatabaseReader() {
    this->close_database_connection();
}

sqlite3 *DatabaseReader::get_cursor() {
    return this->db;
}

sqlite3_stmt *DatabaseReader::db_prepare(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    char *sql = sqlite3_vmprintf(fmt, args);
    va_end(args);
    sqlite3_stmt *compiled_statment;
    if (sqlite3_prepare_v2(
            this->db, sql, -1, &compiled_statment, NULL) != SQLITE_OK) {
        fail("Error reading database: %i: %s\n",
            sqlite3_errcode(this->db), sqlite3_errmsg(this->db));
    }
    sqlite3_free(sql);
    return compiled_statment;
}

std::map<int, int> *DatabaseReader::get_key_to_neuron_id_mapping(char* label) {
    sqlite3_stmt *compiled_statment = db_prepare(
        "SELECT * FROM label_event_atom_view WHERE label=\"%q\"", label);
    std::map<int, int> *key_to_neuron_id_map = new std::map<int, int>();
    while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
        int neuron_id = sqlite3_column_int(compiled_statment, 0);
        int key = sqlite3_column_int(compiled_statment, 1);
        (*key_to_neuron_id_map)[key] = neuron_id;
    }
    return key_to_neuron_id_map;
}

std::map<int, int> *DatabaseReader::get_neuron_id_to_key_mapping(char* label) {
    sqlite3_stmt *compiled_statment = db_prepare(
        "SELECT * FROM label_event_atom_view WHERE label=\"%q\"", label);

    std::map<int, int> *neuron_id_to_key_map = new std::map<int, int>();
    while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
        int neuron_id = sqlite3_column_int(compiled_statment, 0);
        int key = sqlite3_column_int(compiled_statment, 1);
        (*neuron_id_to_key_map)[neuron_id] = key;
    }
    return neuron_id_to_key_map;
}

ip_tag_info *DatabaseReader::get_live_output_details(char *label) {
    sqlite3_stmt *compiled_statment = db_prepare(
        "SELECT * FROM app_output_tag_view WHERE pre_vertex_label=\"%q\""
        " AND post_vertex_label LIKE \"LiveSpikeReceiver%%\"",
        label);

    if (sqlite3_step(compiled_statment) != SQLITE_ROW) {
        fail("No IP tag found for population %s\n", label);
    }

    ip_tag_info *tag_info = (ip_tag_info *) malloc(sizeof(ip_tag_info));
    tag_info->ip_address = get_column_string_copy(compiled_statment, 0);
    tag_info->port = sqlite3_column_int(compiled_statment, 1);
    tag_info->strip_sdp = sqlite3_column_int(compiled_statment, 2) == 1;
    tag_info->board_address = get_column_string_copy(compiled_statment, 3);
    tag_info->tag = sqlite3_column_int(compiled_statment, 4);
    return tag_info;
}

float DatabaseReader::get_configuration_parameter_value(char *parameter_name) {
    sqlite3_stmt *compiled_statment = db_prepare(
        "SELECT value FROM configuration_parameters"
        " WHERE parameter_id = \"%q\"", parameter_name);

    if (sqlite3_step(compiled_statment) != SQLITE_ROW) {
        fail("Parameter %s not found\n", parameter_name);
    }

    return sqlite3_column_double(compiled_statment, 0);
}

std::vector<placement *> *DatabaseReader::get_placements(char *label) {
    sqlite3_stmt *compiled_statment = db_prepare(
        "SELECT x, y, p FROM application_vertex_placements WHERE vertex_label=\"%q\"",
        label);

    std::vector<placement *> *placements = new std::vector<placement *>();
    while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
        placement *plmnt = (placement *) malloc(sizeof(placement));
        plmnt->x = sqlite3_column_int(compiled_statment, 0);
        plmnt->y = sqlite3_column_int(compiled_statment, 1);
        plmnt->p = sqlite3_column_int(compiled_statment, 2);
        placements->push_back(plmnt);
    }

    if (placements->size() == 0) {
        fail("No placements found for %s", label);
    }
    return placements;
}

char *DatabaseReader::get_ip_address(int x, int y) {
    sqlite3_stmt *compiled_statment = db_prepare(
        "SELECT eth_ip_address FROM chip_eth_info WHERE x=%d AND y=%d OR"
        " x=0 AND y=0 ORDER BY x DESC", x, y);

    if (sqlite3_step(compiled_statment) != SQLITE_ROW) {
        fail("IP address for %d, %d not found\n", x, y);
    }
    return get_column_string_copy(compiled_statment, 0);
}

void DatabaseReader::close_database_connection() {
    sqlite3_close(this->db);
}
