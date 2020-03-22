#include <security/search.h>

/*
 * What each element is
 * @param {char*} symbol The symbol; eg AAPL
 * @param {char*} full_name The entire company name; eg APPLE INC
 */
struct csv_data {
  char* symbol;
  char* full_name;
};

/*
 * Holds the array of data for each stock entry
 * @param {size_t} n The number of elements
 * @param {struct csv_data*} data The list of n elements size
 */
struct db {
  size_t n;
  struct csv_data* data;
};

struct db db_search;

enum RISKI_ERROR_CODE search_init(char* csv_db) {
  PTR_CHECK(csv_db, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  FILE* fp = fopen(csv_db, "r");
  PTR_CHECK(fp, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  db_search.data = NULL;
  db_search.n = 0;

  // seems to be big enough...
  char line[1024];

  while (fgets(line, 1024, fp)) {
    char* tok = NULL;
    db_search.n += 1;
    db_search.data =
        (struct csv_data*)(realloc(db_search.data, sizeof(struct csv_data) * db_search.n));

    PTR_CHECK(db_search.data, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

    tok = strtok(line, ",");
    db_search.data[db_search.n - 1].symbol = strdup(tok);
    tok = strtok(NULL, ",");
    db_search.data[db_search.n - 1].full_name = strdup(tok);
  }

  fclose(fp);
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE search_search(char* seq, char** json) {
  PTR_CHECK(seq, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(json, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(db_search.data, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct string_builder* sb = NULL;
  TRACE(string_builder_new(&sb));

  TRACE(string_builder_append(sb, "[", 1));

  for (size_t i = 0; i < db_search.n; ++i) {
    if (strncmp(seq, db_search.data[i].symbol, strlen(seq)) == 0) {
      printf("%s->?\t%s\n", db_search.data[i].symbol, db_search.data[i].symbol);
      TRACE(string_builder_append(sb, "{\"symbol\":\"", 9));
      TRACE(string_builder_append(sb, db_search.data[i].symbol,
                                  strlen(db_search.data[i].symbol)));
      TRACE(string_builder_append(sb, "\",\"fullName\":\"", 12));
      TRACE(string_builder_append(sb, db_search.data[i].full_name,
                                  strlen(db_search.data[i].full_name)));
      TRACE(string_builder_append(sb, "\"},", 3));
    }
  }

  TRACE(string_builder_append(sb, "]", 1));

  char* dat = NULL;
  TRACE(string_builder_str(sb, &dat));
  TRACE(string_builder_free(&sb));

  size_t len = strlen(dat);
  dat[len-2] = ']';
  dat[len-1] = '\x0';

  *json = dat;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE search_free() {
  for (size_t i = 0; i < db_search.n; ++i) {
    free(db_search.data[i].full_name);
    free(db_search.data[i].symbol);
  }
  free(db_search.data);
  return RISKI_ERROR_CODE_NONE;
}
