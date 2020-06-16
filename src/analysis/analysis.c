#include <analysis/analysis.h>
#include <sched.h>
#include <stdatomic.h>

/*
 * The analysis info struct is the value of the linked list
 * created by analysis_list.
 *
 * @param {struct chart*} cht A candle stick chart
 * @param {size_t} start_candle The start analysis candle
 * @param {size_t} end_candle The end analysis candle
 * @param {struct analysis_info* | NULL} next The next element in the list
 * @param {struct analysis_info* | NULL} prev The prev element in the list
 */
struct analysis_info {
  struct chart *cht;
  size_t start_candle;
  size_t end_candle;
  struct analysis_info *next;
  struct analysis_info *prev;
};

/*
 * An overarching structure containing meta data for the linked list
 * @param {struct analysis_info*} head The head of the linked list
 * @param {struct analysis_info*} tail The tail of the linked list
 * @param {pthread_mutex_t} can_remove A mutex locking head and tail
 *  from being written and read at the same time.
 * @param {size_t} num_elements The number of elements in the list
 */
struct analysis_list {
  struct analysis_info *head;
  struct analysis_info *tail;
  pthread_mutex_t can_remove;
  int _Atomic num_elements;

  // 4 extra usable bytes in this structure
  char _p1[4];
};

/*
 * A list of loaded vtables
 */
struct analysis_functions {
  size_t num_functions;
  void **handles;
  struct vtable **funs;
};

/*
 * Loaded functions
 */
static struct analysis_functions loaded_funs = {0, NULL, NULL};

/*
 * Keeps track of which bin the next request will go into.
 * This is incremented everytime analysis_push is called and
 * modded (%) by num_analysis_threads
 */
static long current_analysis_index = 0;

/*
 * The number of availibale threads that can work. If the number of
 * threads > 2, then two are taken away for the web browser to display
 * the chart.
 *
 * TODO: Add compile time option to not reduce the number threads for
 * analysis threads. This should only be turned on if chart display
 * is being offloaded to different machine.
 */
static long num_analysis_threads = 0;

/*
 * A list of struct analysis_list* of length num_analysis_threads.
 * Each thread has its own bin that it pops. This ensures equal
 * working amung all the threads.
 */
static struct analysis_list **thread_operations;

/*
 * A wait check must be set before starting the analysis workflow.
 * Analysis is haulted until init_completed is set to true in
 * which case pushing and popping to the analysis_list can occure
 */
static bool init_completed = false;

/*
 * A list of pthreads that are performing analysis
 */
static pthread_t *threads;

// Set to 1 if the threads need to be joined
int ANALYSIS_INTERRUPED = 0;

static void *analysis_thread_func(void *index) {
  // wait for the sync

  // get the assigned bin id of this analysis thread
  long assigned_bin = *((long *)index);
  free(index);

  // this thread will only monitor this bin of analysis requests
  struct analysis_list *bin = thread_operations[assigned_bin];
  // logger_info("thread was assigned bin id %lu %p", assigned_bin, bin);
  logger_info(__func__, FILENAME_SHORT, __LINE__, "assigned thread bin #%d",
              assigned_bin);

  while (ANALYSIS_INTERRUPED == 0) {
    int ne = atomic_load_explicit(&bin->num_elements, memory_order_seq_cst);
    if (ne == 0) {
      sched_yield();
      continue;
    }

    // get the next analysis in the queue
    struct analysis_info *inf = NULL;
    pthread_mutex_lock(&(bin->can_remove));
    analysis_pop(bin, &inf);
    pthread_mutex_unlock(&(bin->can_remove));

    // analysis_pop returns NULL if there is nothing to do
    // if (!inf) {
    // wait a few seconds for data to populate
    //  continue;
    //}
    struct chart *cht = inf->cht;

    size_t start_index = inf->start_candle;
    (void)start_index;

    size_t end_candle = inf->end_candle;

    // aquire the analysis struct first
    // chart_analysis_lock(cht);

    // group the analysis into sections from simplest to hardest

    // loop through each function
    for (size_t i = 0; i < loaded_funs.num_functions; ++i) {
      clock_t begin = clock();
      TRACE_HAULT(loaded_funs.funs[i]->run(cht, end_candle));
      clock_t end = clock();
      long ts = end - begin;
      TRACE_HAULT(logger_info(
          __func__, FILENAME_SHORT, __LINE__, "[TIMIT] %s@%d => %d/cycles",
          loaded_funs.funs[i]->get_name(), assigned_bin, ts));
    }

    // release the analysis struct lock
    // chart_analysis_unlock(cht);
    free(inf);
  }
  return NULL;
}

static enum RISKI_ERROR_CODE analysis_load() {
  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir("./analysis/")) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir(dir)) != NULL) {
      printf("%s\n", ent->d_name);
      if (ent->d_type == 8) // file
      {
        TRACE(logger_info(__func__, FILENAME_SHORT, __LINE__,
                          "trying to load library %s", ent->d_name));

        void *handle;
        char *loc = NULL;
        loc = (char *)malloc((12 + strlen(ent->d_name)) * sizeof(char));
        strcpy(loc, "./analysis/");
        strcat(loc, ent->d_name);
        handle = dlopen(loc, RTLD_NOW);

        if (!handle) {
          printf("handle bad %s\n", loc);
          fprintf(stderr, "dlopen failed: %s\n", dlerror());
          exit(1);
        }

        struct vtable *dyn;
        dyn = (struct vtable *)dlsym(handle, "exports");
        if (!dyn) {
          fprintf(stderr, "dlopen failed: %s\n", dlerror());
          exit(1);
        }

        TRACE(logger_analysis("N/A", dyn->get_name(), __func__, FILENAME_SHORT,
                              __LINE__, "Loaded %s by %s", dyn->get_name(),
                              dyn->get_author()));

        loaded_funs.num_functions += 1;
        printf("%lu", loaded_funs.num_functions);
        loaded_funs.funs = (struct vtable **)realloc(
            loaded_funs.funs,
            sizeof(struct vtable **) * loaded_funs.num_functions);
        loaded_funs.funs[loaded_funs.num_functions - 1] = dyn;

        loaded_funs.handles = (void **)realloc(
            loaded_funs.handles, sizeof(void *) * loaded_funs.num_functions);

        loaded_funs.handles[loaded_funs.num_functions - 1] = handle;

        free(loc);
      }
    }
    closedir(dir);
  } else {
    /* could not open directory */
    return RISKI_ERROR_CODE_UNKNOWN;
  }

  logger_info(__func__, FILENAME_SHORT, __LINE__, "loaded %lu analysis",
              loaded_funs.num_functions);

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE analysis_init() {

  TRACE(analysis_load());

  long numCPU = sysconf(_SC_NPROCESSORS_ONLN);

  // log_info("processor has %d processing threads", numCPU);
  TRACE(logger_info(__func__, FILENAME_SHORT, __LINE__,
                    "machine has %d threads", numCPU));
  // log_trace("creating %d processing units", numCPU);

  num_analysis_threads = numCPU;

  if (numCPU - 3 > 0) {
    num_analysis_threads = numCPU - 3;
  } else {
    logger_warning(__func__, FILENAME_SHORT, __LINE__,
                   "machine has less than 4 cores");
  }

  TRACE(logger_info(__func__, FILENAME_SHORT, __LINE__,
                    "creating %d analysis threads", numCPU));

  // create the list container
  thread_operations = (struct analysis_list **)malloc(
      (uint64_t)num_analysis_threads * sizeof(struct analysis_list *));

  // make sure malloc was correct
  PTR_CHECK(thread_operations, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  // create the lists
  for (long i = 0; i < num_analysis_threads; ++i) {
    thread_operations[i] =
        (struct analysis_list *)malloc(1 * sizeof(struct analysis_list));

    // make sure malloc was correct
    PTR_CHECK(thread_operations, RISKI_ERROR_CODE_MALLOC_ERROR,
              RISKI_ERROR_TEXT);

    pthread_mutex_init(&(thread_operations[i]->can_remove), NULL);

    thread_operations[i]->head = NULL;
    thread_operations[i]->tail = NULL;

    atomic_store_explicit(&thread_operations[i]->num_elements, 0,
                          memory_order_seq_cst);
  }

  threads =
      (pthread_t *)calloc((uint64_t)num_analysis_threads, sizeof(pthread_t));

  PTR_CHECK(threads, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  for (long i = 0; i < num_analysis_threads; ++i) {
    long *id = (long *)malloc(1 * sizeof(long));

    PTR_CHECK(id, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

    *id = i;
    pthread_create(&(threads[i]), NULL, analysis_thread_func, (void *)id);
  }
  init_completed = true;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE analysis_create_info(struct chart *cht, size_t start,
                                           size_t end,
                                           struct analysis_info **inf) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(inf, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct analysis_info *element =
      (struct analysis_info *)malloc(1 * sizeof(struct analysis_info));

  PTR_CHECK(element, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  element->cht = cht;
  element->start_candle = start;
  element->end_candle = end;

  element->next = NULL;
  element->prev = NULL;

  *inf = element;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE analysis_push(struct chart *cht, size_t start,
                                    size_t end) {
  while (!init_completed)
    ;

  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(thread_operations, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // what bin the analysis will go into
  long thread_bin = current_analysis_index % num_analysis_threads;
  // get the list

  struct analysis_list *bin = thread_operations[thread_bin];

  // create the new element
  struct analysis_info *element = NULL;

  // Create a new info struct
  TRACE(analysis_create_info(cht, start, end, &element));

  // Make sure the info object was set correctly
  PTR_CHECK(element, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // The number of elements
  int ne = 0;

  // aquire lock to make sure no removing occurs
  pthread_mutex_lock(&(bin->can_remove));

  // we will always push to the end of the bin
  if (bin->tail == NULL && bin->head == NULL) {
    // no elements in the list (easy case)
    bin->head = element;
    bin->tail = element;
    // bin->num_elements = 1;
    atomic_store_explicit(&bin->num_elements, 1, memory_order_seq_cst);
    ne = 1;
    pthread_mutex_unlock(&(bin->can_remove));
  } else {
    // insert into the end of the dll
    // get tail
    struct analysis_info *tail = bin->tail;

    // add to the list
    bin->tail->next = element;
    bin->tail->prev = tail;
    bin->tail = element;
    // bin->num_elements += 1;
    atomic_fetch_add_explicit(&bin->num_elements, 1, memory_order_seq_cst);
    ne = atomic_load_explicit(&bin->num_elements, memory_order_seq_cst);
    pthread_mutex_unlock(&(bin->can_remove));
  }

  if (ne > 5) {
    TRACE(logger_warning(__func__, FILENAME_SHORT, __LINE__,
                         "thread id #%lu has fallen behind by %lu charts",
                         thread_bin, ne));
  }

  current_analysis_index += 1;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE analysis_pop(struct analysis_list *bin,
                                   struct analysis_info **inf) {
  PTR_CHECK(bin, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct analysis_info *element = bin->head;

  // remove the head
  if (bin->head) {
    bin->head = bin->head->next;
    // bin->num_elements -= 1;
    atomic_fetch_sub_explicit(&bin->num_elements, 1, memory_order_seq_cst);
  }

  // if the next element was not null
  // set the previous element to NULL
  if (bin->head) {
    bin->head->prev = NULL;
  } else {
    bin->head = NULL;
    bin->tail = NULL;
  }

  PTR_CHECK(inf, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  *inf = element;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE analysis_cleanup() {
  ANALYSIS_INTERRUPED = 1;
  for (long i = 0; i < num_analysis_threads; ++i) {
    pthread_join(threads[i], NULL);
  }
  for (long i = 0; i < num_analysis_threads; ++i) {
    free(thread_operations[i]);
  }
  free(thread_operations);
  free(threads);
  free(loaded_funs.funs);

  for (size_t i = 0; i < loaded_funs.num_functions; ++i) {
    dlclose(loaded_funs.handles[i]);
  }
  free(loaded_funs.handles);

  return RISKI_ERROR_CODE_NONE;
}
