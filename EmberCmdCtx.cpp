
#include "EmberCmdCtx.h"
#include "EmberCtx.h"

EmberCmdCtx::EmberCmdCtx(EmberCtx *ctx)
{
    m_ctx = ctx;
}

EmberCmdCtx::~EmberCmdCtx()
{
}

void EmberCmdCtx::VPrint(const char *fmt, va_list vl)
{
}

void EmberCmdCtx::VBroadcast(const char *fmt, va_list vl)
{
}

