#include <exchange/exchange.h>

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
  char* n = (char*) malloc(strlen(name)*sizeof(char));
  strcpy(n, name);

  struct exchange* e = (struct exchange*) calloc(1, sizeof(struct exchange)); 
  e->name = n;

  return e;
}

void exchange_put(struct exchange* e) {

}

void exchange_free(struct exchange** e) {
  free((*e)->name);
  for (size_t i = 0; i < SECURITY_HASH_MODULE_VAL; ++i) {
    // free the ll entries
    security_free(&((*e)->entries[i].val));
    if ((*e)->entries[i].next != NULL) {
      struct ll* cur = (*e)->entries[i].next;
      while (cur != NULL) {
        struct ll* tmp = cur;
        cur = cur->next;

        security_free(&cur->val);
        free(tmp);
      } 
    }
  } 
  free(*e);
  *e = NULL;
}
