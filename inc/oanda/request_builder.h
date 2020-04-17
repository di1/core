#ifndef OANDA_REQUEST_BUILDER_
#define OANDA_REQUEST_BUILDER_

#include <error_codes.h>
#include <string_builder.h>
#include <tracer.h>

#include "analysis/enumations.h"

enum RISKI_ERROR_CODE oanda_v20_v3_accounts(char* host, char* api_key,
                                            char** res);
enum RISKI_ERROR_CODE oanda_v20_v3_accounts_instruments(char* host,
                                                        char* api_key,
                                                        char** res,
                                                        char* account_id);

enum RISKI_ERROR_CODE oanda_v20_v3_accounts_pricing(char* host, char* api_key,
                                                    char** instrument_name,
                                                    size_t num_instruments,
                                                    char* account_id,
                                                    char** res);
#endif
