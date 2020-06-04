#ifndef STRING_BUILDER_
#define STRING_BUILDER_

#include <error_codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tracer.h>
/*
 * Private string builder struct
 */
struct string_builder;

/*
 * Creates a new string builder to work with
 * @return {struct string_builder*} A string builder object
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE string_builder_new(struct string_builder **sb);

/*
 * Appends a null terminated c string to the data inside
 * of a string builder.
 * @param {struct string_builder*} sb The string builder
 * @param {char*} c The text to append
 * @param {size_t} n The length of c
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE string_builder_append(struct string_builder *sb, char *c);

/*
 * Gets the string built by the string builder. The *c pointer is
 * set to the value of the string.
 * @param {struct string_builder*} sb The string builder
 * @param {char**} c A pointer to a char* array
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE string_builder_str(struct string_builder *sb, char **c);

/*
 * Frees a string builder object including the string it represents
 * @param {struct string_builder*} sb The string builder to free
 * @return {enum RISKI_ERROR_CODE} The status
 */
enum RISKI_ERROR_CODE string_builder_free(struct string_builder **sb);

#endif
