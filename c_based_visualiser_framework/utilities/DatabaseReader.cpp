#include "DatabaseReader.h"
#include "colour.h"

#include <map>
#include <sqlite3.h>
#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include <string.h>

using namespace std;

static inline char *get_column_string_copy(sqlite3_stmt *compiled_statement,
                                           int column) {
    char *label = (char*) sqlite3_column_text(compiled_statement, column);
    char *labelcopy = (char *) malloc(strlen(label) + 1);
    strcpy(labelcopy, label);
    return labelcopy;
}


DatabaseReader::DatabaseReader(char *database_path) {
    int rc;
    fprintf(stderr, "Reading database %s\n", database_path);
    rc = sqlite3_open(database_path, &this->db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(this->db));
    } else{
        fprintf(stderr, "Opened database successfully\n");
    }
}

DatabaseReader::~DatabaseReader() {
    this->close_database_connection();
}

sqlite3 *DatabaseReader::get_cursor() {
    return this->db;
}

std::vector<char *> *DatabaseReader::get_live_population_labels() {
    char *sql = sqlite3_mprintf(
        "SELECT pre_vertices.vertex_label"
        " FROM Partitionable_vertices as pre_vertices"
        " JOIN Partitionable_edges as edges"
        " ON pre_vertices.partitionable_vertex_id == edges.pre_vertex"
        " JOIN Partitionable_vertices as post_vertices"
        " ON edges.post_vertex = post_vertices.vertex_id"
        " WHERE post_vertices.vertex_label == \"Monitor\"");
    sqlite3_stmt *compiled_statment;
    if (sqlite3_prepare_v2(this->db, sql, -1,
                           &compiled_statment, NULL) == SQLITE_OK){
        std::vector<char *> *labels = new std::vector<char *>();
        while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
            char *label = get_column_string_copy(compiled_statment, 0);
            labels->push_back(label);
        }
        free(sql);
        return labels;
    } else {
        fprintf(stderr, "Error reading database: %i: %s\n",
                sqlite3_errcode(this->db),
                sqlite3_errmsg(this->db));
        exit(-1);
    }
}

std::map<int, int> *DatabaseReader::get_key_to_neuron_id_mapping(char* label) {
    char *sql = sqlite3_mprintf(
        "SELECT n.neuron_id as n_id, n.key as key"
        " FROM key_to_neuron_mapping as n"
        " JOIN Partitionable_vertices as p ON n.vertex_id = p.vertex_id"
        " WHERE p.vertex_label=\"%q\"", label);
    sqlite3_stmt *compiled_statment;
    if (sqlite3_prepare_v2(this->db, sql, -1,
                           &compiled_statment, NULL) == SQLITE_OK){
        std::map<int, int> *key_to_neuron_id_map = new std::map<int, int>();
        while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
            int neuron_id = sqlite3_column_int(compiled_statment, 0);
            int key = sqlite3_column_int(compiled_statment, 1);
            (*key_to_neuron_id_map)[key] = neuron_id;
        }
        free(sql);
        return key_to_neuron_id_map;
    } else {
        fprintf(stderr, "Error reading database: %i: %s\n",
                sqlite3_errcode(this->db),
                sqlite3_errmsg(this->db));
        exit(-1);
    }
}

std::map<int, int> *DatabaseReader::get_neuron_id_to_key_mapping(char* label) {
    char *sql = sqlite3_mprintf(
        "SELECT n.neuron_id as n_id, n.key as key"
        " FROM key_to_neuron_mapping as n"
        " JOIN Partitionable_vertices as p ON n.vertex_id = p.vertex_id"
        " WHERE p.vertex_label=\"%q\"", label);
    sqlite3_stmt *compiled_statment;
    if (sqlite3_prepare_v2(this->db, sql, -1,
                           &compiled_statment, NULL) == SQLITE_OK){
        std::map<int, int> *neuron_id_to_key_map = new std::map<int, int>();
        while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
            int neuron_id = sqlite3_column_int(compiled_statment, 0);
            int key = sqlite3_column_int(compiled_statment, 1);
            (*neuron_id_to_key_map)[neuron_id] = key;
        }
        free(sql);
        return neuron_id_to_key_map;
    } else {
        fprintf(stderr, "Error reading database: %i: %s\n",
                sqlite3_errcode(this->db),
                sqlite3_errmsg(this->db));
        exit(-1);
    }
}

ip_tag_info *DatabaseReader::get_live_output_details(char *label) {
    char *sql = sqlite3_mprintf(
        "SELECT tag.ip_address, tag.port, tag.strip_sdp FROM IP_tags as tag"
        " JOIN graph_mapper_vertex as mapper"
        " ON tag.vertex_id = mapper.partitioned_vertex_id"
        " JOIN Partitionable_vertices as post_vertices"
        " ON mapper.partitionable_vertex_id = post_vertices.vertex_id"
        " JOIN Partitionable_edges as edges"
        " ON mapper.partitionable_vertex_id == edges.post_vertex"
        " JOIN Partitionable_vertices as pre_vertices"
        " ON edges.pre_vertex == pre_vertices.vertex_id"
        " WHERE pre_vertices.vertex_label == \"%q\""
        " AND post_vertices.vertex_label == \"Monitor\"", label);
    sqlite3_stmt *compiled_statment;
    if (sqlite3_prepare_v2(this->db, sql, -1,
                           &compiled_statment, NULL) == SQLITE_OK){
        free(sql);
        if (sqlite3_step(compiled_statment) == SQLITE_ROW) {
            ip_tag_info *tag_info = (ip_tag_info *) malloc(sizeof(ip_tag_info));
            tag_info->ip_address = get_column_string_copy(compiled_statment, 0);
            tag_info->port = sqlite3_column_int(compiled_statment, 1);
            tag_info->strip_sdp = sqlite3_column_int(compiled_statment, 2) == 1;
            return tag_info;
        }
        fprintf(stderr, "No ip tag found for population %s\n", label);
        exit(-1);
    } else {
        fprintf(stderr, "Error reading database: %i: %s\n",
                sqlite3_errcode(this->db),
                sqlite3_errmsg(this->db));
        exit(-1);
    }
}

reverse_ip_tag_info *DatabaseReader::get_live_input_details(char *label) {
    char *sql = sqlite3_mprintf(
        "SELECT tag.board_address, tag.port as port"
        " FROM Reverse_IP_tags as tag"
        " JOIN graph_mapper_vertex as mapper"
        " ON tag.vertex_id = mapper.partitioned_vertex_id"
        " JOIN Partitionable_vertices as partitionable"
        " ON mapper.partitionable_vertex_id = partitionable.vertex_id"
        " WHERE partitionable.vertex_label=\"%q\"", label);
    sqlite3_stmt *compiled_statment;
    if (sqlite3_prepare_v2(this->db, sql, -1,
                           &compiled_statment, NULL) == SQLITE_OK){
        free(sql);
        if (sqlite3_step(compiled_statment) == SQLITE_ROW) {
            reverse_ip_tag_info *tag_info =
                (reverse_ip_tag_info *) malloc(sizeof(reverse_ip_tag_info));
            tag_info->board_address = get_column_string_copy(compiled_statment,
                                                             0);
            tag_info->port = sqlite3_column_int(compiled_statment, 1);
            return tag_info;
        }
        fprintf(stderr, "No reverse ip tag found for population %s\n", label);
        exit(-1);
    } else {
        fprintf(stderr, "Error reading database: %i: %s\n",
                sqlite3_errcode(this->db),
                sqlite3_errmsg(this->db));
        exit(-1);
    }
}

int DatabaseReader::get_n_neurons(char *label) {
    char *sql = sqlite3_mprintf(
        "SELECT no_atoms FROM Partitionable_vertices"
        " WHERE vertex_label = \"%q\"", label);
    sqlite3_stmt *compiled_statment;
    if (sqlite3_prepare_v2(this->db, sql, -1,
                           &compiled_statment, NULL) == SQLITE_OK){
        free(sql);
        if (sqlite3_step(compiled_statment) == SQLITE_ROW) {
            return sqlite3_column_int(compiled_statment, 0);
        }
        fprintf(stderr, "Population %s not found\n", label);
        exit(-1);
    } else {
        fprintf(stderr, "Error reading database: %i: %s\n",
                sqlite3_errcode(this->db),
                sqlite3_errmsg(this->db));
        exit(-1);
    }
}

float DatabaseReader::get_configuration_parameter_value(char *parameter_name) {
    char *sql = sqlite3_mprintf(
        "SELECT value FROM configuration_parameters"
        " WHERE parameter_id = \"%q\"", parameter_name);
    sqlite3_stmt *compiled_statment;
    if (sqlite3_prepare_v2(this->db, sql, -1,
                           &compiled_statment, NULL) == SQLITE_OK){
        free(sql);
        if (sqlite3_step(compiled_statment) == SQLITE_ROW) {
            return sqlite3_column_double(compiled_statment, 0);
        }
        fprintf(stderr, "Parameter %s not found\n", parameter_name);
        exit(-1);
    } else {
        fprintf(stderr, "Error reading database: %i: %s\n",
                sqlite3_errcode(this->db),
                sqlite3_errmsg(this->db));
        exit(-1);
    }
}

void DatabaseReader::close_database_connection(){
    sqlite3_close(this->db);
}
