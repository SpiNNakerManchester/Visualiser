#include "ColourReader.h"

#include <stdio.h>
#include <stdlib.h>

ColourReader::ColourReader(char *path) {
    FILE *colour_fp = fopen(path, "r");
    if (colour_fp == NULL) {
        fprintf(stderr, "Colour file %s not found\n", path);
        exit(-1);
    }
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
        std::string key(pop_label);
        this->colour_map[key] = colour;
    }
    fclose(colour_fp);
}

std::vector<char *> *ColourReader::get_labels() {
    std::vector<char *> *labels = new std::vector<char *>();
    for (std::map<std::string, colour>::iterator iter
            = this->colour_map.begin(); iter != this->colour_map.end();
            ++iter) {
        labels->push_back((char *) iter->first.c_str());
    }
    return labels;
}

colour ColourReader::get_colour(char *label) {
    std::string key(label);
    return this->colour_map[key];
}
