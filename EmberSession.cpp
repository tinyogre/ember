
#include "ember.h"
#include "EmberSession.h"
#include "EmberCtx.h"
#include "EmberString.h"

#include <unistd.h>
#include <stdio.h>

using namespace ember;

EmberSession::EmberSession(EmberCtx *ctx, int sock) :
    m_ctx(ctx),
    m_next(NULL),
    m_sock(sock),
    m_inputBuf(NULL),
    m_inputBufSize(0),
    m_inputBufUsed(0)
{
}

EmberSession::~EmberSession()
{
}

void EmberSession::Send(const char *buf, size_t len)
{
    if(m_sock >= 0) {
        write(m_sock, buf, len);
    }
}

void EmberSession::VPrint(const char *fmt, va_list vl)
{
    char buf[MAX_OUTPUT_LINE];
    vsnprintf(buf, sizeof(buf), fmt, vl);
    Send(buf, strlen(buf));
}

void EmberSession::Print(const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    char buf[MAX_OUTPUT_LINE];
    vsnprintf(buf, sizeof(buf), fmt, vl);
    va_end(vl);
    Send(buf, strlen(buf));
}

void EmberSession::VBroadcast(const char *fmt, va_list vl)
{
}

void EmberSession::Reserve(size_t bytes)
{
    if(!m_inputBuf) {
        m_inputBufSize = 256;
        m_inputBufUsed = 0;
        m_inputBuf = (char *)m_ctx->GetOptions().allocFn(m_inputBufSize);
        m_inputBuf[0] = 0;
    }
    if(bytes >= m_inputBufSize) {
        size_t newSize = m_inputBufSize * 2;
        char *newBuf = (char *)m_ctx->GetOptions().allocFn(newSize);
        memcpy(newBuf, m_inputBuf, m_inputBufUsed + 1);
        m_inputBuf = newBuf;
        m_inputBufSize = newSize;
    }
}

void EmberSession::AddInput(const char *buf, size_t bytes)
{
    Reserve(m_inputBufUsed + bytes + 1);

    strncpy(m_inputBuf + m_inputBufUsed, buf, bytes);
    m_inputBufUsed += bytes;
    m_inputBuf[m_inputBufUsed] = 0;

    for(int c = 0; c < m_inputBufUsed; c++) {
        if(m_inputBuf[c] == '\r' || m_inputBuf[c] == '\n') {
            m_inputBuf[c] = 0;
            m_ctx->ExecuteCommand(this, m_inputBuf);
            ++c;
            while(m_inputBuf[c] == '\r' || m_inputBuf[c] == '\n') {
                ++c;
            }
            if(m_inputBufUsed - c > 0) {
                memmove(m_inputBuf, &m_inputBuf[c], m_inputBufUsed - c);
            }
            m_inputBufUsed -= c;
            c = -1;
        }
    }
}

void EmberSession::DoRead()
{
    char buf[1024];
    int r = read(m_sock, buf, sizeof(buf));
    if(r > 0) {
        AddInput(buf, (size_t)r);
    }
}

void EmberSession::DoWrite()
{
}

bool EmberSession::PendingWrites()
{
    return false;
}
