#ifndef OANDA_REQUEST_BUILDER_
#define OANDA_REQUEST_BUILDER_

#include <error_codes.h>
#include <string_builder.h>
#include <tracer.h>

enum RISKI_ERROR_CODE oanda_v20_v3_accounts(char* host, char* api_key,
                                            char** res);

#endif
