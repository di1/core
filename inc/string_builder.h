#ifndef STRING_BUILDER_
#define STRING_BUILDER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tracer.h>

/*
 * Private string builder struct
 */
struct string_builder;

/*
 * Possible error codes for the string builder
 */
enum STRING_BUILDER_ERROR_CODE {
  STRING_BUILDER_NO_ERROR = 0,
  STRING_BUILDER_MALLOC_FAILURE = 1,
  STRING_BUILDER_INVALID_PTR = 2,
  STRING_BUILDER_STRCAT_ERR = 3
};

/*
 * String representations of the error codes
 */
const char* STRING_BUILDER_ERROR_STR[4] = {
    "STRING_BUILDER_NO_ERROR", "STRING_BUILDER_MALLOC_FAILURE",
    "STRING_BUILDER_INVALID_PTR", "STRING_BUILDER_STRCAT_ERR"};

/*
 * Creates a new string builder to work with
 * @return {struct string_builder*} A string builder object
 * @return {enum STRING_BUILDER_ERROR_CODE} The status
 */
enum STRING_BUILDER_ERROR_CODE string_builder_new(struct string_builder** sb);

/*
 * Appends a null terminated c string to the data inside
 * of a string builder.
 * @param {struct string_builder*} sb The string builder
 * @param {char*} c The text to append
 * @param {size_t} n The length of c
 * @return {enum STRING_BUILDER_ERROR_CODE} The status
 */
enum STRING_BUILDER_ERROR_CODE string_builder_append(struct string_builder* sb,
                                                     char* c, size_t n);

/*
 * Gets the string built by the string builder. The *c pointer is
 * set to the value of the string.
 * @param {struct string_builder*} sb The string builder
 * @param {char**} c A pointer to a char* array
 * @return {enum STRING_BUILDER_ERROR_CODE} The status
 */
enum STRING_BUILDER_ERROR_CODE string_builder_str(struct string_builder* sb,
                                                  char** c);

/*
 * Frees a string builder object including the string it represents
 * @param {struct string_builder*} sb The string builder to free
 * @return {enum STRING_BUILDER_ERROR_CODE} The status
 */
enum STRING_BUILDER_ERROR_CODE string_builder_free(struct string_builder** sb);

#endif
