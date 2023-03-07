/*
 * Copyright (c) 2015 The University of Manchester
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
