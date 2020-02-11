#ifndef ANALYSIS_
#define ANALYSIS_

#include <log/log.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

// foward declaration to struct chart in chart/chart.h
struct chart;

/**
 * Adds a security to be analyzed.
 */
void analysis_add(struct chart* sec);

/**
 * Initalizes threads to perform analysis on
 */
void analysis_init();

/**
 * Pushes information to a thread to analyize it
 */
void analysis_push(struct chart* sec, size_t start, size_t end);

#endif
