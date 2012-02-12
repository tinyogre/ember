
#include "EmberSession.h"
#include "EmberCtx.h"

EmberSession::EmberSession(EmberCtx *ctx)
{
    m_ctx = ctx;
}

EmberSession::~EmberSession()
{
}

void EmberSession::VPrint(const char *fmt, va_list vl)
{
}

void EmberSession::VBroadcast(const char *fmt, va_list vl)
{
}

