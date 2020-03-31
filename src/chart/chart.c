#include <chart/chart.h>

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
  size_t score;
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
  PTR_CHECK(name, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  *name = cht->name;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_new(uint64_t interval, char* name,
                                struct chart** cht_) {
  PTR_CHECK(name, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(cht_, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

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
  PTR_CHECK(cht->candles, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // Initialize inner analysis struct
  cht->analysis =
      (struct chart_analysis*)malloc(1 * sizeof(struct chart_analysis));
  PTR_CHECK(cht->analysis, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // Initialize the single candle patterns
  cht->analysis->scp = (enum SINGLE_CANDLE_PATTERNS*)malloc(
      (cht->num_candles_allocated) * sizeof(enum SINGLE_CANDLE_PATTERNS));
  PTR_CHECK(cht->analysis->scp, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

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
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_analysis_lock(struct chart* cht) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  pthread_mutex_lock(&(cht->analysis->lock));
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_analysis_unlock(struct chart* cht) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  pthread_mutex_unlock(&(cht->analysis->lock));
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_put_single_candle_pattern(
    struct chart* cht, size_t index, enum SINGLE_CANDLE_PATTERNS identifier) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  RANGE_CHECK(index, 0, cht->cur_candle, RISKI_ERROR_CODE_INVALID_RANGE,
              RISKI_ERROR_TEXT);
  cht->analysis->scp[index] = identifier;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_invalidate_trends(struct chart* cht) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // First invalidate any trend lines that were broken before
  // this will stop us from joining new trend lines with previously
  // broken ones.

  if (cht->cur_candle < 2) return RISKI_ERROR_CODE_NONE;

  struct chart_analysis* cur_analysis = cht->analysis;

  for (size_t i = 0; i < cur_analysis->num_trend_lines_horizontal; ++i) {
    struct trend_line* t = &cur_analysis->trend_lines_horizontal[i];

    int64_t close = 0;

    if (t->direction > 1) continue;

    if (t->direction == DIRECTION_SUPPORT) {
      // get the candle close and low
      TRACE(candle_close(cht->candles[cht->cur_candle - 1], &close));
      int64_t low = 0;
      TRACE(candle_low(cht->candles[t->start_index], &low));
      if (close < low) {
        t->direction = DIRECTION_INVALIDATED_SUPPORT;
        break;
      }
    } else if (t->direction == DIRECTION_RESISTANCE) {
      TRACE(candle_close(cht->candles[cht->cur_candle - 1], &close));
      int64_t high = 0;
      TRACE(candle_high(cht->candles[t->start_index], &high));
      if (close > high) {
        t->direction = DIRECTION_INVALIDATED_RESISTANCE;
        break;
      }
    }
  }
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_put_sloped_line_pattern(struct chart* cht,
                                                    size_t start, size_t end,
                                                    enum DIRECTION direction, size_t score) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  RANGE_CHECK(score, 0, 101, RISKI_ERROR_CODE_INVALID_RANGE, RISKI_ERROR_TEXT);

  struct chart_analysis* cur_analysis = cht->analysis;

  cur_analysis->num_trend_lines_sloped += 1;
  cur_analysis->trend_lines_sloped = (struct trend_line*)realloc(
      cur_analysis->trend_lines_sloped,
      cur_analysis->num_trend_lines_sloped * sizeof(struct trend_line));

  PTR_CHECK(cur_analysis->trend_lines_sloped, RISKI_ERROR_CODE_MALLOC_ERROR,
            RISKI_ERROR_TEXT);

  size_t num_trend_lines = cur_analysis->num_trend_lines_sloped;
  cur_analysis->trend_lines_sloped[num_trend_lines - 1].direction = direction;
  cur_analysis->trend_lines_sloped[num_trend_lines - 1].start_index = start;
  cur_analysis->trend_lines_sloped[num_trend_lines - 1].end_index = end;
  cur_analysis->trend_lines_sloped[num_trend_lines - 1].score = score;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_put_horizontal_line_pattern(
    struct chart* cht, size_t start, size_t end, enum DIRECTION direction) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  RANGE_CHECK(start, 0, end, RISKI_ERROR_CODE_INVALID_RANGE, RISKI_ERROR_TEXT);
  RANGE_CHECK(end, start + 1, cht->cur_candle, RISKI_ERROR_CODE_INVALID_RANGE,
              RISKI_ERROR_TEXT);

  struct chart_analysis* cur_analysis = cht->analysis;

  // Compute the "trend union"
  // If the start price of the current trend line is the
  // same end price of a different line then join the two lines together.

  int64_t current_trend_start_price = 0;
  switch (direction) {
    case DIRECTION_SUPPORT:
      TRACE(candle_low(cht->candles[end], &current_trend_start_price));
      break;
    case DIRECTION_RESISTANCE:
      TRACE(candle_high(cht->candles[end], &current_trend_start_price));
      break;
    default:
      (void)current_trend_start_price;
      TRACE(logger_error(RISKI_ERROR_CODE_UNKNOWN, __func__, __FILENAME__,
                         __LINE__,
                         "DIRECTION_INVALIDATED is not a valid direction"));
  }

  for (size_t i = 0; i < cur_analysis->num_trend_lines_horizontal; ++i) {
    struct trend_line* t = &cur_analysis->trend_lines_horizontal[i];

    // don't join to an invalidated trend
    if (t->direction > 1) continue;

    // simple join
    if (t->start_index == start && t->direction == direction) {
      t->end_index = end;
      return RISKI_ERROR_CODE_NONE;
    }
  }

  // Add 1 new element to the analysis
  cur_analysis->num_trend_lines_horizontal += 1;
  cur_analysis->trend_lines_horizontal = (struct trend_line*)realloc(
      cur_analysis->trend_lines_horizontal,
      cur_analysis->num_trend_lines_horizontal * sizeof(struct trend_line));

  PTR_CHECK(cur_analysis->trend_lines_horizontal, RISKI_ERROR_CODE_MALLOC_ERROR,
            RISKI_ERROR_TEXT);

  size_t num_trend_lines = cur_analysis->num_trend_lines_horizontal;
  cur_analysis->trend_lines_horizontal[num_trend_lines - 1].direction =
      direction;
  cur_analysis->trend_lines_horizontal[num_trend_lines - 1].start_index = start;
  cur_analysis->trend_lines_horizontal[num_trend_lines - 1].end_index = end;
  cur_analysis->trend_lines_horizontal[num_trend_lines - 1].score = 0;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_new_candle(struct chart* cht, int64_t price) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  TRACE(candle_new(price, cht->last_update, &cht->candles[cht->cur_candle]));
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_get_candle(struct chart* cht, size_t index,
                                       struct candle** cnd) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  RANGE_CHECK(index, 0, cht->cur_candle, RISKI_ERROR_CODE_INVALID_RANGE,
              RISKI_ERROR_TEXT);

  *cnd = cht->candles[index];
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_update(struct chart* cht, int64_t price,
                                   uint64_t ts) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // special case where this is the first chart update
  if (cht->last_update == 0) {
    cht->last_update = ts;
    TRACE(chart_new_candle(cht, price));
    return RISKI_ERROR_CODE_NONE;
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

        int64_t close = 0;
        TRACE(candle_close(cht->candles[cht->cur_candle - 1], &close));
        TRACE(chart_new_candle(cht, close));
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
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_json(struct chart* cht, char** json) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // if the last update is 0 then there is no chart
  // information so we can't construct a valid json
  if (cht->last_update == 0) {
    *json = NULL;
    return RISKI_ERROR_CODE_NONE;
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
  PTR_CHECK(buf, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);
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

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_analysis_json(struct chart* cht, char** json) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct chart_analysis* chta = cht->analysis;

  pthread_mutex_lock(&(chta->lock));
  size_t num_candles = cht->cur_candle + 1;

  struct string_builder* sb = NULL;
  TRACE(string_builder_new(&sb));
  TRACE(string_builder_append(sb, "{\"analysis\":{\"singleCandle\":["));
  for (size_t i = 0; i < num_candles; ++i) {
    char resbuf[10] = {0};
    sprintf(resbuf, "%u", chta->scp[i]);
    if (i != num_candles - 1) strcat(resbuf, ",");
    TRACE(string_builder_append(sb, resbuf));
  }
  TRACE(string_builder_append(sb, "], \"trendLines\":["));
  for (size_t i = 0; i < chta->num_trend_lines_horizontal; ++i) {
    char trend_line_json_buf[100];
    sprintf(trend_line_json_buf, "{\"s\":%lu,\"e\":%lu,\"d\":%u,\"score\":%lu}",
            chta->trend_lines_horizontal[i].start_index,
            chta->trend_lines_horizontal[i].end_index,
            chta->trend_lines_horizontal[i].direction,
            chta->trend_lines_horizontal[i].score);
    if (i != chta->num_trend_lines_horizontal - 1)
      strcat(trend_line_json_buf, ",");
    TRACE(string_builder_append(sb, trend_line_json_buf));
  }
  TRACE(string_builder_append(sb, "], \"slopedLines\": ["));
  for (size_t i = 0; i < chta->num_trend_lines_sloped; ++i) {
    char trend_line_json_buf[100];
    sprintf(trend_line_json_buf, "{\"s\":%lu,\"e\":%lu,\"d\":%u,\"score\":%lu}",
            chta->trend_lines_sloped[i].start_index,
            chta->trend_lines_sloped[i].end_index,
            chta->trend_lines_sloped[i].direction,
            chta->trend_lines_sloped[i].score);
    if (i != chta->num_trend_lines_sloped - 1) strcat(trend_line_json_buf, ",");
    TRACE(string_builder_append(sb, trend_line_json_buf));
  }
  TRACE(string_builder_append(sb, "]}}"));
  pthread_mutex_unlock(&(chta->lock));

  char* str_rep = NULL;
  TRACE(string_builder_str(sb, &str_rep));
  TRACE(string_builder_free(&sb));

  *json = str_rep;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_latest_candle(struct chart* cht, char** json) {
  // {"latestCandle":}

  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  size_t total_json_size = 18 + JSON_CANDLE_MAX_LEN;

  char* buf = (char*)calloc(total_json_size, sizeof(char));
  PTR_CHECK(buf, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);
  strcat(buf, "{\"latestCandle\":\x0");

  char* tmp_candle_json = NULL;
  TRACE(candle_json(cht->candles[cht->cur_candle], &tmp_candle_json));
  strcat(buf, tmp_candle_json);
  strcat(buf, "}\x0");

  free(tmp_candle_json);

  *json = buf;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE chart_free(struct chart** cht) {
  PTR_CHECK(cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(*cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  if ((*cht)->last_update != 0) {
    for (size_t i = 0; i < ((*cht)->cur_candle + 1); ++i) {
      TRACE(candle_free(&((*cht)->candles[i])));
    }
  }
  free((*cht)->candles);
  free(*cht);
  free((*cht)->analysis->scp);
  *cht = NULL;

  return RISKI_ERROR_CODE_NONE;
}
