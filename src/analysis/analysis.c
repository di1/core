#include "api.h"
#include <analysis/analysis.h>
#include <dlfcn.h>
#include <time.h>

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
struct analysis_info
{
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
struct analysis_list
{
  struct analysis_info *head;
  struct analysis_info *tail;
  pthread_mutex_t can_remove;
  size_t _Atomic num_elements;
};

/*
 * A list of loaded vtables
 */
struct analysis_functions
{
  size_t num_functions;
  struct vtable **funs;
};

/*
 * Loaded functions
 */
struct analysis_functions loaded_funs = { 0, NULL };

/*
 * Keeps track of which bin the next request will go into.
 * This is incremented everytime analysis_push is called and
 * modded (%) by num_analysis_threads
 */
size_t current_analysis_index = 0;

/*
 * The number of availibale threads that can work. If the number of
 * threads > 2, then two are taken away for the web browser to display
 * the chart.
 *
 * TODO: Add compile time option to not reduce the number threads for
 * analysis threads. This should only be turned on if chart display
 * is being offloaded to different machine.
 */
size_t num_analysis_threads = 0;

/*
 * A list of struct analysis_list* of length num_analysis_threads.
 * Each thread has its own bin that it pops. This ensures equal
 * working amung all the threads.
 */
struct analysis_list **thread_operations;

/*
 * A wait check must be set before starting the analysis workflow.
 * Analysis is haulted until init_completed is set to true in
 * which case pushing and popping to the analysis_list can occure
 */
bool init_completed = false;

/*
 * A list of pthreads that are performing analysis
 */
pthread_t *threads;

#define SINGLE_CANDLE_PATTERN_PERFORM(ANALYSIS_FUNCTION)                      \
  do                                                                          \
    {                                                                         \
      enum SINGLE_CANDLE_PATTERNS res = SINGLE_CANDLE_PATTERN_NONE;           \
      TRACE (ANALYSIS_FUNCTION (last_candle, &res));                          \
      TRACE (chart_put_single_candle_pattern (cht, end_candle - 1, res));     \
      if (res != SINGLE_CANDLE_PATTERN_NONE)                                  \
        {                                                                     \
          return RISKI_ERROR_CODE_NONE;                                       \
        }                                                                     \
    }                                                                         \
  while (0)

#define DOUBLE_CANDLE_PATTERN_PERFORM(ANALYSIS_FUNCTION)                      \
  do                                                                          \
    {                                                                         \
      enum DOUBLE_CANDLE_PATTERNS res = DOUBLE_CANDLE_PATTERNS_NONE;          \
      TRACE (ANALYSIS_FUNCTION (end_candle - 1, cht, &res));                  \
      TRACE (chart_put_double_candle_pattern (cht, end_candle - 1, res));     \
      if (res != DOUBLE_CANDLE_PATTERNS_NONE)                                 \
        {                                                                     \
          return RISKI_ERROR_CODE_NONE;                                       \
        }                                                                     \
    }                                                                         \
  while (0)

enum RISKI_ERROR_CODE
simple_analysis (struct chart *cht, size_t end_candle)
{
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct candle *last_candle = NULL;
  TRACE (chart_get_candle (cht, end_candle - 1, &last_candle));

  // Make sure this is a valid candle, fill in candles will have the same
  // start and end time
  uint64_t last_candle_volume = 0;
  TRACE (candle_volume (last_candle, &last_candle_volume));
  if (last_candle_volume == 0)
    return RISKI_ERROR_CODE_NONE;

  // Now that we have a valid candle perform all single candle
  // analysis

  return RISKI_ERROR_CODE_NONE;
}

// Set to 1 if the threads need to be joined
int ANALYSIS_INTERRUPED = 0;

void *
analysis_thread_func (void *index)
{
  // wait for the sync

  // get the assigned bin id of this analysis thread
  size_t assigned_bin = *((size_t *)index);
  free (index);

  // this thread will only monitor this bin of analysis requests
  struct analysis_list *bin = thread_operations[assigned_bin];
  // logger_info("thread was assigned bin id %lu %p", assigned_bin, bin);
  logger_info (__func__, __FILENAME__, __LINE__, "assigned thread bin #%d",
               assigned_bin);

  while (ANALYSIS_INTERRUPED == 0)
    {
      if (bin->num_elements == 0)
        {
          sleep (0);
          continue;
        }

      // get the next analysis in the queue
      struct analysis_info *inf = NULL;
      pthread_mutex_lock (&(bin->can_remove));
      analysis_pop (bin, &inf);
      pthread_mutex_unlock (&(bin->can_remove));

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
      chart_analysis_lock (cht);

      // group the analysis into sections from simplest to hardest

      // loop through each function
      for (size_t i = 0; i < loaded_funs.num_functions; ++i)
        {
          TRACE_HAULT(loaded_funs.funs[i]->run (cht, end_candle));
        }

      // release the analysis struct lock
      chart_analysis_unlock (cht);
      free (inf);
    }
  return NULL;
}

enum RISKI_ERROR_CODE
analysis_load ()
{
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir ("./analysis/")) != NULL)
    {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL)
        {
          if (ent->d_type == 8) // file
            {
              TRACE (logger_info (__func__, __FILENAME__, __LINE__,
                                  "trying to load library %s", ent->d_name));

              void *handle;
              char *loc = NULL;
              loc = (char *)malloc ((12 + strlen (ent->d_name))
                                    * sizeof (char));
              strcpy (loc, "./analysis/");
              strcat (loc, ent->d_name);
              handle = dlopen (loc, RTLD_NOW);

              if (!handle)
                {
                  printf ("handle bad %s\n", loc);
                  fprintf (stderr, "dlopen failed: %s\n", dlerror ());
                  exit (1);
                }

              struct vtable *dyn;
              dyn = (struct vtable *)dlsym (handle, "exports");
              if (!dyn)
                {
                  fprintf (stderr, "dlopen failed: %s\n", dlerror ());
                  exit (1);
                }

              TRACE(logger_analysis("N/A", dyn->get_name(), __func__,
                  __FILENAME__, __LINE__, "Loaded %s by %s", 
                  dyn->get_name(), dyn->get_author()));

              loaded_funs.num_functions += 1;
              loaded_funs.funs = (struct vtable **)realloc (
                  loaded_funs.funs,
                  sizeof (struct vtable **) * loaded_funs.num_functions);
              loaded_funs.funs[loaded_funs.num_functions-1] = dyn;
            }
        }
      closedir (dir);
    }
  else
    {
      /* could not open directory */
      return RISKI_ERROR_CODE_UNKNOWN;
    }

  logger_info (__func__, __FILENAME__, __LINE__, "loaded %lu analysis",
               loaded_funs.funs);

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
analysis_init ()
{

  TRACE (analysis_load ());

  int numCPU = sysconf (_SC_NPROCESSORS_ONLN);
  // log_info("processor has %d processing threads", numCPU);
  TRACE (logger_info (__func__, __FILENAME__, __LINE__,
                      "machine has %d threads", numCPU));
  // log_trace("creating %d processing units", numCPU);

  num_analysis_threads = numCPU;

  if (numCPU - 3 > 0)
    {
      num_analysis_threads = numCPU - 3;
    }
  else
    {
      logger_warning (__func__, __FILENAME__, __LINE__,
                      "machine has less than 4 cores");
    }

  TRACE (logger_info (__func__, __FILENAME__, __LINE__,
                      "creating %d analysis threads", numCPU));

  // create the list container
  thread_operations = (struct analysis_list **)malloc (
      num_analysis_threads * sizeof (struct analysis_list *));

  // make sure malloc was correct
  PTR_CHECK (thread_operations, RISKI_ERROR_CODE_MALLOC_ERROR,
             RISKI_ERROR_TEXT);

  // create the lists
  for (size_t i = 0; i < num_analysis_threads; ++i)
    {
      thread_operations[i]
          = (struct analysis_list *)malloc (1 * sizeof (struct analysis_list));

      // make sure malloc was correct
      PTR_CHECK (thread_operations, RISKI_ERROR_CODE_MALLOC_ERROR,
                 RISKI_ERROR_TEXT);

      pthread_mutex_init (&(thread_operations[i]->can_remove), NULL);

      thread_operations[i]->head = NULL;
      thread_operations[i]->tail = NULL;

      thread_operations[i]->num_elements = 0;
    }

  threads = (pthread_t *)calloc (num_analysis_threads, sizeof (pthread_t));

  PTR_CHECK (threads, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  for (size_t i = 0; i < num_analysis_threads; ++i)
    {
      size_t *id = (size_t *)malloc (1 * sizeof (size_t));

      PTR_CHECK (id, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

      *id = i;
      pthread_create (&(threads[i]), NULL, analysis_thread_func, (void *)id);
    }
  init_completed = true;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
analysis_create_info (struct chart *cht, size_t start, size_t end,
                      struct analysis_info **inf)
{
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (inf, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct analysis_info *element
      = (struct analysis_info *)malloc (1 * sizeof (struct analysis_info));

  PTR_CHECK (element, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  element->cht = cht;
  element->start_candle = start;
  element->end_candle = end;

  element->next = NULL;
  element->prev = NULL;

  *inf = element;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
analysis_push (struct chart *cht, size_t start, size_t end)
{
  while (!init_completed)
    ;

  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (thread_operations, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // what bin the analysis will go into
  size_t thread_bin = current_analysis_index % num_analysis_threads;
  // get the list

  struct analysis_list *bin = thread_operations[thread_bin];

  // create the new element
  struct analysis_info *element = NULL;

  // Create a new info struct
  TRACE (analysis_create_info (cht, start, end, &element));

  // Make sure the info object was set correctly
  PTR_CHECK (element, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // The number of elements
  size_t ne = 0;

  // aquire lock to make sure no removing occurs
  pthread_mutex_lock (&(bin->can_remove));

  // we will always push to the end of the bin
  if (bin->tail == NULL && bin->head == NULL)
    {
      // no elements in the list (easy case)
      bin->head = element;
      bin->tail = element;
      bin->num_elements = 1;
      ne = bin->num_elements;
      pthread_mutex_unlock (&(bin->can_remove));
    }
  else
    {
      // insert into the end of the dll
      // get tail
      struct analysis_info *tail = bin->tail;

      // add to the list
      bin->tail->next = element;
      bin->tail->prev = tail;
      bin->tail = element;
      bin->num_elements += 1;
      ne = bin->num_elements;
      pthread_mutex_unlock (&(bin->can_remove));
    }

  if (ne > 5)
    {
      TRACE (logger_warning (__func__, __FILENAME__, __LINE__,
                             "thread id #%lu has fallen behind by %lu charts",
                             thread_bin, ne));
    }

  current_analysis_index += 1;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
analysis_pop (struct analysis_list *bin, struct analysis_info **inf)
{
  PTR_CHECK (bin, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct analysis_info *element = bin->head;

  // remove the head
  if (bin->head)
    {
      bin->head = bin->head->next;
      bin->num_elements -= 1;
    }

  // if the next element was not null
  // set the previous element to NULL
  if (bin->head)
    {
      bin->head->prev = NULL;
    }
  else
    {
      bin->head = NULL;
      bin->tail = NULL;
    }

  PTR_CHECK (inf, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  *inf = element;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_analysis_json (struct chart *cht, char **json)
{
  // TODO
  (void)cht;
  (void)json;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
analysis_cleanup ()
{
  ANALYSIS_INTERRUPED = 1;
  for (size_t i = 0; i < num_analysis_threads; ++i)
    {
      pthread_join (threads[i], NULL);
    }
  for (size_t i = 0; i < num_analysis_threads; ++i)
    {
      free (thread_operations[i]);
    }
  free (thread_operations);
  free (threads);
  return RISKI_ERROR_CODE_NONE;
}
