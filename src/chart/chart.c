#include <chart/chart.h>

const char* CHART_ERROR_CODE_STR[4] = {"CHART_NO_ERROR", "NULL_PTR",
                                       "CHART_MALLOC_ERROR",
                                       "CHART_INVALID_RANGE"};

/*
 * The maximum number of candles any given chart has
 */
size_t g_max_candles = 0;

/*
 * The struct to represent a trend line
 * @param {size_t} start_index The starting candle
 * @param {size_t} end_index The end index candle
 * @param {enum DIRECTION} direction The direction
 */
struct trend_line {
  size_t start_index;
  size_t end_index;
  enum DIRECTION direction;
};

/*
 * A structure to hold the analysis results of a chart
 * @param {pthread_mutex_t} lock Locks the analysis from being read or modified
 * from a different thread not holding the lock.
 * @param {enum SINGLE_CANDLE_PATTERNS*} scp A list the size of
 * num_candles_allocated where each element defines the analysis on that
 * specific candle index in the chart.
 * @param {size_t} num_trend_lines_horizontal The number of horizontal trend
 * lines.
 * @param {struct trend_line*} trend_lines_horizontal A list of trend lines
 * that have a slope of 0.
 * @param {size_t} num_trend_lines_sloped The number of sloped trend lines
 * @param {struct trend_line*} trend_lines_sloped A list of trend lines that
 * have a non 0 slope.
 */
struct chart_analysis {
  pthread_mutex_t lock;
  enum SINGLE_CANDLE_PATTERNS* scp;
  size_t num_trend_lines_horizontal;
  struct trend_line* trend_lines_horizontal;
  size_t num_trend_lines_sloped;
  struct trend_line* trend_lines_sloped;
};

/*
 * A struct representing the chart
 * @param {uint64_t} interval The interval between two candles
 * @param {size_t} num_candles_allocated The number of allocated candles
 * @param {size_t} cur_candle The current number of allocated candles
 * @param {uint64_t} last_update The start of the last candle
 * @param {struct candle**} candles The list of candles
 * @param {struct chart_analysis*} analysis The analysis coorisponding to the
 * chart.
 * @param {char*} name The name of the chart
 */
struct chart {
  uint64_t interval;
  size_t num_candles_allocated;
  size_t cur_candle;
  uint64_t last_update;
  struct candle** candles;
  struct chart_analysis* analysis;
  char* name;
};

enum RISKI_ERROR_CODE chart_get_name(struct chart* cht, char** name) {
  PTR_CHECK(name, NULL_PTR, CHART_ERROR_CODE_STR);
  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);

  *name = cht->name;
  return CHART_NO_ERROR;
}

enum RISKI_ERROR_CODE chart_new(uint64_t interval, char* name,
                                struct chart** cht_) {
  PTR_CHECK(name, NULL_PTR, CHART_ERROR_CODE_STR);
  PTR_CHECK(cht_, NULL_PTR, CHART_ERROR_CODE_STR);

  // Initialize the chart
  struct chart* cht = (struct chart*)malloc(1 * sizeof(struct chart));
  cht->interval = interval;
  cht->name = name;
  cht->num_candles_allocated = 1440;
  cht->cur_candle = 0;
  cht->last_update = 0;

  // Create a list of candles pre allocated for 1 days worth
  cht->candles = (struct candle**)malloc((cht->num_candles_allocated) *
                                         sizeof(struct chart*));
  PTR_CHECK(cht->candles, NULL_PTR, CHART_ERROR_CODE_STR);

  // Initialize inner analysis struct
  cht->analysis =
      (struct chart_analysis*)malloc(1 * sizeof(struct chart_analysis));
  PTR_CHECK(cht->analysis, NULL_PTR, CHART_ERROR_CODE_STR);

  // Initialize the single candle patterns
  cht->analysis->scp = (enum SINGLE_CANDLE_PATTERNS*)malloc(
      (cht->num_candles_allocated) * sizeof(enum SINGLE_CANDLE_PATTERNS));
  PTR_CHECK(cht->analysis->scp, NULL_PTR, CHART_ERROR_CODE_STR);

  cht->analysis->trend_lines_horizontal = NULL;
  cht->analysis->num_trend_lines_horizontal = 0;

  cht->analysis->num_trend_lines_sloped = 0;
  cht->analysis->trend_lines_sloped = NULL;

  // initalize all the analysis to NONE
  for (size_t i = 0; i < cht->num_candles_allocated; ++i) {
    cht->analysis->scp[i] = SINGLE_CANDLE_PATTERN_NONE;
  }

  pthread_mutex_init(&(cht->analysis->lock), NULL);

  *cht_ = cht;
  return CHART_NO_ERROR;
}

enum RISKI_ERROR_CODE chart_analysis_lock(struct chart* cht) {
  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);
  pthread_mutex_lock(&(cht->analysis->lock));
  return CHART_NO_ERROR;
}

enum RISKI_ERROR_CODE chart_analysis_unlock(struct chart* cht) {
  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);
  pthread_mutex_unlock(&(cht->analysis->lock));
  return CHART_NO_ERROR;
}

enum RISKI_ERROR_CODE chart_put_single_candle_pattern(
    struct chart* cht, size_t index, enum SINGLE_CANDLE_PATTERNS identifier) {
  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);
  RANGE_CHECK(index, 0, cht->cur_candle, CHART_CANDLE_INDEX_INVALID,
              CHART_ERROR_CODE_STR);
  cht->analysis->scp[index] = identifier;
  return CHART_NO_ERROR;
}

enum RISKI_ERROR_CODE chart_invalidate_trends(struct chart* cht) {
  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);

  // First invalidate any trend lines that were broken before
  // this will stop us from joining new trend lines with previously
  // broken ones.

  if (cht->cur_candle < 2) return CHART_NO_ERROR;

  struct chart_analysis* cur_analysis = cht->analysis;

  for (size_t i = 0; i < cur_analysis->num_trend_lines_horizontal; ++i) {
    struct trend_line* t = &cur_analysis->trend_lines_horizontal[i];

    if (t->direction > 1) continue;

    if (t->direction == DIRECTION_SUPPORT) {
      if (candle_close(cht->candles[cht->cur_candle - 1]) <
          candle_low(cht->candles[t->end_index])) {
        t->direction = DIRECTION_INVALIDATED_SUPPORT;
        break;
      }
    } else if (t->direction == DIRECTION_RESISTANCE) {
      if (candle_close(cht->candles[cht->cur_candle - 1]) >
          candle_high(cht->candles[t->end_index])) {
        t->direction = DIRECTION_INVALIDATED_RESISTANCE;
        break;
      }
    }
  }
  return CHART_NO_ERROR;
}

enum RISKI_ERROR_CODE chart_put_sloped_line_pattern(struct chart* cht,
                                                    size_t start, size_t end,
                                                    enum DIRECTION direction) {
  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);
  struct chart_analysis* cur_analysis = cht->analysis;

  cur_analysis->num_trend_lines_sloped += 1;
  cur_analysis->trend_lines_sloped = (struct trend_line*)realloc(
      cur_analysis->trend_lines_sloped,
      cur_analysis->num_trend_lines_sloped * sizeof(struct trend_line));

  PTR_CHECK(cur_analysis->trend_lines_sloped, CHART_MALLOC_ERROR,
            CHART_ERROR_CODE_STR);

  size_t num_trend_lines = cur_analysis->num_trend_lines_sloped;
  cur_analysis->trend_lines_sloped[num_trend_lines - 1].direction = direction;
  cur_analysis->trend_lines_sloped[num_trend_lines - 1].start_index = start;
  cur_analysis->trend_lines_sloped[num_trend_lines - 1].end_index = end;

  return CHART_NO_ERROR;
}

enum RISKI_ERROR_CODE chart_put_horizontal_line_pattern(
    struct chart* cht, size_t start, size_t end, enum DIRECTION direction) {
  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);
  RANGE_CHECK(start, 0, end, CHART_CANDLE_INDEX_INVALID, CHART_ERROR_CODE_STR);
  RANGE_CHECK(end, start + 1, cht->cur_candle, CHART_CANDLE_INDEX_INVALID,
              CHART_ERROR_CODE_STR);

  struct chart_analysis* cur_analysis = cht->analysis;

  // Compute the "trend union"
  // If the start price of the current trend line is the
  // same end price of a different line then join the two lines together.

  int64_t current_trend_start_price = 0;
  switch (direction) {
    case DIRECTION_SUPPORT:
      current_trend_start_price = candle_low(cht->candles[end]);
      break;
    case DIRECTION_RESISTANCE:
      current_trend_start_price = candle_high(cht->candles[end]);
      break;
    default:
      (void)current_trend_start_price;
      log_error("DIRECTION_INVALIDATED is not a valid direction");
      exit(1);
  }

  for (size_t i = 0; i < cur_analysis->num_trend_lines_horizontal; ++i) {
    struct trend_line* t = &cur_analysis->trend_lines_horizontal[i];

    // don't join to an invalidated trend
    if (t->direction > 1) continue;

    // simple join
    if (t->start_index == start && t->direction == direction) {
      t->end_index = end;
      return CHART_NO_ERROR;
    }
  }

  // Add 1 new element to the analysis
  cur_analysis->num_trend_lines_horizontal += 1;
  cur_analysis->trend_lines_horizontal = (struct trend_line*)realloc(
      cur_analysis->trend_lines_horizontal,
      cur_analysis->num_trend_lines_horizontal * sizeof(struct trend_line));

  PTR_CHECK(cur_analysis->trend_lines_horizontal, CHART_MALLOC_ERROR,
            CHART_ERROR_CODE_STR);

  size_t num_trend_lines = cur_analysis->num_trend_lines_horizontal;
  cur_analysis->trend_lines_horizontal[num_trend_lines - 1].direction =
      direction;
  cur_analysis->trend_lines_horizontal[num_trend_lines - 1].start_index = start;
  cur_analysis->trend_lines_horizontal[num_trend_lines - 1].end_index = end;

  return CHART_NO_ERROR;
}

enum RISKI_ERROR_CODE chart_new_candle(struct chart* cht, int64_t price) {
  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);
  TRACE(candle_new(price, cht->last_update, &cht->candles[cht->cur_candle]));
  return CHART_NO_ERROR;
}

enum CHART_ERROR_CODE chart_get_candle(struct chart* cht, size_t index,
                                       struct candle** cnd) {
  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);
  RANGE_CHECK(index, 0, cht->cur_candle, CHART_CANDLE_INDEX_INVALID,
              CHART_ERROR_CODE_STR);

  *cnd = cht->candles[index];
  return CHART_NO_ERROR;
}

enum RISKI_ERROR_CODE chart_update(struct chart* cht, int64_t price,
                                   uint64_t ts) {
  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);

  // special case where this is the first chart update
  if (cht->last_update == 0) {
    cht->last_update = ts;
    TRACE(chart_new_candle(cht, price));
    return CHART_NO_ERROR;
  }

  // check if the interval requires us to make a new candle
  if (ts - cht->last_update > cht->interval) {
    // create a new candle
    // check if fill-ins are reqired
    size_t fill_in_candles = (ts - cht->last_update) / cht->interval;

    if (fill_in_candles != 1) {
      for (size_t i = 0; i < fill_in_candles - 1; ++i) {
        // fill in the candles in between with dojies of the
        // current candle before creating the new candle
        cht->cur_candle += 1;
        cht->last_update += cht->interval;
        TRACE(chart_new_candle(
            cht, candle_close(cht->candles[cht->cur_candle - 1])));
      }
    }

    cht->last_update = ts;
    cht->cur_candle += 1;
    chart_new_candle(cht, price);

    // queue up analysis on the newly finalized chart
    TRACE(analysis_push(cht, 0, cht->cur_candle));
  } else {
    // update the current candle
    TRACE(candle_update(cht->candles[cht->cur_candle], price, ts));
  }

  if (cht->cur_candle > g_max_candles) g_max_candles = cht->cur_candle;
}

enum RISKI_ERROR_CODE chart_json(struct chart* cht, char** json) {
  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);
  PTR_CHECK(json, NULL_PTR, CHART_ERROR_CODE_STR);

  // if the last update is 0 then there is no chart
  // information so we can't construct a valid json
  if (cht->last_update == 0) {
    *json = NULL;
    return CHART_NO_ERROR;
  }

  size_t num_candles = cht->cur_candle + 1;
  // chart proto = {"chart":[c1,c2,c3,c4...]}

  // allocate enough space to hold each candle
  size_t total_json_size = JSON_CANDLE_MAX_LEN * num_candles;

  // add (num_candles-1) commas
  total_json_size += (num_candles - 1);

  // add room for {"chart":[]}
  total_json_size += 12;

  // add room for null character
  total_json_size += 1;

  char* buf = (char*)calloc(total_json_size, sizeof(char));
  PTR_CHECK(buf, CHART_MALLOC_ERROR, CHART_ERROR_CODE_STR);
  strcat(buf, "{\"chart\":[\x0");

  char* tmp_candle_json = NULL;
  for (size_t i = 0; i < num_candles; ++i) {
    TRACE(candle_json(cht->candles[i], &tmp_candle_json));
    strcat(buf, tmp_candle_json);
    if (i != num_candles - 1) strcat(buf, ",\x0");

    free(tmp_candle_json);
    tmp_candle_json = NULL;
  }
  strcat(buf, "]}\x0");
  *json = buf;

  return CHART_NO_ERROR;
}

enum RISKI_ERROR_CODE chart_analysis_json(struct chart* cht, char** json) {
  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);
  PTR_CHECK(*json, NULL_PTR, CHART_ERROR_CODE_STR);

  struct chart_analysis* chta = cht->analysis;

  pthread_mutex_lock(&(chta->lock));

  size_t num_candles = cht->cur_candle + 1;
  // analysis proto {"analysis":
  //                {"singleCandle":[000000000,000000000,000000000,...]},
  //                {"trendLines":[{s:000000000,e:00000000,d:00000000}...]}}

  // begining prototype length {"analy...[
  size_t total_json_size = 30;
  // commas
  total_json_size += (num_candles - 1);

  // integer for each candle
  total_json_size += (num_candles * 8);

  // ending ]},
  total_json_size += 3;

  // {"trend_lines":[
  total_json_size += 16;

  // each trend line {s:...}
  total_json_size += (chta->num_trend_lines_horizontal * 36);

  // commas between each trend json
  total_json_size += (chta->num_trend_lines_horizontal - 1);

  // ending ]}}
  total_json_size += 3;

  // build the single candle analysis json
  char* buf = (char*)calloc(total_json_size, sizeof(char));
  PTR_CHECK(buf, CHART_MALLOC_ERROR, CHART_ERROR_CODE_STR);

  strcat(buf, "{\"analysis\":{\"singleCandle\":[\x0");
  for (size_t i = 0; i < num_candles; ++i) {
    char resbuf[10] = {0};
    sprintf(resbuf, "%u", chta->scp[i]);
    if (i != num_candles - 1) strcat(resbuf, ",\x0");
    strcat(buf, resbuf);
  }

  strcat(buf, "],\x0");

  // build the trend_lines json
  strcat(buf, "\"trendLines\":[\x0");
  for (size_t i = 0; i < chta->num_trend_lines_horizontal; ++i) {
    char trend_line_json_buf[34];
    sprintf(trend_line_json_buf, "{\"s\":%lu,\"e\":%lu,\"d\":%u}",
            chta->trend_lines_horizontal[i].start_index,
            chta->trend_lines_horizontal[i].end_index,
            (chta->trend_lines_horizontal[i].direction));
    if (i != chta->num_trend_lines_horizontal - 1)
      strcat(trend_line_json_buf, ",\x0");
    strcat(buf, trend_line_json_buf);
  }

  strcat(buf, "]}}\x0");
  pthread_mutex_unlock(&(chta->lock));

  *json = buf;
}

enum RISKI_ERROR_CODE chart_latest_candle(struct chart* cht, char** json) {
  // {"latestCandle":}

  PTR_CHECK(cht, NULL_PTR, CHART_ERROR_CODE_STR);
  PTR_CHECK(json, NULL_PTR, CHART_ERROR_CODE_STR);

  size_t total_json_size = 18 + JSON_CANDLE_MAX_LEN;

  char* buf = (char*)calloc(total_json_size, sizeof(char));
  PTR_CHECK(buf, CHART_MALLOC_ERROR, CHART_ERROR_CODE_STR);
  strcat(buf, "{\"latestCandle\":\x0");

  char* tmp_candle_json = NULL;
  TRACE(candle_json(cht->candles[cht->cur_candle], &tmp_candle_json));
  strcat(buf, tmp_candle_json);
  strcat(buf, "}\x0");

  free(tmp_candle_json);

  *json = NULL;
}

enum RISKI_ERROR_CODE chart_free(struct chart** cht) {
  if ((*cht)->last_update != 0) {
    for (size_t i = 0; i < ((*cht)->cur_candle + 1); ++i) {
      candle_free(&((*cht)->candles[i]));
    }
  }
  free((*cht)->candles);
  free(*cht);
  free((*cht)->analysis->scp);
  *cht = NULL;
}
