#include "chart/candle.h"
#include "log/log.h"
#include <chart/chart.h>
#include <pthread.h>

// The maximum number of candles any given chart has
size_t g_max_candles = 0;

struct chart {
  // the interval between the two candles in nanoseconds
  uint64_t interval; 

  // the number of candles _slots_ that are pre allocated
  size_t num_candles_allocated;

  // the current candle
  size_t cur_candle;

  // the start of the last candle
  uint64_t last_update;

  // the list of candles 
  struct candle** candles;

};

struct chart* chart_new(uint64_t interval) {

  struct chart* cht = (struct chart*) malloc(1*sizeof(struct chart));
  cht->interval = interval;
  cht->num_candles_allocated = 1440;
  cht->cur_candle = 0;
  cht->last_update = 0;
  cht->candles = (struct candle**) malloc(1440*sizeof(struct chart*));

  return cht;
}

inline void chart_new_candle(struct chart* cht, int64_t price) {
  cht->candles[cht->cur_candle] = candle_new(price, cht->last_update);
}

void chart_update(struct chart* cht, int64_t price, uint64_t ts) {

  // special case where this is the first chart update
  if (cht->last_update == 0) {
    cht->last_update = ts;
    // check if we need to pre fill the data to make sure
    // all charts are on the same candle index
    for (size_t i = 0; i < g_max_candles; ++i) {
      chart_new_candle(cht, price);
      cht->cur_candle += 1;
    }
    cht->last_update = ts;
    chart_new_candle(cht, price);
    return;
  }

  // check if the interval requires us to make a new candle
  if (ts - cht->last_update > cht->interval) {
    // create a new candle
    
    // check if fill-ins are reqired
    size_t fill_in_candles = (ts - cht->last_update) / cht->interval; 

    if (fill_in_candles != 1) {
      for (size_t i = 0; i < fill_in_candles-1; ++i) {
        // fill in the candles in between with dojies of the
        // current candle before creating the new candle
        cht->cur_candle += 1;
        cht->last_update += cht->interval;
        chart_new_candle(cht, candle_close(cht->candles[cht->cur_candle-1]));
        candle_update(cht->candles[cht->cur_candle], 
          candle_close(cht->candles[cht->cur_candle]),
          cht->last_update + cht->interval); 
      } 
    }

    cht->last_update = ts;
    cht->cur_candle += 1;
    chart_new_candle(cht, price); 
  } else {
    // update the current candle
    candle_update(cht->candles[cht->cur_candle], price, ts); 
  }

  if (cht->cur_candle > g_max_candles)
    g_max_candles = cht->cur_candle;
}

char* chart_json(struct chart* cht) {

  // if the last update is 0 then there is no chart
  // information so we can't construct a valid json
  if (cht->last_update == 0)
    return NULL;

  size_t num_candles = cht->cur_candle + 1;
  // chart proto = {"chart":[c1,c2,c3,c4...]}
 
  // allocate enough space to hold each candle 
  size_t total_json_size = JSON_CANDLE_MAX_LEN * num_candles; 

  // add (num_candles-1) commas
  total_json_size += (num_candles-1);

  // add room for {"chart":[]}
  total_json_size += 12;

  // add room for null character
  total_json_size += 1;

  char* buf = (char*) calloc(total_json_size, sizeof(char));
  strcat(buf, "{\"chart\":[\x0");
  
  char* tmp_candle_json = NULL;
  for (size_t i = 0; i < num_candles; ++i) {
    tmp_candle_json = candle_json(cht->candles[i]);
    strcat(buf, tmp_candle_json);
    if (i != num_candles-1)
      strcat(buf, ",\x0");

    free(tmp_candle_json);
    tmp_candle_json = NULL;
  }
  strcat(buf, "]}\x0");
  
  return buf;
}

char* chart_latest_candle(struct chart* cht) {
  // {"latest_candle":}
  

  size_t total_json_size = 18 + JSON_CANDLE_MAX_LEN;
  char* buf = (char*) calloc(total_json_size, sizeof(char));
  strcat(buf, "{\"latest_candle\":\x0");

  char* tmp_candle_json = NULL;
  tmp_candle_json = candle_json(cht->candles[cht->cur_candle]);
  strcat(buf, tmp_candle_json);
  strcat(buf, "}\x0");

  free(tmp_candle_json);
  return buf;
}

void chart_free(struct chart** cht) {
  if ((*cht)->last_update != 0) {
    for (size_t i = 0; i < ((*cht)->cur_candle+1); ++i) {
      candle_free(&((*cht)->candles[i]));
    }
  }
  free((*cht)->candles);
  free(*cht);
  *cht = NULL;
}

void test_chart() {

  struct chart* cht = chart_new(10);
  ASSERT_TEST(cht != NULL);
  ASSERT_TEST(cht->interval == 10);
  ASSERT_TEST(cht->num_candles_allocated == 1440);
  ASSERT_TEST(cht->cur_candle == 0);
  ASSERT_TEST(cht->candles != NULL);
  ASSERT_TEST(cht->last_update == 0);

  // make sure chart json returns null since there is no information 
  ASSERT_TEST(chart_json(cht) == NULL);

  // send an update to the chart
  chart_update(cht, 10000, 1);
  chart_update(cht, 20000, 9);

  ASSERT_TEST(cht->last_update == 1);

  // single the candle tests pass make sure there is still 1 candle
  ASSERT_TEST(cht->cur_candle == 0);

  // make sure we add a new candle
  chart_update(cht, 30000, 12);
  ASSERT_TEST(cht->cur_candle == 1);
  ASSERT_TEST(cht->last_update == 12);

  char* tmp_chart_buf = chart_json(cht);
  ASSERT_TEST(strcmp(tmp_chart_buf,
        "{\"chart\":["
        "{\"candle\":"
        "{\"o\":10000,\"h\":20000,\"l\":10000,\"c\":20000,\"s\":1,\"e\":9}},"
        "{\"candle\":{\"o\":30000,\"h\":30000,\"l\":30000,\"c\":30000,\"s\":12"
        ",\"e\":12}}]}") == 0);

  chart_free(&cht);
  ASSERT_TEST(cht == NULL);

  free(tmp_chart_buf);
}
