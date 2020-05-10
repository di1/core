#ifndef VM_
#define VM_

#include <tracer.h>
#include <smakel/lex.h>
#include <smakel/ast.h>

/**
 * Loads a program from the file system for
 * execution
 * @param {const char*} The location
 */
enum RISKI_ERROR_CODE vm_load(const char* loc);

#endif
