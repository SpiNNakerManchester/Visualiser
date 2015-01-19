#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <map>
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
    int plot_time_ms = -1;
    float timestep_ms = -1.0;
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
		if (strcmp(argv[arg_index], "-plot_time") == 0){
            plot_time_ms = atoi(get_next_arg(arg_index, argv, argc));
        }
		if (strcmp(argv[arg_index], "-timestep") == 0){
            timestep_ms = atof(get_next_arg(arg_index, argv, argc));
        }
		if (strcmp(argv[arg_index], "-remote_host") == 0) {
		    remote_host = get_next_arg(arg_index, argv, argc);
		}
    }

	if (hand_shake_listen_port_no == -1 or colour_file_path == NULL
			or absolute_file_path == NULL or packet_listener_port_no == -1
			or plot_time_ms == -1 or timestep_ms == -1.0) {
		printf("Usage is \n "
				"-hand_shake_port "
				"<port which the visualiser will listen to for database hand shaking> \n"
		        " -database "
		        "<file path to where the database is located>\n"
			    " -colour_map "
			    "<file path to where the colour is located>\n"
				" -port "
				"<port which the visualiser will listen for packets>\n"
		        " -plot_time "
		        "<duration of the simulation is milliseconds>\n"
		        " -timestep "
		        "<simulation timestep, in milliseconds\n"
		        " [-remote_host] "
		        "<optional remote host, which will allow port triggering>\n");
		return 1;
	}

	std::map<int, char*> *y_axis_labels;
	std::map<int, int> *key_to_neuronid_map;
	std::map<int, struct colour> *neuron_id_to_colour_map;
	// initilise the visulaiser config
	DatabaseMessageConnection hand_shaker(hand_shake_listen_port_no);
	eieio_message message;
	printf("awaiting tool chain hand shake to say database is ready \n");
    message = hand_shaker.recieve_notification();
	printf("received tool chain hand shake to say database is ready \n");
	DatabaseReader reader(absolute_file_path);
	printf("reading in labels \n");
	y_axis_labels = reader.read_database_for_labels();
	printf("reading in keys \n");
	key_to_neuronid_map = reader.read_database_for_keys();
	printf("reading in colour maps\n");
	neuron_id_to_colour_map = reader.read_color_map(colour_file_path);
	printf("closing database connection \n");
	reader.close_database_connection();
	// create and send the eieio command message confirming database read
	printf("send confirmation database connection \n");
	hand_shaker.send_ready_notification();
	hand_shaker.close_connection();
	printf("close connection \n");
	// set up visualiser packet listener
	fprintf(stderr, "Starting\n");
	SocketQueuer queuer(packet_listener_port_no, remote_host);
	queuer.start();
    //create visualiser
    RasterPlot plotter(argc, argv, &queuer, y_axis_labels,
                       key_to_neuronid_map, neuron_id_to_colour_map,
                       plot_time_ms, timestep_ms);
	return 0;
}
