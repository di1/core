#include <exchange/exchange.h>

/*
 * The number of securities that have been added to the exchange
 */
size_t num_securities = 0;

/*
 * Linked list for the hash bins
 * @param {struct security*} val The value of the bin
 * @param {struct ll*} next The next element in the list
 */
struct ll
{
  struct security *val;
  struct ll *next;
};

/*
 * Holds the exchange information
 * @param {char*} name The name of the exchange
 * @param {struct ll} A fixed number of bins for the hashmap
 */
struct exchange
{
  char *name;
  struct ll entries[SECURITY_HASH_MODULE_VAL];
};

enum RISKI_ERROR_CODE
exchange_new (char *name, struct exchange **exchange)
{
  PTR_CHECK (name, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (exchange, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  char *n = (char *)malloc ((strlen (name) + 1) * sizeof (char));
  PTR_CHECK (n, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);
  strcpy (n, name);

  struct exchange *e = (struct exchange *)calloc (1, sizeof (struct exchange));
  PTR_CHECK (e, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);
  e->name = n;

  *exchange = e;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
exchange_put (struct exchange *e, char *name, uint64_t interval)
{
  struct security *s = NULL;
  TRACE (security_new (name, interval, &s));

  size_t index = 0;
  TRACE (security_get_hash (s, &index));

  struct ll *val = &e->entries[index];
  if (val->val == NULL)
    {
      val->val = s;
    }
  else
    {
      while (val->next != NULL)
        {
          val = val->next;
        }

      struct ll *next_entry = (struct ll *)malloc (1 * sizeof (struct ll));
      PTR_CHECK (next_entry, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);
      next_entry->next = NULL;
      next_entry->val = s;

      val->next = next_entry;
    }

  num_securities += 1;
  if (num_securities % 200 == 0)
    TRACE (logger_info (__func__, __FILENAME__, __LINE__,
                        "~%lu securities monitored", num_securities));

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
exchange_get (struct exchange *e, char *name, struct security **sec)
{
  PTR_CHECK (e, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (sec, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  size_t hash = 0;
  TRACE (security_hash (name, &hash));

  struct ll *bin_list = &e->entries[hash];

  while (bin_list != NULL)
    {
      if (bin_list->val == NULL)
        {
          *sec = NULL;
          return RISKI_ERROR_CODE_NONE;
        }
      bool isequal = false;
      TRACE (security_cmp (name, bin_list->val, &isequal));

      if (isequal)
        {
          *sec = bin_list->val;
          return RISKI_ERROR_CODE_NONE;
        }
      bin_list = bin_list->next;
    }

  *sec = NULL;
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE
exchange_free (struct exchange **e)
{
  PTR_CHECK (e, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK (*e, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  free ((*e)->name);
  for (size_t i = 0; i < SECURITY_HASH_MODULE_VAL; ++i)
    {
      // free the ll entries
      if ((*e)->entries[i].val != NULL)
        {
          TRACE (security_free (&((*e)->entries[i].val)));
          if ((*e)->entries[i].next != NULL)
            {
              struct ll *cur = (*e)->entries[i].next;
              while (cur != NULL)
                {
                  struct ll *tmp = cur;
                  cur = cur->next;
                  TRACE (security_free (&tmp->val));
                  free (tmp);
                }
            }
        }
    }
  free (*e);
  *e = NULL;
  return RISKI_ERROR_CODE_NONE;
}
