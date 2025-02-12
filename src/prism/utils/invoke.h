#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t pad[72];
} CContextTypes;

#ifdef __cplusplus
extern "C" {
#endif
typedef CContextTypes (*InvokeFunc)();
CContextTypes invoke(InvokeFunc native_code, CContextTypes *args, size_t length);
#ifdef __cplusplus
}
#endif