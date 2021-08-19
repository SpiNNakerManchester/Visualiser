/*
 * Copyright (c) 2015-2021 The University of Manchester
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ColourReader.h"

#include <stdio.h>
#include <stdlib.h>

ColourReader::ColourReader(char *path) {
    this->labels = new std::vector<char *>();

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
        sscanf(line, "%s\t%d\t%d\t%d", pop_label, &r, &g, &b);
        colour.r = (float) r / 255.0;
        colour.g = (float) g / 255.0;
        colour.b = (float) b / 255.0;
        std::string key(pop_label);
        this->colour_map[key] = colour;
        this->labels->push_back(pop_label);
    }
    fclose(colour_fp);
}

std::vector<char *> *ColourReader::get_labels() {
    return this->labels;
}

colour ColourReader::get_colour(char *label) {
    std::string key(label);
    return this->colour_map[key];
}
