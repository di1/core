#include <security/security.h>

/*
 * Holds information about a given security
 * @param {char*} name The name of the security
 * @param {size_t} hash The hash of the security name
 * @param {struct book*} b The order book
 * @param {struct chart*} cht The chart
 * @param {pthread_mutex_t} m_chart_update Lock mutex for getting chart info
 */
struct security {
  char* name;
  size_t hash;
  struct book* b;
  struct chart* cht;
  pthread_mutex_t m_chart_update;
};

size_t hash(unsigned char* str) {
  size_t hash = 5381;
  int c;
  while ((c = *str++)) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  hash = hash % SECURITY_HASH_MODULE_VAL;
  return hash;
}

enum RISKI_ERROR_CODE security_get_analysis(struct security* sec, char** json) {
  PTR_CHECK(sec, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  char* dat = NULL;
  TRACE(chart_analysis_json(sec->cht, &dat));

  *json = dat;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE security_get_hash(struct security* s, size_t* hash) {
  PTR_CHECK(s, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(hash, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  *hash = s->hash;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE security_hash(char* name, size_t* index) {
  PTR_CHECK(name, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(index, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  *index = hash((unsigned char*)name);
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE security_get_chart(struct security* sec, char** json) {
  PTR_CHECK(sec, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  pthread_mutex_lock(&(sec->m_chart_update));

  char* dat = NULL;
  TRACE(chart_json(sec->cht, &dat));
  pthread_mutex_unlock(&(sec->m_chart_update));

  *json = dat;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE security_get_latest_candle(struct security* sec,
                                                 char** json) {
  PTR_CHECK(sec, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  pthread_mutex_lock(&(sec->m_chart_update));

  char* dat = NULL;
  TRACE(chart_latest_candle(sec->cht, &dat));
  pthread_mutex_unlock(&(sec->m_chart_update));
  *json = dat;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE security_cmp(char* n1, struct security* s, bool* res) {
  PTR_CHECK(n1, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(s, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(res, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  *res = (strcmp(n1, s->name)) == 0;

  return RISKI_ERROR_CODE_NONE;
}

// creates a new security
enum RISKI_ERROR_CODE security_new(char* name, uint64_t interval,
                                   struct security** sec) {
  PTR_CHECK(name, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(sec, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct security* sec_ = (struct security*)malloc(1 * sizeof(struct security));

  PTR_CHECK(sec_, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  char* n = (char*)malloc((strlen(name) + 1) * sizeof(char));

  PTR_CHECK(n, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);
  strcpy(n, name);

  sec_->name = n;
  sec_->b = book_new();
  TRACE(chart_new(interval, n, &(sec_->cht)));
  sec_->hash = hash((unsigned char*)n);
  pthread_mutex_init(&(sec_->m_chart_update), NULL);

  *sec = sec_;

  return RISKI_ERROR_CODE_NONE;
}

// this is just an abstraction on the book update function
enum RISKI_ERROR_CODE security_book_update(struct security* sec, bool side,
                                           int64_t price, int64_t quantity) {
  PTR_CHECK(sec, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  book_update(side, sec->b, price, quantity);
  return RISKI_ERROR_CODE_NONE;
}

// this is just an abstraction on the chart update function
enum RISKI_ERROR_CODE security_chart_update(struct security* sec, int64_t price,
                                            uint64_t ts) {
  PTR_CHECK(sec, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  pthread_mutex_lock(&(sec->m_chart_update));
  chart_update(sec->cht, price, ts);
  pthread_mutex_unlock(&(sec->m_chart_update));

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE security_free(struct security** sec) {
  PTR_CHECK(sec, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  book_free(&((*sec)->b));
  TRACE(chart_free(&(*sec)->cht));
  free((*sec)->name);
  (*sec)->name = NULL;
  free(*sec);
  *sec = NULL;

  return RISKI_ERROR_CODE_NONE;
}
