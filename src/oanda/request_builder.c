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

enum RISKI_ERROR_CODE oanda_v20_v3_accounts_instruments(char* host,
                                                        char* api_key,
                                                        char** res,
                                                        char* account_id) {
  PTR_CHECK(res, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(account_id, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct string_builder* sb;
  TRACE(string_builder_new(&sb));

  TRACE(string_builder_append(sb, "GET /v3/accounts/"));
  TRACE(string_builder_append(sb, account_id));
  TRACE(string_builder_append(sb, "/instruments HTTP/1.1\r\n"));

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

enum RISKI_ERROR_CODE oanda_v20_v3_accounts_pricing(char* host, char* api_key,
                                                    char** instrument_name,
                                                    size_t num_instruments,
                                                    char* account_id,
                                                    char** res) {
  PTR_CHECK(res, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);
  PTR_CHECK(account_id, RISKI_ERROR_CODE_NULL_PTR, RISKI_ERROR_TEXT);

  struct string_builder* sb;
  TRACE(string_builder_new(&sb));

  TRACE(string_builder_append(sb, "GET /v3/accounts/"));
  TRACE(string_builder_append(sb, account_id));
  TRACE(string_builder_append(sb, "/pricing?instruments="));
  for (size_t i = 0; i < num_instruments; ++i) {
    TRACE(string_builder_append(sb, instrument_name[i]));
    if (i != num_instruments - 1) TRACE(string_builder_append(sb, ","));
  }

  TRACE(string_builder_append(sb, " HTTP/1.1\r\n"));

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
