#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <map>
#include <string>
#include <deque>
#include <string.h>
#include "main.h"
#include "utilities/DatabaseReader.h"
#include "utilities/DatabaseMessageConnection.h"
#include "utilities/SocketQueuer.h"
#include "raster_view/RasterPlot.h"
#include "utilities/colour.h"

/*
 * main.cpp
 *main entrnace to the vis
 *  Created on: 2 Dec 2014
 *      Author: alan and rowley
 */

char* get_next_arg(int position, char **argv, int argc){
	if (position + 1 > argc){
		throw "missing a element";
	}
	else{
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

    int hand_shake_listen_port_no = -1;
    int packet_listener_port_no = -1;
    char* absolute_file_path = NULL;
    char* colour_file_path = NULL;
    char *remote_host = NULL;

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
		if (strcmp(argv[arg_index], "-port") == 0){
			packet_listener_port_no = atoi(get_next_arg(arg_index, argv, argc));
		}
		if (strcmp(argv[arg_index], "-remote_host") == 0) {
		    remote_host = get_next_arg(arg_index, argv, argc);
		}
    }

	if (colour_file_path == NULL
			or absolute_file_path == NULL or packet_listener_port_no == -1) {
		printf("Usage is \n "
				"[-hand_shake_port]"
				"<optional port which the visualiser will listen to for"
		        " database hand shaking> \n"
		        " -database "
		        "<file path to where the database is located>\n"
			    " -colour_map "
			    "<file path to where the colour is located>\n"
				" -port "
				"<port which the visualiser will listen for packets>\n"
		        " [-remote_host] "
		        "<optional remote host, which will allow port triggering>\n");
		return 1;
	}

	std::map<int, char*> *y_axis_labels;
	std::map<int, int> *key_to_neuronid_map;
	std::map<int, struct colour> *neuron_id_to_colour_map;
	// initilise the visulaiser config

	DatabaseMessageConnection *hand_shaker = NULL;
	if (hand_shake_listen_port_no != -1) {
        hand_shaker = new DatabaseMessageConnection(hand_shake_listen_port_no);
        printf("awaiting tool chain hand shake to say database is ready \n");
        hand_shaker->recieve_notification();
        printf("received tool chain hand shake to say database is ready \n");
	}

	DatabaseReader reader(absolute_file_path);
	printf("reading in labels \n");
	y_axis_labels = reader.read_database_for_labels();
	printf("reading in keys \n");
	key_to_neuronid_map = reader.read_database_for_keys();
	printf("reading in colour maps\n");
	neuron_id_to_colour_map = reader.read_color_map(colour_file_path);
	printf("reading parameters\n");
	std::map<std::string, float> *config_params =
	        reader.get_configuration_parameters();
	printf("closing database connection \n");
	reader.close_database_connection();

	if (hand_shaker != NULL) {

        // create and send the eieio command message confirming database read
        printf("send confirmation database connection \n");
        hand_shaker->send_ready_notification();
        hand_shaker->close_connection();
        delete hand_shaker;
        printf("close connection \n");
	}

	// set up visualiser packet listener
	fprintf(stderr, "Starting\n");
	SocketQueuer queuer(packet_listener_port_no, remote_host);
	queuer.start();

    //create visualiser
    RasterPlot plotter(argc, argv, &queuer, y_axis_labels,
                       key_to_neuronid_map, neuron_id_to_colour_map,
                       (*config_params)["runtime"],
                       (*config_params)["machine_time_step"] / 1000.0);
	return 0;
}
