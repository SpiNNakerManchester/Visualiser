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
#include "pendulum_view/PendulumPlot.h"

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

    char* absolute_file_path = NULL;
    char* packet_file_path = NULL;
    char* remote_host = NULL;
    int input_width = -1;
    int input_height = -1;
    int output_width = -1;
    int output_height = -1;
    int output_offset = -1;
    int output_per_core_width = -1;
    int output_per_core_height = -1;

    for (int arg_index = 1; arg_index < argc; arg_index+=2){
        if (strcmp(argv[arg_index], "-database") == 0){
            absolute_file_path = get_next_arg(arg_index, argv, argc);
        }
        if (strcmp(argv[arg_index], "-input_dims") == 0){
            input_width = atoi(get_next_arg(arg_index++, argv, argc));
            input_height = atoi(get_next_arg(arg_index, argv, argc));
        }
        if (strcmp(argv[arg_index], "-output_dims") == 0){
            output_width = atoi(get_next_arg(arg_index++, argv, argc));
            output_height = atoi(get_next_arg(arg_index++, argv, argc));
            output_offset = atoi(get_next_arg(arg_index++, argv, argc));
            output_per_core_width = atoi(get_next_arg(arg_index++, argv, argc));
            output_per_core_height = atoi(get_next_arg(arg_index, argv, argc));
        }
        if (strcmp(argv[arg_index], "-remote_host") == 0) {
            remote_host = get_next_arg(arg_index, argv, argc);
        }
    }

    if (input_width <= 0 || input_height <= 0 || output_width <= 0 ||
            output_height <= 0 || output_offset <= 0 ||
            output_per_core_width <= 0 || output_per_core_height <= 0) {
        printf("Missing args\n");
        return 1;
    }

    std::vector<char *> labels;
    char *source_label = (char *) malloc(sizeof(char) * 20);
    sprintf(source_label, "source");
    labels.push_back(source_label);

    SpynnakerLiveSpikesConnection connection(
        (int) labels.size(), &(labels[0]), 0, (char **) NULL,
        (char *) NULL, 0, false);
    std::cerr << "Listening on " << connection.get_local_port() << "\n";

    std::vector<char *> labels2;
    char *target_label = (char *) malloc(sizeof(char) * 20);
    sprintf(target_label, "target");
    labels2.push_back(target_label);
    SpynnakerLiveSpikesConnection connection2(
            (int) labels2.size(), &(labels2[0]), 0, (char **) NULL,
            (char *) NULL, 0, false);
        std::cerr << "Listening on " << connection2.get_local_port() << "\n";

    fprintf(stderr, "Output dims: %i x %i, offset = %i, per core = %i x %i\n",
            output_width, output_height, output_offset,
            output_per_core_width, output_per_core_height);

    // Create the visualiser
    PendulumPlot plotter(argc, argv, true, input_width, input_height,
            output_width, output_height, output_offset, output_per_core_width,
            output_per_core_height);
    connection.add_initialize_callback(source_label, &plotter);
    connection.add_receive_callback(source_label, &plotter);
    connection.add_start_callback(source_label, &plotter);
    connection2.add_receive_callback(target_label, plotter.get_target_receiver());

    if (absolute_file_path != NULL) {
        connection.set_database(absolute_file_path);
    }
    plotter.main_loop();
    return 0;
}
