/*
 * databasereader.h
 *
 *  Created on: 2 Dec 2014
 *      Author: alan
 */
#include <map>
#include <sqlite3.h>
#include "colour.h"

#ifndef DATABASEREADER_H_
#define DATABASEREADER_H_

class DatabaseReader {
	sqlite3 *db;
public:
	DatabaseReader(char*);
	virtual ~DatabaseReader();
	std::map<int, char*> read_database_for_labels();
	std::map<int, int> read_database_for_keys();
	std::map<int, struct colour> read_color_map(char*);
	void close_database_connection();
private:
	static int callback(void *, int, char **, char **);
};

#endif /* DATABASEREADER_H_ */
