#include <analysis/analysis.h>
#include <chart/chart.h>
#include <unistd.h>

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
  pthread_mutex_t can_append;
  pthread_mutex_t can_remove;

  size_t num_elements;

};

// fwd declaration of analysis_pop
struct analysis_info* analysis_pop(struct analysis_list* bin);

size_t current_analysis_index = 0;
size_t num_analysis_threads = 0;

// Holds num_analysis_threads analysis_lists
struct analysis_list** thread_operations;

pthread_t* threads;

bool init_completed = false;

void* analysis_thread_func(void* index) {

  size_t assigned_bin =  *((size_t*)index);
  free(index);

  struct analysis_list* bin = thread_operations[assigned_bin];
  log_trace("thread was assigned bin id %lu %p", assigned_bin, bin);
  while (true) {
    pthread_mutex_lock(&(bin->can_remove));
    struct analysis_info* inf = analysis_pop(bin);
    pthread_mutex_unlock(&(bin->can_remove));
    
    free(inf);
  }

}


void analysis_init() {
  int numCPU = sysconf(_SC_NPROCESSORS_ONLN);
  log_info("processor has %d processing threads", numCPU);
  log_trace("creating %d processing units", numCPU);
  num_analysis_threads = numCPU;

  // create the list container
  thread_operations = (struct analysis_list**) 
    malloc(num_analysis_threads * sizeof(struct analysis_list*));

  // create the lists 
  for (size_t i = 0; i < num_analysis_threads; ++i) {
    thread_operations[i] = (struct analysis_list*)
      malloc(1 * sizeof(struct analysis_list));

    pthread_mutex_init(&(thread_operations[i]->can_remove), NULL);
    pthread_mutex_init(&(thread_operations[i]->can_append), NULL);

    thread_operations[i]->head = NULL;
    thread_operations[i]->tail = NULL;

    thread_operations[i]->num_elements = 0;
  }

  threads = (pthread_t*) calloc(num_analysis_threads, sizeof(pthread_t));
  for (size_t i = 0; i < num_analysis_threads; ++i) {
    size_t* id = (size_t*) malloc(1 * sizeof(size_t));
    *id = i;
    pthread_create(&(threads[i]), NULL, analysis_thread_func, (void*) id);
  }
  init_completed = true;
}

struct analysis_info* analysis_create_info(struct chart* cht, size_t start,
    size_t end) {
  struct analysis_info* element =
   (struct analysis_info*) malloc(1 * sizeof(struct analysis_info));

  element->cht = cht;
  element->start_candle = start;
  element->end_candle = end;

  element->next = NULL;
  element->prev = NULL;

  return element;

}

void analysis_push(struct chart* cht, size_t start, size_t end) {
 
  while (!init_completed);

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
    log_warn("thread id %lu analysis is %lu candles behind",
        thread_bin, ne);
  }

  current_analysis_index += 1;
}

struct analysis_info* analysis_pop(struct analysis_list* bin) {
  // make sure there is no appending

  // make sure no appending can happen

  struct analysis_info* element = bin->head;

  //log_trace("%p", bin->head);
  
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
