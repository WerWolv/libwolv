#pragma once

#define WOLV_STRINGIFY_IMPL(x)          #x
#define WOLV_STRINGIFY(x)               WOLV_STRINGIFY_IMPL(x)

#define WOLV_TOKEN_CONCAT_IMPL(x, y)    x##y
#define WOLV_TOKEN_CONCAT(x, y)         WOLV_TOKEN_CONCAT_IMPL(x, y)

#define WOLV_ANONYMOUS_VARIABLE(prefix) WOLV_TOKEN_CONCAT(prefix, __COUNTER__)