
#include "ember.h"
#include "EmberSession.h"
#include "EmberCtx.h"
#include "EmberString.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#if EMBER_USE_CURSES
#include <curses.h>
#endif

using namespace ember;

EmberSession::EmberSession(EmberCtx *ctx, int sock) :
    m_ctx(ctx),
    m_next(NULL),
    m_sock(sock),
    m_inputBuf(NULL),
    m_inputBufSize(0),
    m_inputBufUsed(0)
{
    if(ctx->GetOptions().greeting) {
        Send(ctx->GetOptions().greeting);
    }

    if(ctx->GetOptions().prompt) {
        Send(ctx->GetOptions().prompt);
    }

#if EMBER_USE_CURSES
	m_cursesFile = fdopen(m_sock, "r+b");
	if(m_cursesFile) {
		m_screen = newterm((char *)"vt100", m_cursesFile, m_cursesFile);
		set_term(m_screen);
		nonl();
		cbreak();
		noecho();
	}
#endif
}

EmberSession::~EmberSession()
{
    if(m_sock >= 0) {
        close(m_sock);
    }
    if(m_inputBuf) {
        m_ctx->Free(m_inputBuf);
    }
}

void EmberSession::Send(const char *buf, int len)
{
    if(len < 0) {
        len = strlen(buf);
    }

    if(len > 0 && m_sock >= 0) {
#if EMBER_USE_CURSES
		SCREEN *old = set_term(m_screen);
		addstr(buf);
		set_term(old);
#else
        write(m_sock, buf, len);
#endif
    }
}

void EmberSession::VPrint(const char *fmt, va_list vl)
{
    char buf[MAX_OUTPUT_LINE];
    vsnprintf(buf, sizeof(buf), fmt, vl);
    Send(buf);
	Send("\n\r");
}

void EmberSession::Print(const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    char buf[MAX_OUTPUT_LINE];
    vsnprintf(buf, sizeof(buf), fmt, vl);
    va_end(vl);
    Send(buf);
	Send("\n\r");
}

void EmberSession::VBroadcast(const char *fmt, va_list vl)
{
    char buf[MAX_OUTPUT_LINE];
    vsnprintf(buf, sizeof(buf), fmt, vl);
    m_ctx->SendBroadcast(buf);
	m_ctx->SendBroadcast("\r\n");
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

	//printf("%d:%s\n", (int)m_inputBufUsed, m_inputBuf);

    // If multiple commands are parsed, don't send a new prompt until after the last one
    bool executed = false;
    for(int c = 0; c < m_inputBufUsed; c++) {
        if(m_inputBuf[c] == '\r' || m_inputBuf[c] == '\n') {
            m_inputBuf[c] = 0;
            m_ctx->ExecuteCommand(this, m_inputBuf);
            executed = true;
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
    if(executed && m_ctx->GetOptions().prompt) {
        Send(m_ctx->GetOptions().prompt);
    }
}

bool EmberSession::DoRead()
{
    char buf[1024];
#if EMBER_USE_CURSES
	SCREEN *old = set_term(m_screen);
	int c = getch();
	if(c == ERR) {
		if(feof(m_cursesFile)) {
            m_ctx->Log("Disconnect: session %p: (%d) %s", this, errno, strerror(errno));
            m_ctx->Disconnect(this);
			set_term(old);
			return true;
		}
	} else {
		buf[0] = c;
		buf[1] = 0;
		AddInput(buf, 1);
	}
	set_term(old);
#else
    int r = read(m_sock, buf, sizeof(buf));
    if(r > 0) {
		buf[r] = 0;
        AddInput(buf, (size_t)r);
    } else if(r < 0) {
        if(errno == EINTR || errno == EAGAIN) {
            return false;
        } else {
            m_ctx->Log("Disconnect: session %p: (%d) %s", this, errno, strerror(errno));
            m_ctx->Disconnect(this);
            // this object is now deleted, don't use it any more!
            return true;
        }
    }
#endif
    return false;
}

bool EmberSession::DoWrite()
{
    return false;
}

bool EmberSession::PendingWrites()
{
    return false;
}
