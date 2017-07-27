#include "DatabaseReader.h"

#include <stdio.h>
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
    fprintf(stderr, "Reading database %s\n", database_path);

    int rc = sqlite3_open(database_path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Opened database successfully\n");
}

DatabaseReader::~DatabaseReader() {
    close_database_connection();
}

sqlite3 *DatabaseReader::get_cursor() {
    return db;
}

void DatabaseReader::prepare(sqlite3_stmt *&statement, const char *sql) {
    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error reading database: %i: %s\n",
                sqlite3_errcode(db), sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    }
}

void DatabaseReader::bind(sqlite3_stmt *&statement, int index,
	const char *value) {
    if (sqlite3_bind_text(statement, 1, value, -1, SQLITE_STATIC) != SQLITE_OK) {
        fprintf(stderr, "Error binding value to statement: %i: %s\n",
        	sqlite3_errcode(db), sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    }
}

void DatabaseReader::step1(sqlite3_stmt *&statement, const char *msg,
	const char *value) {
    if (sqlite3_step(statement) != SQLITE_ROW) {
	fprintf(stderr, msg, value);
	exit(EXIT_FAILURE);
    }
}

std::vector<char *> *DatabaseReader::get_live_output_population_labels() {
    const char *sql =
	    "SELECT pre_vertices.vertex_label"
	    " FROM Application_vertices AS pre_vertices"
	    " JOIN Application_edges AS edges"
	    "   ON pre_vertices.vertex_id = edges.pre_vertex"
	    " JOIN Application_vertices AS post_vertices"
	    "   ON edges.post_vertex = post_vertices.vertex_id"
	    " WHERE post_vertices.vertex_label = 'LiveSpikeReceiver'";
    sqlite3_stmt *compiled_statment;
    prepare(compiled_statment, sql);

    std::vector<char *> *labels = new std::vector<char *>();
    while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
	char *label = get_column_string_copy(compiled_statment, 0);
	labels->push_back(label);
    }
    return labels;
}

std::map<int, int> *DatabaseReader::get_key_to_neuron_id_mapping(char* label) {
    const char *sql =
	    "SELECT n.atom_id AS a_id, n.event_id AS event"
	    " FROM event_to_atom_mapping AS n"
	    " JOIN Application_vertices AS p"
	    "   ON n.vertex_id = p.vertex_id"
	    " WHERE p.vertex_label = ?";
    sqlite3_stmt *compiled_statment;
    prepare(compiled_statment, sql);
    bind(compiled_statment, 1, label);

    auto key_to_neuron_id_map = new std::map<int, int>();
    while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
	int neuron_id = sqlite3_column_int(compiled_statment, 0);
	int key = sqlite3_column_int(compiled_statment, 1);
	(*key_to_neuron_id_map)[key] = neuron_id;
    }
    return key_to_neuron_id_map;
}

std::map<int, int> *DatabaseReader::get_neuron_id_to_key_mapping(char* label) {
    const char *sql =
	    "SELECT n.atom_id AS a_id, n.event_id AS event"
	    " FROM event_to_atom_mapping AS n"
	    " JOIN Application_vertices AS p"
	    "   ON n.vertex_id = p.vertex_id"
	    " WHERE p.vertex_label = ?";
    sqlite3_stmt *compiled_statment;
    prepare(compiled_statment, sql);
    bind(compiled_statment, 1, label);

    auto neuron_id_to_key_map = new std::map<int, int>();
    while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
	int neuron_id = sqlite3_column_int(compiled_statment, 0);
	int key = sqlite3_column_int(compiled_statment, 1);
	(*neuron_id_to_key_map)[neuron_id] = key;
    }
    return neuron_id_to_key_map;
}

ip_tag_info *DatabaseReader::get_live_output_details(char *label) {
    const char *sql =
	    "SELECT tag.ip_address, tag.port, tag.strip_sdp"
	    " FROM IP_tags AS tag"
	    " JOIN graph_mapper_vertex AS mapper"
	    "   ON tag.vertex_id = mapper.machine_vertex_id"
	    " JOIN Application_vertices AS post_vertices"
	    "   ON mapper.application_vertex_id = post_vertices.vertex_id"
	    " JOIN Application_edges AS edges"
	    "   ON mapper.application_vertex_id = edges.post_vertex"
	    " JOIN Application_vertices AS pre_vertices"
	    "   ON edges.pre_vertex = pre_vertices.vertex_id"
	    " WHERE pre_vertices.vertex_label = ?"
	    "   AND post_vertices.vertex_label = 'LiveSpikeReceiver'"
	    " LIMIT 1";
    sqlite3_stmt *compiled_statment;
    prepare(compiled_statment, sql);
    bind(compiled_statment, 1, label);
    step1(compiled_statment, "No ip tag found for population %s\n", label);

    ip_tag_info *tag_info = (ip_tag_info *) malloc(sizeof(ip_tag_info));
    tag_info->ip_address = get_column_string_copy(compiled_statment, 0);
    tag_info->port = sqlite3_column_int(compiled_statment, 1);
    tag_info->strip_sdp = sqlite3_column_int(compiled_statment, 2) == 1;
    return tag_info;
}

reverse_ip_tag_info *DatabaseReader::get_live_input_details(char *label) {
    const char *sql =
	    "SELECT tag.board_address, tag.port AS port"
	    " FROM Reverse_IP_tags AS tag"
	    " JOIN graph_mapper_vertex AS mapper"
	    "   ON tag.vertex_id = mapper.machine_vertex_id"
	    " JOIN Application_vertices AS application"
	    "   ON mapper.application_vertex_id = application.vertex_id"
	    " WHERE application.vertex_label = ?"
	    " LIMIT 1";
    sqlite3_stmt *compiled_statment;
    prepare(compiled_statment, sql);
    bind(compiled_statment, 1, label);
    step1(compiled_statment,
	    "No reverse ip tag found for population %s\n", label);

    reverse_ip_tag_info *tag_info =
	    (reverse_ip_tag_info *) malloc(sizeof(reverse_ip_tag_info));
    tag_info->board_address = get_column_string_copy(compiled_statment, 0);
    tag_info->port = sqlite3_column_int(compiled_statment, 1);
    return tag_info;
}

int DatabaseReader::get_n_neurons(char *label) {
    const char *sql =
        "SELECT no_atoms FROM Application_vertices"
        " WHERE vertex_label = ?"
        " LIMIT 1";
    sqlite3_stmt *compiled_statment;
    prepare(compiled_statment, sql);
    bind(compiled_statment, 1, label);
    step1(compiled_statment, "Population %s not found\n", label);

    return sqlite3_column_int(compiled_statment, 0);
}

float DatabaseReader::get_configuration_parameter_value(char *parameter_name) {
    const char *sql =
        "SELECT value FROM configuration_parameters"
        " WHERE parameter_id = ?"
        " LIMIT 1";
    sqlite3_stmt *compiled_statment;
    prepare(compiled_statment, sql);
    bind(compiled_statment, 1, parameter_name);
    step1(compiled_statment, "Parameter %s not found\n", parameter_name);

    return sqlite3_column_double(compiled_statment, 0);
}

void DatabaseReader::close_database_connection() {
    sqlite3_close(db);
    db = nullptr;
}
