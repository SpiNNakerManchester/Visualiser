#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <string.h>
#include <algorithm>
#include <SpiNNakerFrontEndCOmmonLiveEventsConnection.h>
#include "main.h"
#include "utilities/ColourReader.h"
#include "grid_view/gridPlot.h"

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
    int grid_size_x = 0;
    int grid_size_y = 0;
    char* remote_host = NULL;

    for (int arg_index = 1; arg_index < argc; arg_index+=2){

        if (strcmp(argv[arg_index], "-hand_shake_port") == 0){
            hand_shake_listen_port_no = atoi(get_next_arg(
                arg_index, argv, argc));
        }
        if (strcmp(argv[arg_index], "-database") == 0){
            absolute_file_path = get_next_arg(arg_index, argv, argc);
        }
        if (strcmp(argv[arg_index], "-grid_size_x") == 0){
           grid_size_x  = atoi(get_next_arg(arg_index, argv, argc));
        }
        if (strcmp(argv[arg_index], "-grid_size_y") == 0){
           grid_size_y  = atoi(get_next_arg(arg_index, argv, argc));
        }
        if (strcmp(argv[arg_index], "-remote_host") == 0) {
            remote_host = get_next_arg(arg_index, argv, argc);
        }
    }

    if (grid_size_x <= 0 || grid_size_y <= 0) {
        printf("Usage is \n"
               " -grid_size_x "
               "<the grid size in the x axis that this visualiser is "
               "presenting>\n"
               " -grid_size_y "
               "<the grid size in the y axis that this visualiser is "
               "presenting>\n"
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

    std::vector<char *> labels;

    char *label = (char *) malloc(sizeof(char) * 7);
    sprintf(label, "Cells");
    labels.push_back(label);

    SpiNNakerFrontEndCOmmonLiveEventsConnection connection(
        (int) labels.size(), &(labels[0]), 0, (char **) NULL,
        (char *) NULL, hand_shake_listen_port_no, false);

    // Create the visualiser
    GridPlotPartitionable plotter(argc, argv, grid_size_x, grid_size_y,
                       absolute_file_path == NULL);
    for (int i = 0; i < labels.size(); i++) {
        connection.add_initialize_callback(labels[i], &plotter);
        connection.add_receive_callback(labels[i], &plotter);
        connection.add_start_callback(labels[i], &plotter);
    }
    if (absolute_file_path != NULL) {
        connection.set_database(absolute_file_path);
    }
    plotter.main_loop();
    return 0;
}
