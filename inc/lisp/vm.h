#ifndef VM_
#define VM_

#include <lisp/ast.h>
#include <lisp/lex.h>
#include <tracer.h>

/**
 * Loads a program from the file system for
 * execution
 * @param {const char*} The location
 */
enum RISKI_ERROR_CODE vm_load(const char* loc);

#endif
