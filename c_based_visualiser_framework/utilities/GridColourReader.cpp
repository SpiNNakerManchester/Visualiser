#include "GridColourReader.h"

#include <stdio.h>
#include <stdlib.h>

GridColourReader::GridColourReader(char *path) {
    this->labels = new std::vector<char *>();

    FILE *colour_fp = fopen(path, "r");
    if (colour_fp == NULL) {
        fprintf(stderr, "Colour file %s not found\n", path);
        exit(-1);
    }
    char line[80];
    bool found_state = false;
    bool is_state = false;
    while (fgets(line, 80, colour_fp) != NULL) {
        if(! found_state){
            if( line == "state"){
                is_state = true;
            }
            else{
                is_state = false;
            }
            found_state = true;
        }
        else{
            if(is_state){
                int r;
                int g;
                int b;
                float state;
                struct colour colour;
                char *pop_label = (char *) malloc(80);

                sscanf(line, "%s\t%f\t%d\t%d\t%d", pop_label, &r, &g, &b);
                colour.r = (float) r / 255.0;
                colour.g = (float) g / 255.0;
                colour.b = (float) b / 255.0;

                // store colour to state
                this->colour_map[state] = colour;

                // store label
                std::string key(pop_label);
                this->labels->push_back(pop_label);
            }
            else{
                printf("Havent programmed the range based data yet");
            }
        }
    }
    fclose(colour_fp);
}

std::vector<char *> *ColourReader::get_labels() {
    return this->labels;
}

colour GridColourReader::get_colour(float state) {
    return this->colour_map[state];
}
