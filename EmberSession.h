
#ifndef EMBER_SESSION_H__
#define EMBER_SESSION_H__

#include <stdarg.h>

class EmberCtx;

class EmberSession
{
protected:
    EmberCtx *m_ctx;

public:
    EmberSession(EmberCtx *ctx);
    ~EmberSession();

    void VPrint(const char *fmt, va_list vl);
    void VBroadcast(const char *fmt, va_list vl);
};

#endif
