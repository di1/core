#include <exchange/exchange.h>

size_t num_securities = 0;

char** security_names = NULL;

// linked list prototype
struct ll {
  // current security value
  struct security* val;

  // next value
  struct ll* next;
};

struct exchange {
  char* name;
  struct ll entries[SECURITY_HASH_MODULE_VAL];
};

struct exchange* exchange_new(char* name) {
  char* n = (char*) malloc((strlen(name)+1)*sizeof(char));
  strcpy(n, name);

  struct exchange* e = (struct exchange*) calloc(1, sizeof(struct exchange));
  e->name = n;

  return e;
}

void exchange_put(struct exchange* e, char* name, uint64_t interval) {
  struct security* s = security_new(name, interval);
  size_t index = security_get_hash(s);

  struct ll* val = &e->entries[index];
  if (val->val == NULL) {
    val->val = s;
  } else {
    while (val->next != NULL) {
      val = val->next;
    }

    struct ll* next_entry = (struct ll*) malloc(1*sizeof(struct ll));
    next_entry->next = NULL;
    next_entry->val = s;

    val->next = next_entry;
  }

  num_securities += 1;
  if (num_securities%200 == 0)
    log_debug("~%lu securities monitored", num_securities);
  security_names = (char**) realloc(security_names, num_securities*sizeof(char*));

  char* n = (char*) malloc((strlen(name)+1)*sizeof(char));
  strcpy(n, name);

  security_names[num_securities-1] = n;

}

struct security* exchange_get(struct exchange* e, char* name) {

  size_t hash = security_hash(name);
  struct ll* bin_list = &e->entries[hash];

  while (bin_list != NULL) {
    if (bin_list->val == NULL)
      return NULL;
    if (security_cmp(name, bin_list->val)) {
      return bin_list->val;
    }
    bin_list = bin_list->next;
  }

  return NULL;
}

void exchange_free(struct exchange** e) {
  free((*e)->name);
  for (size_t i = 0; i < SECURITY_HASH_MODULE_VAL; ++i) {
    // free the ll entries
    if ((*e)->entries[i].val != NULL) {
      security_free(&((*e)->entries[i].val));
      if ((*e)->entries[i].next != NULL) {
        struct ll* cur = (*e)->entries[i].next;
        while (cur != NULL) {
          struct ll* tmp = cur;
          cur = cur->next;
          security_free(&tmp->val);
          free(tmp);
        }
      }
    }
  }
  free(*e);
  *e = NULL;

  for (size_t i = 0; i < num_securities; ++i) {
    free(security_names[i]);
  }
  free(security_names);
}

void test_exchange() {
  struct exchange* ex = exchange_new("test");
  ASSERT_TEST(ex != NULL);

  exchange_put(ex, "test    ", 1000);

  struct security* sec = exchange_get(ex, "test    ");
  ASSERT_TEST(security_get_hash(sec) == security_hash("test    "));

  exchange_put(ex, "test1234", 10000);

  sec = exchange_get(ex, "test1234");
  ASSERT_TEST(security_get_hash(sec) == security_hash("test1234"));

  exchange_free(&ex);
  ASSERT_TEST(ex == NULL);

}
