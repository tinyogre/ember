
#ifndef EMBER_CMD_CTX_H__
#define EMBER_CMD_CTX_H__

#include <stdarg.h>

class EmberCtx;

class EmberCmdCtx
{
protected:
    EmberCtx *m_ctx;

public:
    EmberCmdCtx(EmberCtx *ctx);
    ~EmberCmdCtx();

    void VPrint(const char *fmt, va_list vl);
    void VBroadcast(const char *fmt, va_list vl);
};

#endif
