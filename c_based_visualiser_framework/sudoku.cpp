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
#include "sudoku_view/SudokuPlot.h"

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
    int neurons_per_number = 0;
    char* remote_host = NULL;
    float ms_per_bin = 0.0;

    for (int arg_index = 1; arg_index < argc; arg_index+=2){

        if (strcmp(argv[arg_index], "-hand_shake_port") == 0){
            hand_shake_listen_port_no = atoi(get_next_arg(
                arg_index, argv, argc));
        }
        if (strcmp(argv[arg_index], "-database") == 0){
            absolute_file_path = get_next_arg(arg_index, argv, argc);
        }
        if (strcmp(argv[arg_index], "-neurons_per_number") == 0){
            neurons_per_number = atoi(get_next_arg(arg_index, argv, argc));
        }
        if (strcmp(argv[arg_index], "-remote_host") == 0) {
            remote_host = get_next_arg(arg_index, argv, argc);
        }
        if (strcmp(argv[arg_index], "-ms_per_bin") == 0) {
            ms_per_bin = atof(get_next_arg(arg_index, argv, argc));
        }
    }

    if (neurons_per_number <= 0 || ms_per_bin <= 0.0) {
        printf("Usage is \n"
               " -neurons_per_number "
               "<the number of neurons that represent each number in a cell>\n"
               " -ms_per_bin "
               "<optional number of milliseconds to show at once>\n"
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
    /*for (uint32_t i = 0; i < 81; i++) {
        uint32_t x = (i / 9) + 1;
        uint32_t y = (i % 9) + 1;
        char *label = (char *) malloc(sizeof(char) * 7);
        sprintf(label, "Cell%u%u", x, y);
        labels.push_back(label);
    }*/
    char *label = (char *) malloc(sizeof(char) * 7);
    sprintf(label, "Cells");
    labels.push_back(label);

    SpynnakerLiveSpikesConnection connection(
        (int) labels.size(), &(labels[0]), 0, (char **) NULL,
        (char *) NULL, hand_shake_listen_port_no, false);

    // Create the visualiser
    SudokuPlot plotter(argc, argv, neurons_per_number, ms_per_bin,
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
