#include <pthread.h>
#include <security/security.h>

struct security {
  // the name of the security
  // always 8 characters long with right space padding
  char* name;

  // the hash of the name
  size_t hash;

  // the current order book of the security
  struct book* b;

  // the current chart of the security
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

char* security_get_analysis(struct security* sec) {
  return chart_analysis_json(sec->cht);
}

size_t security_get_hash(struct security* s) { return s->hash; }

size_t security_hash(char* name) { return hash((unsigned char*)name); }

char* security_get_chart(struct security* sec) {
  pthread_mutex_lock(&(sec->m_chart_update));
  char* ret = chart_json(sec->cht);
  pthread_mutex_unlock(&(sec->m_chart_update));
  return ret;
}

char* security_get_latest_candle(struct security* sec) {
  pthread_mutex_lock(&(sec->m_chart_update));
  char* ret = chart_latest_candle(sec->cht);
  pthread_mutex_unlock(&(sec->m_chart_update));
  return ret;
}

bool security_cmp(char* n1, struct security* s) {
  return (strcmp(n1, s->name)) == 0;
}

// creates a new security
struct security* security_new(char* name, uint64_t interval) {
  struct security* sec = (struct security*)malloc(1 * sizeof(struct security));

  char* n = (char*)malloc((strlen(name) + 1) * sizeof(char));
  strcpy(n, name);

  sec->name = n;
  sec->b = book_new();
  sec->cht = chart_new(interval, n);
  sec->hash = hash((unsigned char*)n);
  pthread_mutex_init(&(sec->m_chart_update), NULL);
  return sec;
}

// this is just an abstraction on the book update function
void security_book_update(struct security* sec, bool side, int64_t price,
                          int64_t quantity) {
  book_update(side, sec->b, price, quantity);
}

// this is just an abstraction on the chart update function
void security_chart_update(struct security* sec, int64_t price, uint64_t ts) {
  pthread_mutex_lock(&(sec->m_chart_update));
  chart_update(sec->cht, price, ts);
  pthread_mutex_unlock(&(sec->m_chart_update));
}

void security_free(struct security** sec) {
  book_free(&((*sec)->b));
  chart_free(&(*sec)->cht);
  free((*sec)->name);
  (*sec)->name = NULL;
  free(*sec);
  *sec = NULL;
}

void test_security() {
  struct security* sec = security_new("ABC     ", 10);
  ASSERT_TEST(sec != NULL);
  ASSERT_TEST(sec->b != NULL);
  ASSERT_TEST(sec->cht != NULL);
  ASSERT_TEST(strcmp(sec->name, "ABC     ") == 0);
  ASSERT_TEST(security_hash("ABC     ") == sec->hash);

  security_free(&sec);
  ASSERT_TEST(sec == NULL);
}
