#include <string_builder.h>

struct string_builder {
  // The allocated size of the buffer "c"
  size_t allocated_size;

  // The length of the current string
  size_t len;

  // The character buffer
  char* c;
};

enum RISKI_ERROR_CODE string_builder_new(struct string_builder** sb) {
  struct string_builder* sb_ =
      (struct string_builder*)malloc(1 * sizeof(struct string_builder));

  PTR_CHECK(sb, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);

  sb_->allocated_size = 0;
  sb_->len = 0;
  sb_->c = NULL;

  *sb = sb_;

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE string_builder_append(struct string_builder* sb, char* c,
                                            size_t n) {
  (void)n;
  // Make sure sb is a valid pointer
  PTR_CHECK(sb, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(c, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  // Reallocate if needed
  if (sb->allocated_size == 0) {
    sb->allocated_size = strlen(c) + 1;
    sb->c = strdup(c);
    return RISKI_ERROR_CODE_NONE;
  }
  sb->allocated_size += strlen(c) + 2;
  sb->c = (char*)realloc(sb->c, sb->allocated_size * sizeof(char));
  PTR_CHECK(sb->c, RISKI_ERROR_CODE_MALLOC_ERROR, RISKI_ERROR_TEXT);
  strcat(sb->c, c);
  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE string_builder_str(struct string_builder* sb, char** c) {
  // Make sure sb is a valid pointer
  PTR_CHECK(sb, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  // Make sure c is a valid pointer
  PTR_CHECK(c, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  *c = strdup(sb->c);

  return RISKI_ERROR_CODE_NONE;
}

enum RISKI_ERROR_CODE string_builder_free(struct string_builder** sb) {
  // Make sure sb is a valid pointer
  PTR_CHECK(sb, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(*sb, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  free((*sb)->c);
  free(*sb);
  *sb = NULL;

  return RISKI_ERROR_CODE_NONE;
}
