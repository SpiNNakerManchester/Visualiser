#include <vector>
#include <map>
#include <string>
#include "colour.h"

#ifndef _COLOUR_MAP_H_
#define _COLOUR_MAP_H_

class ColourReader {

    public:
        ColourReader(char *path);
        virtual ~ColourReader();
        std::vector<char *> *get_labels();
        colour get_colour(char *label);

    private:
        std::map<std::string, colour> colour_map;
        std::vector<char *> *labels;
};

#endif
