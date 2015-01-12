/*
 * databasereader.cpp
 *
 *  Created on: 2 Dec 2014
 *      Author: alan
 */

#include "DatabaseReader.h"
#include "colour.h"

#include <map>
#include <sqlite3.h>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <stdlib.h>

using namespace std;

struct database_data{
	int no_columns;
	char ** fields;
	char ** attributes;
};

DatabaseReader::DatabaseReader(char* given_path_to_database) {
    char* absolute_path_to_database;
    map<int, char*> y_axis_labels;
    map<int, int> key_to_neuronid_map;

    absolute_path_to_database = given_path_to_database;
    int rc;
    rc = sqlite3_open(given_path_to_database, &this->db);
    if(rc){
       fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(this->db));
    }
    else{
      fprintf(stderr, "Opened database successfully\n");
    }
}

map<int, int> DatabaseReader::read_database_for_keys(){
	//convert sql to char
	string sqls =
			"SELECT n.neuron_id, n.key n.vertex_id FROM key_to_neuron_mapping "
			"as n AND Partitionable_vertices as p INNER JOIN n ON "
			"p.vertex_id=n.vertex_id WHERE p.recorded == 1 "
			"ORDER BY p.vertex_id, n.neuron_id";
	char* sql = &sqls[0];
	map<int, int> key_to_neuronid_map;
	sqlite3_stmt *compiled_statment;
	int offset = 0;
	int current_counter = 0;
	int current_vertex_id = -1;
	if (sqlite3_prepare_v2(this->db, sql, -1,
			               &compiled_statment, NULL) == SQLITE_OK){
		while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
			int neuron_id = sqlite3_column_int(compiled_statment, 1);
			int key = sqlite3_column_int(compiled_statment, 2);
			int vertex_id = sqlite3_column_int(compiled_statment, 2);
			key_to_neuronid_map[neuron_id + offset] = key;
			if (vertex_id != current_vertex_id){
				offset = current_counter;
				current_vertex_id = vertex_id;
			}
			current_counter ++;
		 }
	}
	//close query and return labels
	sqlite3_finalize(compiled_statment);
	return key_to_neuronid_map;
}

map<int, char*> DatabaseReader::read_database_for_labels(){
	// convert sql to char
	string sqls = "SELECT vertex_label, no_atoms FROM Partitionable_vertices "
			      "WHERE recorded == 1 SORTBY vertex_id";
	int offset = 0;
	char* sql = &sqls[0];
	map<int, char*> y_axis_labels;
	sqlite3_stmt *compiled_statment;
	/* Execute SQL statement */
	if (sqlite3_prepare_v2(this->db, sql, -1,
			               &compiled_statment, NULL) == SQLITE_OK){
		 while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
			char* label = (char*)sqlite3_column_text(compiled_statment, 1);
			int n_atoms = sqlite3_column_int(compiled_statment, 2);
			y_axis_labels[(n_atoms/2) + offset] = label;
			offset += n_atoms;
		 }
	}
	//close query and return labels
	sqlite3_finalize(compiled_statment);
	return y_axis_labels;
}

map<int, struct colour> DatabaseReader::read_color_map(char* colour_file_path){
	// reads in the colour mapping file and expands
	map<string, struct colour> pop_label_to_colour_map;
	FILE *colour_fp = fopen(colour_file_path, "r");
	char line[80];
	while (fgets(line, 80, colour_fp) != NULL) {
		int r;
		int g;
		int b;
		struct colour colour;
		char *pop_label = (char *) malloc(80);
		sscanf(line, "%s\t%d\t%d\t%d",pop_label, &r, &g, &b);
		colour.r = (float) r / 255.0;
		colour.g = (float) g / 255.0;
		colour.b = (float) b / 255.0;
		string key(pop_label);
		free(pop_label);
		pop_label_to_colour_map[key] = colour;
	}
	fclose(colour_fp);
	// convert sql to char
	string sqls = "SELECT vertex_label, no_atoms FROM Partitionable_vertices "
				  "WHERE recorded == 1 SORTBY vertex_id";
	int offset = 0;
	char* sql = &sqls[0];
	map<int, struct colour> neuron_id_to_colour_map;
	sqlite3_stmt *compiled_statment;
	/* Execute SQL statement */
	if (sqlite3_prepare_v2(this->db, sql, -1,
						   &compiled_statment, NULL) == SQLITE_OK){
		 while (sqlite3_step(compiled_statment) == SQLITE_ROW) {
			char* label = (char*)sqlite3_column_text(compiled_statment, 1);
			string key(label);
			int n_atoms = sqlite3_column_int(compiled_statment, 2);
			// convert to neuron id mapping
			int nid =0;
			for(nid=0; nid < n_atoms; nid++){
				neuron_id_to_colour_map[nid + offset] = pop_label_to_colour_map[key];
			}
			offset += n_atoms;
		 }
	}
	//close query and return labels
	sqlite3_finalize(compiled_statment);
	return neuron_id_to_colour_map;
}

void DatabaseReader::close_database_connection(){
	sqlite3_close(this->db);
}

DatabaseReader::~DatabaseReader() {
	// TODO Auto-generated destructor stub
}

