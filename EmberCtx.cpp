
#include "EmberCtx.h"
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

EmberCtx::EmberCtx(ember_opt *options) :
    m_options(*options),
    m_listenSock(-1),
    m_lastError(EMBER_OK),
    m_lastErrorText(NULL)
{
    Init();
}

EmberCtx::~EmberCtx()
{
}

void EmberCtx::Init()
{
    if(!m_options.allocFn) {
        m_options.allocFn = malloc;
    }

    if(!m_options.freeFn) {
        m_options.freeFn = free;
    }
}

ember_err EmberCtx::Start()
{
    ember_err err = EMBER_OK;
    if(m_options.listenPort) {
        err = StartListening(m_options.listenPort);
    }
    return err;
}

ember_err EmberCtx::StartListening(int port)
{
    m_listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if(m_listenSock < 0) {
        return SetLastError(EMBER_CANT_CREATE_SOCKET, strerror(errno));
    }

    int enable = 1;
    if(setsockopt(m_listenSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) != 0) {
        return SetLastError(EMBER_CANT_SETSOCKOPT, strerror(errno));
    }

    struct sockaddr_in addr;
    memset(&addr, sizeof(addr), 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if(bind(m_listenSock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        return SetLastError(EMBER_BIND_ERROR, strerror(errno));
    }

    if(listen(m_listenSock, 10) < 0) {
        return SetLastError(EMBER_LISTEN_ERROR, strerror(errno));
    }
}

void EmberCtx::AddCommand(const char *name, ember_cmd_fcn fcn, const char *argFmt, const char *helpText)
{
}

int EmberCtx::Poll(int timeoutMS)
{
    return 0;
}

ember_err EmberCtx::SetLastError(ember_err err, const char *text)
{
    if(m_lastErrorText) {
        m_options.freeFn(m_lastErrorText);
    }
    m_lastErrorText = (char *)m_options.allocFn(strlen(text) + 1);
    strcpy(m_lastErrorText, text);
    m_lastError = err;
}

ember_err EmberCtx::GetLastError(char *buf, size_t size)
{
    if(!m_lastErrorText) {
        if(buf && size > 0) {
            buf[0] = 0;
        }
    } else {
        strncpy(buf, m_lastErrorText, size);
    }
    return m_lastError;
}
