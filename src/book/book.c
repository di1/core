#include <book/book.h>

/**
 * Defines a level inside the book
 */
struct level {
  // The price as a fixed point number
  int64_t price;

  // Quantity
  int64_t quantity;
};

/**
 * The meta class of a book
 */
struct book {
  // The number of buys on the buy side
  int64_t buys_len;

  // The number of sells on the sell side
  int64_t sells_len;

  // Hold the buys side
  struct level *buys;

  // Hold the sells side
  struct level *sells;
};

struct book *book_new() {
  // Allocate a new book
  struct book *t = (struct book *)(malloc(1 * sizeof(struct book)));

  // set number of buys and sells to 0
  t->buys_len = 0;
  t->sells_len = 0;

  // set both arrays to NULL since their are 0 elements
  t->buys = NULL;
  t->sells = NULL;
  return t;
}

// removes a price lvl from the lvl array
static void remove_lvl(struct level **lvls, int64_t index, int64_t count) {
  // delete the price lvl
  // tmp array
  struct level *lvl_tmp =
      (struct level *)(malloc((uint64_t)(count - 1) * sizeof(struct level)));
  // copy the left side
  for (int64_t i = 0; i < index; ++i) {
    lvl_tmp[i] = (*lvls)[i];
  }
  // copy the right side
  for (int64_t i = index + 1; i < count; ++i) {
    lvl_tmp[i - 1] = (*lvls)[i];
  }

  // 0,1,2,3

  // free old lvls
  free(*lvls);
  *lvls = lvl_tmp;

  // special case if count is not 0
  if (count - 1 == 0) {
    free(lvl_tmp);
    *lvls = NULL;
  }
}

static void insert_at(struct level **lvls, int64_t len, int64_t at,
                      struct level lvl) {
  len += 1;

  // create a new array
  struct level *new_lvl =
      (struct level *)(malloc((uint64_t)len * sizeof(struct level)));

  // put the new data inside
  new_lvl[at] = lvl;

  // copy the right side data
  for (int64_t i = 0; i <= (at - 1); ++i) {
    new_lvl[i] = (*lvls)[i];
  }

  // copy the left side data
  for (int64_t i = (at); i < (len - 1); ++i) {
    new_lvl[i + 1] = (*lvls)[i];
  }

  free(*lvls);
  *lvls = new_lvl;
}

void book_update(bool side, struct book *t, int64_t price, int64_t quantity) {
  /*
   * note that order books will always be ordered from least to greatest
   * [100,101,102,103,104,105,106]
   */

  struct level **lvls = (side) ? &t->buys : &t->sells;
  int64_t *count = (side) ? &t->buys_len : &t->sells_len;

  // there are no elements in this list
  if (*count == 0) {
    // no prices in lvls so just make a new one
    *lvls = realloc(*lvls, (uint64_t)((*count) + 1) * sizeof(struct level));
    (*lvls)[0].price = price;
    (*lvls[0]).quantity = quantity;
    *count = (*count) + 1;
    return;
  }

  // Binary search of lvls to find the price element
  // to update it or to create a new level
  int64_t start = 0;
  int64_t end = *count;
  while (start <= end) {
    int64_t cur = (start + end) / 2;

    if (cur >= *count)
      break;

    // because of fixed point math, we can compare these
    // floating point numbers
    if (((*lvls)[cur]).price == price) {
      if (quantity != 0) {
        // don't delete just update the quantity
        ((*lvls)[cur]).quantity = quantity;
        return;
      } else {
        // delete the lvl
        remove_lvl(lvls, cur, *count);
        *count = (*count) - 1;
        return;
      }
    } else if (((*lvls)[cur]).price < price) {
      start = cur + 1;
      continue;
    } else if (((*lvls)[cur]).price > price) {
      end = cur - 1;
      continue;
    }
  }

  struct level tmp;
  tmp.price = price;
  tmp.quantity = quantity;

  insert_at(lvls, (start == end) ? (*count) : (*count), start, tmp);
  *count += 1;
}

void book_free(struct book **t) {
  if ((*t)->buys_len != 0)
    free((*t)->buys);
  (*t)->buys = NULL;

  if ((*t)->sells_len != 0)
    free((*t)->sells);
  (*t)->sells = NULL;

  free(*t);
  *t = NULL;
}
