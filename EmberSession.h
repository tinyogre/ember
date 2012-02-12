
#ifndef EMBER_SESSION_H__
#define EMBER_SESSION_H__

#include <stdarg.h>

class EmberCtx;

class EmberSession
{
protected:
    EmberCtx *m_ctx;
    EmberSession *m_next;
    int m_sock;
    friend class EmberCtx;
    char *m_inputBuf;
    size_t m_inputBufUsed;
    size_t m_inputBufSize;

    void Reserve(size_t bytes);
    void AddInput(const char *buf, size_t bytes);

    void Send(const char *buf, size_t len);

public:
    EmberSession(EmberCtx *ctx, int sock);
    ~EmberSession();

    void Print(const char *fmt, ...);
    void VPrint(const char *fmt, va_list vl);
    void VBroadcast(const char *fmt, va_list vl);

    void DoRead();
    void DoWrite();
    bool PendingWrites();
};

#endif
