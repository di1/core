#include <chart/candle.h>

struct candle {
  // the open price
  int64_t open;

  // the high price
  int64_t high;

  // the low price
  int64_t low;

  // the close price
  int64_t close;

  // the start time
  uint64_t start_time;

  // the end time
  uint64_t end_time;
};

struct candle* candle_new(int64_t price, uint64_t time) {
  struct candle* c = (struct candle*) malloc (1*sizeof(struct candle));
  c->open = price;
  c->high = price;
  c->low = price;
  c->close = price;
  c->start_time = time;
  c->end_time = time;

  return c;
}

void candle_update(struct candle* c, int64_t price, uint64_t time) {

  // set the close time only if the last price time
  // is greater than the most recent price
  if (time > c->end_time) {
    c->end_time = time;
    c->close = price;
  }

  // update the high and low
  // the time doesn't matter in this case
  if (price > c->high)
    c->high = price;
  if (price < c->low)
    c->low = price;
}

void candle_free(struct candle** c) {
  free(*c);
  *c = NULL;
}

// convert a candle struct to json
char* candle_json(struct candle* c) {
  /**
   * {
   *  "candle" : {
   *    "o": 00000000000000000000,
   *    "h": 00000000000000000000,
   *    "l": 00000000000000000000,
   *    "c": 00000000000000000000,
   *    "s": 00000000000000000000,
   *    "e": 00000000000000000000
   *  }
   * }
   */
  
  // max 170 characters (no whitespace or new lines
  char* buf = (char*) malloc(JSON_CANDLE_MAX_LEN*sizeof(char));
  int ret = sprintf(buf, "{\"candle\":{"
      "\"o\":%ld,\"h\":%ld,\"l\":%ld,\"c\":%ld,\"s\":%lu,\"e\":%lu}}",
      c->open, c->high, c->low, c->close, c->start_time, c->end_time);

  if (ret == -1) {
    log_error("sprintf on candle ran out of space in buf of 170 allocated"
        " characters");
  }

  return buf;

}

void test_candle() {
  
  // make sure candle creation is good
  struct candle* c = candle_new(0, 0);
  ASSERT_TEST(c != NULL);
  ASSERT_TEST(c->start_time == 0);
  ASSERT_TEST(c->end_time == 0);
  ASSERT_TEST(c->open == 0);
  ASSERT_TEST(c->high == 0);
  ASSERT_TEST(c->low == 0);
  ASSERT_TEST(c->close == 0);

  // new high
  candle_update(c, 1, 1);
  ASSERT_TEST(c->end_time == 1);
  ASSERT_TEST(c->open == 0);
  ASSERT_TEST(c->high == 1);
  ASSERT_TEST(c->low == 0);
  ASSERT_TEST(c->close == 1);

  // new low
  candle_update(c, -1, 2);
  ASSERT_TEST(c->end_time == 2);
  ASSERT_TEST(c->open == 0);
  ASSERT_TEST(c->high == 1);
  ASSERT_TEST(c->low == -1);
  ASSERT_TEST(c->close == -1);

  // test high that happend before
  candle_update(c, 5, 1);
  ASSERT_TEST(c->end_time == 2);
  ASSERT_TEST(c->open == 0);
  ASSERT_TEST(c->high == 5);
  ASSERT_TEST(c->low == -1);
  ASSERT_TEST(c->close == -1);

  // test low that happened before
  candle_update(c, -5, 1);
  ASSERT_TEST(c->end_time == 2);
  ASSERT_TEST(c->open == 0);
  ASSERT_TEST(c->high == 5);
  ASSERT_TEST(c->low == -5);
  ASSERT_TEST(c->close == -1);

  // test price update that doesn't update high or low
  candle_update(c, 3, 6);
  ASSERT_TEST(c->end_time == 6);
  ASSERT_TEST(c->open == 0);
  ASSERT_TEST(c->high == 5);
  ASSERT_TEST(c->low == -5);
  ASSERT_TEST(c->close == 3);

  // make sure the json is correct
  char* json = candle_json(c);
  ASSERT_TEST(
    strcmp(json,
      "{\"candle\":{\"o\":0,\"h\":5,\"l\":-5,\"c\":3,\"s\":0,\"e\":6}}") == 0);

  // make sure freeing works
  candle_free(&c);
  ASSERT_TEST(c == NULL);  
  
  // free the candle json
  free(json);
}
