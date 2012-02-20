
#ifndef EMBER_SESSION_H__
#define EMBER_SESSION_H__

#include <stdarg.h>

class EmberCtx;

#define EMBER_USE_CURSES 0

#ifdef EMBER_USE_CURSES
#include <stdio.h>
#include <curses.h>
#endif

class EmberSession
{
protected:
    EmberCtx *m_ctx;
    EmberSession *m_next;
    int m_sock;

#if EMBER_USE_CURSES
	FILE *m_cursesFile;
	SCREEN *m_screen;
#endif

    friend class EmberCtx;
    char *m_inputBuf;
    size_t m_inputBufUsed;
    size_t m_inputBufSize;

    void Reserve(size_t bytes);
    void AddInput(const char *buf, size_t bytes);

    void Send(const char *buf, int len = -1);

public:
    EmberSession(EmberCtx *ctx, int sock);
    ~EmberSession();

    void Print(const char *fmt, ...);
    void VPrint(const char *fmt, va_list vl);
    void VBroadcast(const char *fmt, va_list vl);

    bool DoRead();
    bool DoWrite();
    bool PendingWrites();

    EmberCtx *GetCtx() { return m_ctx; }
};

#endif
