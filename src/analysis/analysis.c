#include <analysis/analysis.h>
#include <analysis/doji.h>
#include <analysis/horizontal_line.h>
#include <analysis/marubozu.h>
#include <analysis/spinning_top.h>
#include <chart/candle.h>
#include <chart/chart.h>

struct analysis_info {
  // chart to analyize
  struct chart* cht;

  // where to start in the chart
  size_t start_candle;

  // where to end in the chart
  size_t end_candle;

  // the next element
  struct analysis_info* next;

  // the previous element
  struct analysis_info* prev;
};

struct analysis_list {
  // the head of the list
  struct analysis_info* head;

  // the tail of the list
  struct analysis_info* tail;

  // Need two mutexes to stop race conditions
  // where 1 thread tries to append and 1 thread
  // tries to remove
  pthread_mutex_t can_remove;

  size_t num_elements;
};

// fwd declaration of analysis_pop
struct analysis_info* analysis_pop(struct analysis_list* bin);

// Keeps track of which bin the next request will go into
size_t current_analysis_index = 0;
size_t num_analysis_threads = 0;

// Holds num_analysis_threads analysis_lists
struct analysis_list** thread_operations;

// Set to true once all the threads have been created
bool init_completed = false;

// The list of analysis threads
pthread_t* threads;

#define SINGLE_CANDLE_PATTERN_PERFORM(ANALYSIS_FUNCTION)         \
  do {                                                           \
    ret = ANALYSIS_FUNCTION(last_candle);                        \
    if (ret != SINGLE_CANDLE_PATTERN_NONE) {                     \
      chart_put_single_candle_pattern(cht, end_candle - 1, ret); \
      return;                                                    \
    }                                                            \
  } while (0);

/**
 * Performs simple candle stick analysis on the chart
 * because this only requires the most recent candle
 * we only need to perform analysis on the last candle.
 *
 * Analysis done in this thread only relies on the end_candle
 * index, which since is 1 indexed will be end_candle-1 in the
 * array.
 */
void simple_analysis(struct chart* cht, size_t end_candle) {
  struct candle* last_candle = chart_get_candle(cht, end_candle - 1);

  if (!last_candle) {
    log_error("%lu end_candle", end_candle);
    exit(1);
  }

  // Make sure this is a valid candle, fill in candles will have the same
  // start and end time
  if (candle_volume(last_candle) == 0) return;

  // Now that we have a valid candle perform all single candle
  // analysis

  // return value of each single analysis
  enum SINGLE_CANDLE_PATTERNS ret;

  SINGLE_CANDLE_PATTERN_PERFORM(is_white_marubozu);
  SINGLE_CANDLE_PATTERN_PERFORM(is_black_marubozu);
  SINGLE_CANDLE_PATTERN_PERFORM(is_white_spinning_top);
  SINGLE_CANDLE_PATTERN_PERFORM(is_black_spinning_top);
  SINGLE_CANDLE_PATTERN_PERFORM(is_doji_dragonfly);
  SINGLE_CANDLE_PATTERN_PERFORM(is_doji_gravestone);
  SINGLE_CANDLE_PATTERN_PERFORM(is_doji_generic);
}

void* analysis_thread_func(void* index) {
  // wait for the sync

  // get the assigned bin id of this analysis thread
  size_t assigned_bin = *((size_t*)index);
  free(index);

  // this thread will only monitor this bin of analysis requests
  struct analysis_list* bin = thread_operations[assigned_bin];
  log_trace("thread was assigned bin id %lu %p", assigned_bin, bin);

  while (true) {
    // get the next analysis in the queue
    pthread_mutex_lock(&(bin->can_remove));
    struct analysis_info* inf = analysis_pop(bin);
    pthread_mutex_unlock(&(bin->can_remove));

    // analysis_pop returns NULL if there is nothing to do
    if (!inf) continue;

    struct chart* cht = inf->cht;

    size_t start_index = inf->start_candle;
    (void)start_index;

    size_t end_candle = inf->end_candle;

    // aquire the analysis struct first
    chart_analysis_lock(cht);

    // group the analysis into sections from simplest to hardest
    simple_analysis(cht, end_candle);
    find_horizontal_line(cht, end_candle);

    // release the analysis struct lock
    chart_analysis_unlock(cht);
    free(inf);
  }
}

void analysis_init() {
  int numCPU = sysconf(_SC_NPROCESSORS_ONLN);
  log_info("processor has %d processing threads", numCPU);
  log_trace("creating %d processing units", numCPU);

  num_analysis_threads = numCPU;

  if (numCPU - 3 > 0) {
    num_analysis_threads = numCPU - 3;
  }

  log_trace("creating %d processing units", numCPU);

  // create the list container
  thread_operations = (struct analysis_list**)malloc(
      num_analysis_threads * sizeof(struct analysis_list*));

  // create the lists
  for (size_t i = 0; i < num_analysis_threads; ++i) {
    thread_operations[i] =
        (struct analysis_list*)malloc(1 * sizeof(struct analysis_list));

    pthread_mutex_init(&(thread_operations[i]->can_remove), NULL);

    thread_operations[i]->head = NULL;
    thread_operations[i]->tail = NULL;

    thread_operations[i]->num_elements = 0;
  }

  threads = (pthread_t*)calloc(num_analysis_threads, sizeof(pthread_t));
  for (size_t i = 0; i < num_analysis_threads; ++i) {
    size_t* id = (size_t*)malloc(1 * sizeof(size_t));
    *id = i;
    pthread_create(&(threads[i]), NULL, analysis_thread_func, (void*)id);
  }
  init_completed = true;
}

struct analysis_info* analysis_create_info(struct chart* cht, size_t start,
                                           size_t end) {
  struct analysis_info* element =
      (struct analysis_info*)malloc(1 * sizeof(struct analysis_info));

  element->cht = cht;
  element->start_candle = start;
  element->end_candle = end;

  element->next = NULL;
  element->prev = NULL;

  return element;
}

void analysis_push(struct chart* cht, size_t start, size_t end) {
  while (!init_completed)
    ;

  // what bin the analysis will go into
  size_t thread_bin = current_analysis_index % num_analysis_threads;
  // get the list
  struct analysis_list* bin = thread_operations[thread_bin];

  // create the new element
  struct analysis_info* element = analysis_create_info(cht, start, end);

  size_t ne = 0;

  // aquire lock to make sure no removing occurs
  pthread_mutex_lock(&(bin->can_remove));

  // we will always push to the end of the bin
  if (bin->tail == NULL && bin->head == NULL) {
    // no elements in the list (easy case)
    bin->head = element;
    bin->tail = element;
    bin->num_elements = 1;
    ne = bin->num_elements;
    pthread_mutex_unlock(&(bin->can_remove));
  } else {
    // insert into the end of the dll
    // get tail
    struct analysis_info* tail = bin->tail;

    // add to the list
    bin->tail->next = element;
    bin->tail->prev = tail;
    bin->tail = element;
    bin->num_elements += 1;
    ne = bin->num_elements;
    pthread_mutex_unlock(&(bin->can_remove));
  }

  if (ne > 5) {
    log_warn("thread id %lu analysis is %lu charts behind", thread_bin, ne);
  }

  current_analysis_index += 1;
}

struct analysis_info* analysis_pop(struct analysis_list* bin) {
  // make sure there is no appending

  // make sure no appending can happen

  struct analysis_info* element = bin->head;

  // log_trace("%p", bin->head);

  // remove the head
  if (bin->head) {
    bin->head = bin->head->next;
    bin->num_elements -= 1;
  }

  // if the next element was not null
  // set the previous element to NULL
  if (bin->head) {
    bin->head->prev = NULL;
  } else {
    bin->head = NULL;
    bin->tail = NULL;
  }

  return element;
}
