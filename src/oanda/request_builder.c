#include <oanda/request_builder.h>

enum RISKI_ERROR_CODE oanda_v20_v3_accounts(char* host, char* api_key,
                                            char** res) {
  PTR_CHECK(res, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct string_builder* sb;
  TRACE(string_builder_new(&sb));

  TRACE(string_builder_append(sb, "GET /v3/accounts HTTP/1.1\r\n"));

  TRACE(string_builder_append(sb, "Host: "));
  TRACE(string_builder_append(sb, host));
  TRACE(string_builder_append(sb, "\r\n"));

  TRACE(string_builder_append(sb, "User-Agent: riski\r\n"));
  TRACE(string_builder_append(sb, "Accept: */*\r\n"));
  TRACE(string_builder_append(sb, "Content-Type: application/json\r\n"));
  TRACE(string_builder_append(sb, "Accept-Datetime-Format: UNIX\r\n"));
  TRACE(string_builder_append(sb, "Authorization: Bearer "));
  TRACE(string_builder_append(sb, api_key));
  TRACE(string_builder_append(sb, "\r\n\r\n"));

  TRACE(string_builder_str(sb, res));
  TRACE(string_builder_free(&sb));

  return RISKI_ERROR_CODE_NONE;
}
