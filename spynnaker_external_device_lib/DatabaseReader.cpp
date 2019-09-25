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

static void __attribute__((noreturn)) fail(const char *fmt,...) {
    va_list arg_list;

    va_start(arg_list, fmt);
    vfprintf(stderr, fmt, arg_list);
    va_end(arg_list);
    exit(1);
}

DatabaseReader::DatabaseReader(char *database_path) {
    int rc;
    fprintf(stderr, "Reading database %s\n", database_path);
    rc = sqlite3_open(database_path, &this->db);
    if (rc) {
	fail("Can't open database: %s\n", sqlite3_errmsg(this->db));
    }
    fprintf(stderr, "Opened database successfully\n");
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
    if (sqlite3_prepare_v2(this->db, sql, -1,
                           &compiled_statment, NULL) != SQLITE_OK) {
	fail("Error reading database: %i: %s\n",
		sqlite3_errcode(this->db), sqlite3_errmsg(this->db));
    }
    sqlite3_free(sql);
    return compiled_statment;
}

std::vector<char *> *DatabaseReader::get_live_output_population_labels() {
    sqlite3_stmt *compiled_statment = db_prepare(
	    "SELECT pre_vertices.vertex_label"
	    " FROM Application_vertices as pre_vertices"
	    " JOIN Application_edges as edges"
	    " ON pre_vertices.vertex_id == edges.pre_vertex"
	    " JOIN Application_vertices as post_vertices"
	    " ON edges.post_vertex = post_vertices.vertex_id"
	    " WHERE post_vertices.vertex_label LIKE \"LiveSpikeReceiver%\"");

    std::vector<char *> *labels = new std::vector<char *>();
    while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
	char *label = get_column_string_copy(compiled_statment, 0);
	labels->push_back(label);
    }

    return labels;
}

std::map<int, int> *DatabaseReader::get_key_to_neuron_id_mapping(char* label) {
    sqlite3_stmt *compiled_statment = db_prepare(
	    "SELECT n.atom_id as a_id, n.event_id as event"
	    " FROM event_to_atom_mapping as n"
	    " JOIN Application_vertices as p"
	    " ON n.vertex_id = p.vertex_id"
	    " WHERE p.vertex_label=\"%q\"", label);

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
	    "SELECT n.atom_id as a_id, n.event_id as event"
	    " FROM event_to_atom_mapping as n"
	    " JOIN Application_vertices as p"
	    " ON n.vertex_id = p.vertex_id"
	    " WHERE p.vertex_label=\"%q\"", label);

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
	    "SELECT tag.ip_address, tag.port, tag.strip_sdp"
	    " FROM IP_tags as tag"
	    " JOIN graph_mapper_vertex as mapper"
	    " ON tag.vertex_id = mapper.machine_vertex_id"
	    " JOIN Application_vertices as post_vertices"
	    " ON mapper.application_vertex_id = post_vertices.vertex_id"
	    " JOIN Application_edges as edges"
	    " ON mapper.application_vertex_id == edges.post_vertex"
	    " JOIN Application_vertices as pre_vertices"
	    " ON edges.pre_vertex == pre_vertices.vertex_id"
	    " WHERE pre_vertices.vertex_label == \"%q\""
	    " AND post_vertices.vertex_label LIKE \"LiveSpikeReceiver%\"",
	    label);

    if (sqlite3_step(compiled_statment) != SQLITE_ROW) {
	fail("No IP tag found for population %s\n", label);
    }

    ip_tag_info *tag_info = (ip_tag_info *) malloc(sizeof(ip_tag_info));
    tag_info->ip_address = get_column_string_copy(compiled_statment, 0);
    tag_info->port = sqlite3_column_int(compiled_statment, 1);
    tag_info->strip_sdp = sqlite3_column_int(compiled_statment, 2) == 1;
    return tag_info;
}

reverse_ip_tag_info *DatabaseReader::get_live_input_details(char *label) {
    sqlite3_stmt *compiled_statment = db_prepare(
	    "SELECT tag.board_address, tag.port as port"
	    " FROM Reverse_IP_tags as tag"
	    " JOIN graph_mapper_vertex as mapper"
	    " ON tag.vertex_id = mapper.machine_vertex_id"
	    " JOIN Application_vertices as application"
	    " ON mapper.application_vertex_id = application.vertex_id"
	    " WHERE application.vertex_label=\"%q\"", label);

    if (sqlite3_step(compiled_statment) != SQLITE_ROW) {
	fail("No reverse IP tag found for population %s\n", label);
    }

    reverse_ip_tag_info *tag_info = (reverse_ip_tag_info *)
	    malloc(sizeof(reverse_ip_tag_info));
    tag_info->board_address = get_column_string_copy(compiled_statment, 0);
    tag_info->port = sqlite3_column_int(compiled_statment, 1);
    return tag_info;
}

int DatabaseReader::get_n_neurons(char *label) {
    sqlite3_stmt *compiled_statment = db_prepare(
	    "SELECT no_atoms FROM Application_vertices"
	    " WHERE vertex_label = \"%q\"", label);

    if (sqlite3_step(compiled_statment) != SQLITE_ROW) {
	fail("Population %s not found\n", label);
    }

    return sqlite3_column_int(compiled_statment, 0);
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

void DatabaseReader::close_database_connection() {
    sqlite3_close(this->db);
}
