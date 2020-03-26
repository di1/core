#ifndef SEARCH_
#define SEARCH_

// std
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// string builder
#include <string_builder.h>

// stack tracer
#include <error_codes.h>
#include <tracer.h>

/*
 * Must be called before trying to search. Will
 * load all data in the csv file
 * @param {char*} csv_db The database to read from
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE search_init(char* csv_db);

/*
 * Given a limmited character set find the best 10 matches.
 * @param {char*} seq The sequence to search for
 * @param {char**} json Will set *json to the best 10 matches
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE search_search(char* seq, char** json);

/*
 * Frees the serach interface
 */
enum RISKI_ERROR_CODE search_free();

#endif
