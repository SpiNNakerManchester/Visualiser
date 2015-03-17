#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <map>
#include <string>
#include <vector>
#include <string.h>
#include "main.h"
#include "utilities/DatabaseReader.h"
#include "utilities/ColourReader.h"
#include "utilities/DatabaseMessageConnection.h"
#include "raster_view/RasterPlot.h"

/*
 * main.cpp
 *main entrnace to the vis
 *  Created on: 2 Dec 2014
 *      Author: alan and rowley
 */

char* get_next_arg(int position, char **argv, int argc){
    if (position + 1 > argc){
        throw "missing a element";
    } else{
        return argv[position + 1];
    }
}

int main(int argc, char **argv){

#ifdef WIN32
    WSADATA wsaData; // if this doesn't work
    //WSAData wsaData; // then try this instead

    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }
#endif

    int hand_shake_listen_port_no = 19999;
    char* absolute_file_path = NULL;
    char* packet_file_path = NULL;
    char* colour_file_path = NULL;
    char* remote_host = NULL;

    for (int arg_index = 1; arg_index < argc; arg_index+=2){

        if (strcmp(argv[arg_index], "-hand_shake_port") == 0){
            hand_shake_listen_port_no = atoi(get_next_arg(arg_index, argv, argc));
        }
        if (strcmp(argv[arg_index], "-database") == 0){
            absolute_file_path = get_next_arg(arg_index, argv, argc);
        }
        if (strcmp(argv[arg_index], "-colour_map") == 0){
            colour_file_path = get_next_arg(arg_index, argv, argc);
        }
        if (strcmp(argv[arg_index], "-remote_host") == 0) {
            remote_host = get_next_arg(arg_index, argv, argc);
        }
    }

    if (colour_file_path == NULL) {
        printf("Usage is \n "
               "-colour_map "
               "<Path to a file containing the population labels to receive,"
               " and their associated colours>\n"
               "[-hand_shake_port]"
               "<optional port which the visualiser will listen to for"
               " database hand shaking>\n"
               "[-database]"
               "<optional file path to where the database is located,"
               " if needed for manual configuration>\n"
               "[-remote_host] "
               "<optional remote host, which will allow port triggering>\n");
        return 1;
    }

    DatabaseMessageConnection *database_message_connection = NULL;
    if (hand_shake_listen_port_no != -1) {
        database_message_connection = new DatabaseMessageConnection(
            hand_shake_listen_port_no);
        printf("awaiting tool chain hand shake to say database is ready \n");
        packet_file_path = database_message_connection->recieve_notification();
        printf("received tool chain hand shake to say database is ready \n");
    } else{
        if (!absolute_file_path){
            printf("no hand shaking occured and you give us a path"
                "to the database. Please rectify one of these and try"
                "again \n");
            return 0;
        }
    }

    // Open the database
    DatabaseReader* database = NULL;
    if (!absolute_file_path){
        printf("using packet based address \n");
        database = new DatabaseReader(packet_file_path);
    } else {
        printf("using command based address \n");
        database = new DatabaseReader(absolute_file_path);
    }

    // Get the details of the populations to be visualised
    ColourReader *colour_reader = new ColourReader(colour_file_path);
    std::vector<char *> *labels = colour_reader->get_labels();

    // Read the database and store the results in a useful form
    std::map<int, char*> *y_axis_labels = new std::map<int, char*>();
    std::map<int, int> *key_to_neuronid_map = new std::map<int, int>();
    std::map<int, colour> *neuron_id_to_colour_map =
            new std::map<int, colour>();
    std::vector<int> *ports_to_listen_to = new std::vector<int>();
    int base_neuron_id = 0;
    for (std::vector<char *>::iterator iter = labels->begin();
            iter != labels->end(); iter++) {
        char *label = *iter;

        // Get the port details
        ip_tag_info *tag = database->get_live_output_details(label);
        ports_to_listen_to->push_back(tag->port);
        free(tag);

        // Get the key to neuron id for this population and the colour
        std::map<int, int> *key_map =
            database->get_key_to_neuron_id_mapping(label);
        colour col = colour_reader->get_colour(label);

        // Add the keys to the global maps, adding the current base neuron id
        for (std::map<int, int>::iterator key_iter = key_map->begin();
                key_iter != key_map->end(); key_iter++) {
            int nid = key_iter->second + base_neuron_id;
            (*key_to_neuronid_map)[key_iter->first] = nid;
            (*neuron_id_to_colour_map)[nid] = col;
        }
        delete key_map;

        // Put the label half-way up the population
        int n_neurons = database->get_n_neurons(label);
        (*y_axis_labels)[base_neuron_id + (n_neurons / 2)] = label;

        // Add to the base neurons for the next population (plus a spacer)
        base_neuron_id += n_neurons + 10;
    }

    // Get other parameters
    float run_time = database->get_configuration_parameter_value(
        (char *) "runtime");
    float machine_time_step = database->get_configuration_parameter_value(
        (char *) "machine_time_step") / 1000.0;

    // Close the database
    database->close_database_connection();
    delete database;

    // Create the visualiser
    RasterPlot plotter(
        argc, argv, remote_host, ports_to_listen_to, y_axis_labels,
        key_to_neuronid_map, neuron_id_to_colour_map, run_time,
        machine_time_step, base_neuron_id, database_message_connection);
    return 0;
}
