#include <map>
#include <string>
#include <vector>
#include <sqlite3.h>

#ifndef _DATABASE_READER_H_
#define _DATABASE_READER_H_

typedef struct ip_tag_info {
    char *ip_address;
    int port;
    bool strip_sdp;
} ip_tag_info;

typedef struct reverse_ip_tag_info {
    char *board_address;
    int port;
} reverse_ip_tag_info;

typedef struct placement {
    int x;
    int y;
    int p;
} placement;

class DatabaseReader {
    sqlite3 *db;
public:
    DatabaseReader(char *database_path);
    virtual ~DatabaseReader();
    sqlite3 *get_cursor();
    std::vector<char *> *get_live_output_population_labels();
    std::map<int, int> *get_key_to_neuron_id_mapping(char* label);
    std::map<int, int> *get_neuron_id_to_key_mapping(char* label);
    ip_tag_info *get_live_output_details(char *label);
    reverse_ip_tag_info *get_live_input_details(char *label);
    std::vector<placement *> *get_placements(char *label);
    char *get_ip_address(int x, int y);
    int get_n_neurons(char *label);
    float get_configuration_parameter_value(char *parameter_name);
    void close_database_connection();
private:
    void db_error();
    sqlite3_stmt *db_prepare(const char *sql, ...);
};

#endif /* _DATABASE_READER_H_ */
