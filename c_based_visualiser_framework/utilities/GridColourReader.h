#include <vector>
#include <map>
#include <string>
#include "colour.h"

#ifndef _COLOUR_MAP_H_
#define _COLOUR_MAP_H_

class GridColourReader {

    public:
        GridColourReader(char *path);
        std::vector<char *> *get_labels();
        colour get_colour(float state);

    private:
        std::map<float, colour> colour_map;
        std::vector<char *> *labels;
};

#endif
