#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <string.h>
#include <algorithm>
#include <SpynnakerLiveSpikesConnection.h>
#include "main.h"
#include "utilities/ColourReader.h"
#include "raster_view/RasterPlot.h"

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
    float ms_per_pixel = 0.0;

    for (int arg_index = 1; arg_index < argc; arg_index+=2){

        if (strcmp(argv[arg_index], "-hand_shake_port") == 0){
            hand_shake_listen_port_no = atoi(get_next_arg(
                arg_index, argv, argc));
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
        if (strcmp(argv[arg_index], "-ms_per_pixel") == 0) {
            ms_per_pixel = atof(get_next_arg(arg_index, argv, argc));
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
               "<optional remote host, which will allow port triggering>\n"
               "[-ms_per_pixel] "
               "<optional number of milliseconds to show per pixel (will cause"
               "scrolling if run time is larger than will fit in window)\n");
        return 1;
    }

    // Get the details of the populations to be visualised
    ColourReader *colour_reader = new ColourReader(colour_file_path);
    std::vector<char *> *labels = colour_reader->get_labels();

    SpynnakerLiveSpikesConnection connection(
        (int) labels->size(), &((*labels)[0]), 0, (char **) NULL,
        (char *) NULL, hand_shake_listen_port_no, false);

    // Create the visualiser
    RasterPlot plotter(argc, argv, colour_reader, ms_per_pixel);
    for (int i = 0; i < labels->size(); i++) {
        connection.add_initialize_callback((*labels)[i], &plotter);
        connection.add_receive_callback((*labels)[i], &plotter);
        connection.add_start_callback((*labels)[i], &plotter);
    }
    plotter.main_loop();
    return 0;
}
