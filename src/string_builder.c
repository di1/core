#include <string_builder.h>

struct string_builder {
  // The allocated size of the buffer "c"
  size_t allocated_size;

  // The length of the current string
  size_t len;

  // The character buffer
  char* c;
};

enum STRING_BUILDER_ERROR_CODE string_builder_new(struct string_builder** sb) {
  struct string_builder* sb_ = (struct string_builder*)
    malloc(1 * sizeof(struct string_builder));

  PTR_CHECK(sb, STRING_BUILDER_MALLOC_FAILURE, STRING_BUILDER_ERROR_STR); 

  sb_->allocated_size = 0;
  sb_->len = 0;
  sb_->c = NULL;

  *sb = sb_;

  return STRING_BUILDER_NO_ERROR;
}

enum STRING_BUILDER_ERROR_CODE string_builder_append(struct string_builder* sb,
    char* c, size_t n) {
  
  // Make sure sb is a valid pointer
  PTR_CHECK(sb, STRING_BUILDER_INVALID_PTR, STRING_BUILDER_ERROR_STR);

  // Reallocate if needed
  if (sb->len + n > sb->allocated_size) {
    sb->allocated_size = sb->allocated_size * 2;
    sb->c = (char*) realloc(sb->c, sb->allocated_size * sizeof(char));
    PTR_CHECK(sb, STRING_BUILDER_MALLOC_FAILURE, STRING_BUILDER_ERROR_STR);
  }

  // Append the data
  char* ret = strcat(sb->c, c);

  // Verify valid return value
  PTR_CHECK(ret, STRING_BUILDER_STRCAT_ERR, STRING_BUILDER_ERROR_STR);

  return STRING_BUILDER_NO_ERROR;
}

enum STRING_BUILDER_ERROR_CODE string_builder_str(struct string_builder* sb,
    char** c) {
  // Make sure sb is a valid pointer
  PTR_CHECK(sb, STRING_BUILDER_INVALID_PTR, STRING_BUILDER_ERROR_STR);

  // Make sure c is a valid pointer
  PTR_CHECK(c, STRING_BUILDER_INVALID_PTR, STRING_BUILDER_ERROR_STR);

  *c = sb->c;

  return STRING_BUILDER_NO_ERROR;
}

enum STRING_BUILDER_ERROR_CODE string_builder_free(struct string_builder** sb) {
  // Make sure sb is a valid pointer
  PTR_CHECK(sb, STRING_BUILDER_INVALID_PTR, STRING_BUILDER_ERROR_STR);
  PTR_CHECK(*sb, STRING_BUILDER_INVALID_PTR, STRING_BUILDER_ERROR_STR);

  free((*sb)->c);
  free(*sb);
  *sb = NULL;

  return STRING_BUILDER_NO_ERROR;
}
