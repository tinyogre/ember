
// Don't include this file directly, include ember.h

#ifndef DEFINE_EMBER_STRINGS
#define EMERR(errcode) EMBER_##errcode
typedef enum
#else
#undef EMERR
#define EMERR(errcode) "EMBER_" # errcode
const char *s_wocket_errors[] =
#endif
{
    EMERR(OK),
    EMERR(NOT_IMPLEMENTED),
    EMERR(UNINITIALIZED),
    EMERR(OUT_OF_MEMORY),
    EMERR(CANT_CREATE_SOCKET),
    EMERR(BIND_ERROR),
    EMERR(LISTEN_ERROR),
    EMERR(ACCEPT_ERROR),
    EMERR(CANT_SETSOCKOPT),
}
#ifndef DEFINE_EMBER_STRINGS
ember_err
#endif
;
