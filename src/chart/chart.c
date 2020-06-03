#include <chart/chart.h>

#define MAX_INT_STR_LEN ((CHAR_BIT * sizeof (int) - 1) / 3 + 2)

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
struct chart
{
  uint64_t interval;
  size_t num_candles_allocated;
  size_t cur_candle;
  uint64_t last_update;
  struct candle **candles;

  pthread_mutex_t analysis_lock;
  struct analysis_result **analysis;
  char *name;
};

enum RISKI_ERROR_CODE
chart_get_name (struct chart *cht, char **name)
{
  PTR_CHECK (name, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  *name = cht->name;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_analysis_lock (struct chart *cht)
{
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  pthread_mutex_lock (&(cht->analysis_lock));
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_analysis_unlock (struct chart *cht)
{
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  pthread_mutex_unlock (&(cht->analysis_lock));
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_put_analysis (struct chart *cht, size_t idx, struct analysis_result *res)
{
  RANGE_CHECK (idx, 0, cht->cur_candle, RISKI_ERROR_CODE_INVALID_RANGE,
               RISKI_ERROR_TEXT);

  res->next = NULL;

  // check if there is already an analysis here
  if (cht->analysis[idx])
    {
      // get the last element in the list
      struct analysis_result *lst = cht->analysis[idx];
      while (lst->next)
        {
          lst = lst->next;
        }

      // set the next element to given one
      cht->analysis[idx]->next = res;
    }
  else
    {
      // no analysis here so just set it to the given one
      cht->analysis[idx] = res;
    }
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_analysis_trend_line_json (struct trend_line *tl, char **json)
{
  PTR_CHECK (tl, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct string_builder *sb = NULL;
  TRACE (string_builder_new (&sb));

  TRACE (string_builder_append (sb, "{"));
  TRACE (string_builder_append (sb, "\"endIndex\":"));

  char type_str[MAX_INT_STR_LEN];
  sprintf (type_str, "%d", (int)tl->end_index);

  TRACE (string_builder_append (sb, type_str));
  TRACE (string_builder_append (sb, ",\"startIndex\":"));

  sprintf (type_str, "%d", (int)tl->start_index);
  TRACE (string_builder_append (sb, type_str));

  TRACE (string_builder_append (sb, ",\"direction\":"));

  sprintf (type_str, "%d", (int)tl->direction);
  TRACE (string_builder_append (sb, type_str));

  TRACE (string_builder_append (sb, "}"));

  char *sb_str = NULL;
  TRACE (string_builder_str (sb, &sb_str));

  *json = sb_str;

  TRACE (string_builder_free (&sb));

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_analysis_candle_pattern_json (struct candle_pattern *cp, char **json)
{
  PTR_CHECK (cp, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct string_builder *sb = NULL;
  TRACE (string_builder_new (&sb));

  TRACE (string_builder_append (sb, "{"));
  TRACE (string_builder_append (sb, "\"candlesSpanning\":"));

  char type_str[MAX_INT_STR_LEN];
  sprintf (type_str, "%d", (int)cp->candles_spanning);

  TRACE (string_builder_append (sb, type_str));
  TRACE (string_builder_append (sb, ",\"shortCode\":\""));
  TRACE (string_builder_append (sb, cp->short_code));
  TRACE (string_builder_append (sb, "\""));
  TRACE (string_builder_append (sb, "}"));

  char *sb_str = NULL;
  TRACE (string_builder_str (sb, &sb_str));

  *json = sb_str;

  TRACE (string_builder_free (&sb));

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_analysis_result_json (struct analysis_result *analysis, char **json)
{
  PTR_CHECK (analysis, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct string_builder *sb = NULL;
  TRACE (string_builder_new (&sb));

  /*
   * [
   *   {
   *    "type": number,
   *    "drawData": {
   *      // could be candle_pattern | trend_line | ...
   *    }
   *   },
   *   .
   *   .
   *   .
   * ]
   */

  TRACE (string_builder_append (sb, "["));

  while (analysis)
    {

      TRACE (string_builder_append (sb, "{\"type\":"));

      // convert type integer to string
      char type_str[MAX_INT_STR_LEN];
      sprintf (type_str, "%d", analysis->type);

      TRACE (string_builder_append (sb, type_str));
      TRACE (string_builder_append (sb, ",\"data\":"));

      char *data_json = NULL;
      switch (analysis->type)
        {
        case CANDLE_PATTERN:
          TRACE (chart_analysis_candle_pattern_json (analysis->draw_data,
                                                     &data_json));
          break;
        case TREND_LINE:
          TRACE (chart_analysis_trend_line_json (analysis->draw_data,
                                                 &data_json));
          break;
        }
      TRACE (string_builder_append (sb, data_json));
      TRACE (string_builder_append (sb, "}"));

      if (analysis->next)
        {
          TRACE (string_builder_append (sb, ","));
        }
      analysis = analysis->next;
    }

  TRACE (string_builder_append (sb, "]"));

  char *jsn = NULL;
  TRACE (string_builder_str (sb, &jsn));

  *json = jsn;

  TRACE (string_builder_free (&sb));

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_analysis_json (struct chart *cht, char **json)
{
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct string_builder *sb = NULL;
  TRACE (string_builder_new (&sb));

  TRACE (string_builder_append (sb, "{\"analysisFull\": ["));

  for (size_t i = 0; i < cht->cur_candle - 1; ++i)
    {
      char *candle_analysis = NULL;
      if (cht->analysis[i] == NULL)
        {
          TRACE (string_builder_append (sb, "null"));
        }
      else
        {
          TRACE (
              chart_analysis_result_json (cht->analysis[i], &candle_analysis));
          TRACE (string_builder_append (sb, candle_analysis));
        }
      if (i != cht->cur_candle - 2)
        {
          TRACE (string_builder_append (sb, ","));
        }
    }

  TRACE (string_builder_append (sb, "]}"));

  char *jsn = NULL;
  TRACE (string_builder_str (sb, &jsn));
  TRACE (string_builder_free (&sb));

  *json = jsn;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_new (uint64_t interval, char *name, struct chart **cht_)
{
  PTR_CHECK (name, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (cht_, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // Initialize the chart
  struct chart *cht = (struct chart *)malloc (1 * sizeof (struct chart));
  cht->interval = interval;
  cht->name = name;
  cht->num_candles_allocated = 1440;
  cht->cur_candle = 0;
  cht->last_update = 0;

  // Create a list of candles pre allocated for 1 days worth
  cht->candles = (struct candle **)malloc ((cht->num_candles_allocated)
                                           * sizeof (struct chart *));

  // Create the bins for the analysis results at each candle
  cht->analysis = (struct analysis_result **)malloc (
      sizeof (struct analysis_result *) * cht->num_candles_allocated);

  pthread_mutex_init (&(cht->analysis_lock), NULL);

  // Set each bin to null cause there is no analysis there yet
  for (size_t i = 0; i < cht->num_candles_allocated; ++i)
    {
      cht->analysis[i] = NULL;
    }

  PTR_CHECK (cht->candles, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  *cht_ = cht;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_new_candle (struct chart *cht, int64_t price)
{
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  if (cht->cur_candle >= cht->num_candles_allocated)
    {
      size_t prev_candles_allocated = cht->num_candles_allocated;
      cht->num_candles_allocated
          = (size_t) ((double)cht->num_candles_allocated * (double)1.5);
      cht->candles = realloc (cht->candles, sizeof (struct candle **)
                                                * cht->num_candles_allocated);
      cht->analysis = realloc (cht->candles, sizeof (struct analysis_result)
                                                 * cht->num_candles_allocated);

      // set the newly allocated memory to their default state.
      for (size_t i = prev_candles_allocated; i < cht->num_candles_allocated;
           ++i)
        {
          cht->analysis[i] = NULL;
        }
    }

  TRACE (candle_new (price, cht->last_update, &cht->candles[cht->cur_candle]));
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_get_candle (struct chart *cht, size_t index, struct candle **cnd)
{
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  RANGE_CHECK (index, 0, cht->cur_candle, RISKI_ERROR_CODE_INVALID_RANGE,
               RISKI_ERROR_TEXT);
  *cnd = cht->candles[index];
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_update (struct chart *cht, int64_t price, uint64_t ts)
{
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // convert ts into the start time of its coorisponding candle
  // this will sync all the candles to the beginning minute no matter
  // when the server is started
  size_t offset = ts % cht->interval;
  ts = ts - offset;
  // special case where this is the first chart update
  if (cht->last_update == 0)
    {
      // sync the first candle to the closest minute
      cht->last_update = ts;
      TRACE (chart_new_candle (cht, price));
      return RISKI_ERROR_CODE_NONE;
    }

  // check if the interval requires us to make a new candle
  if (ts != cht->last_update)
    {
      // create a new candle
      // check if fill-ins are required
      size_t fill_in_candles = ((ts - cht->last_update) / cht->interval);
      if (fill_in_candles != 1)
        {
          for (size_t i = 0; i < fill_in_candles - 1; ++i)
            {
              // fill in the candles in between with dojies of the
              // current candle before creating the new candle
              cht->cur_candle += 1;
              cht->last_update += cht->interval;

              int64_t close = 0;
              TRACE (candle_close (cht->candles[cht->cur_candle - 1], &close));
              TRACE (chart_new_candle (cht, close));
            }
        }

      cht->last_update = ts;
      cht->cur_candle += 1;
      chart_new_candle (cht, price);

      // queue up analysis on the newly finalized chart
      TRACE (analysis_push (cht, 0, cht->cur_candle));
    }
  else
    {
      // update the current candle
      TRACE (candle_update (cht->candles[cht->cur_candle], price, ts));
    }

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_json (struct chart *cht, char **json)
{
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // if the last update is 0 then there is no chart
  // information so we can't construct a valid json
  if (cht->last_update == 0)
    {
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

  char *buf = (char *)calloc (total_json_size, sizeof (char));
  PTR_CHECK (buf, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);
  strcat (buf, "{\"chart\":[\x0");

  char *tmp_candle_json = NULL;
  for (size_t i = 0; i < num_candles; ++i)
    {
      TRACE (candle_json (cht->candles[i], &tmp_candle_json));
      strcat (buf, tmp_candle_json);
      if (i != num_candles - 1)
        strcat (buf, ",\x0");

      free (tmp_candle_json);
      tmp_candle_json = NULL;
    }
  strcat (buf, "]}\x0");
  *json = buf;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_latest_candle (struct chart *cht, char **json)
{
  // {"latestCandle":}

  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  size_t total_json_size = 18 + JSON_CANDLE_MAX_LEN;

  char *buf = (char *)calloc (total_json_size, sizeof (char));
  PTR_CHECK (buf, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);
  strcat (buf, "{\"latestCandle\":\x0");

  char *tmp_candle_json = NULL;
  TRACE (candle_json (cht->candles[cht->cur_candle], &tmp_candle_json));
  strcat (buf, tmp_candle_json);
  strcat (buf, "}\x0");

  free (tmp_candle_json);

  *json = buf;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
chart_free (struct chart **cht)
{
  PTR_CHECK (cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (*cht, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  if ((*cht)->last_update != 0)
    {
      for (size_t i = 0; i < ((*cht)->cur_candle + 1); ++i)
        {
          TRACE (candle_free (&((*cht)->candles[i])));
        }
    }
  free ((*cht)->candles);
  // TODO free the analysis lists
  free ((*cht)->analysis);
  free ((*cht));
  *cht = NULL;

  return RISKI_ERROR_CODE_NONE;
}

#undef MAX_INT_STR_LEN
