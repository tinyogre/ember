
#include "EmberCtx.h"
#include "EmberMem.h"
#include "EmberSession.h"
#include "EmberCommand.h"
#include "EmberString.h"
#include "StandardCommands.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <poll.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

using namespace ember;

EmberCtx::EmberCtx(ember_opt *options) :
    m_options(*options),
    m_listenSock(-1),
    m_lastError(EMBER_OK),
    m_lastErrorText(NULL),
    m_sessionsHead(NULL),
    m_numSessions(0)
{
    Init();
}

EmberCtx::~EmberCtx()
{
    if(m_argvBuffer) {
        Free(m_argvBuffer);
    }
    if(m_argvArray) {
        Free(m_argvArray);
    }
    while(m_sessionsHead) {
        EmberSession *next = m_sessionsHead->m_next;
        m_sessionsHead->~EmberSession();
        Free(m_sessionsHead);
        m_sessionsHead = next;
    }

    EmberCommandMap::iterator it(m_cmds);
    for(it.First(); it.Current(); it.Next()) {
        EmberCommand *cmd = it.Current();
        m_cmds->Remove(it);
        cmd->~EmberCommand();
        Free(cmd);
    }

    Free(m_cmds);
    close(m_listenSock);
    if(m_lastErrorText) {
        Free(m_lastErrorText);
    }
}

void EmberCtx::Init()
{
    if(!m_options.allocFn) {
        m_options.allocFn = malloc;
    }

    if(!m_options.freeFn) {
        m_options.freeFn = free;
    }

    m_cmds = (StringMap<EmberCommand> *)Alloc(sizeof(StringMap<EmberCommand>));
    new(m_cmds) StringMap<EmberCommand>(this, 32);

    if(!m_options.noStandardCommands) {
        AddStandardCommands(this);
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

static int setNonblocking(int fd)
{
    int flags;

    /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
    /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    /* Otherwise, use the old way of doing it */
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
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

    setNonblocking(m_listenSock);

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
    EmberCommand *cmd = (EmberCommand *)Alloc(sizeof(EmberCommand));
    new(cmd) EmberCommand(this, name, fcn, argFmt, helpText);
    m_cmds->Insert(cmd);
}

ember_err EmberCtx::DoAccept()
{
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int newSock = accept(m_listenSock, (struct sockaddr *)&addr, &addrlen);
    if(newSock < 0) {
        return SetLastError(EMBER_ACCEPT_ERROR, strerror(errno));
    }

    setNonblocking(newSock);

    EmberSession *session = (EmberSession *)m_options.allocFn(sizeof(EmberSession));
    new(session) EmberSession(this, newSock);
    session->m_next = m_sessionsHead;
    m_sessionsHead = session;
    ++m_numSessions;
    Log("DoAccept: newSock = %d\n", newSock);

    return EMBER_OK;
}

void EmberCtx::Disconnect(EmberSession *session)
{
    Log("Disconnect: session %p, sock=%d\n", session, session->m_sock);
    EmberSession *prev = NULL;
    for(EmberSession *chk = m_sessionsHead; chk; chk = chk->m_next) {
        if(chk == session) {
            if(prev) {
                prev->m_next = session->m_next;
            } else {
                m_sessionsHead = session->m_next;
            }
            break;
        }
    }
    session->~EmberSession();
    Free(session);
    m_numSessions--;
}

int EmberCtx::Poll(int timeoutMS)
{
    if(m_options.pollFn) {
        // NYI
        return 0;
    }

    int numfds = m_numSessions + 1; // +1 for listenSock
    struct pollfd *fds = (struct pollfd *)alloca(sizeof(pollfd) * numfds);
    EmberSession **sessions = (EmberSession **)alloca(sizeof(EmberSession *) * numfds);
    
    fds[0].fd = m_listenSock;
    fds[0].events = POLLIN;
    sessions[0] = NULL;
    EmberSession *session = m_sessionsHead;
    for(int i = 0; i < m_numSessions; i++) {
        fds[i+1].fd = session->m_sock;
        fds[i+1].events = POLLIN;
        if(session->PendingWrites()) {
            fds[i+1].events |= POLLOUT;
        }

        sessions[i + 1] = session;
        session = session->m_next;
    }
    int ret = poll(fds, numfds, timeoutMS);
    if(ret > 0) {
        for(int i = 0; i < numfds; i++) {
            if(fds[i].revents & (POLLHUP | POLLERR)) {
                if(sessions[i]) {
                    Disconnect(sessions[i]);
                    sessions[i] = NULL;
                    continue;
                }
            }
            if(fds[i].revents & (POLLIN | POLLERR | POLLHUP)) {
                if(fds[i].fd == m_listenSock) {
                    DoAccept();
                } else if(sessions[i]) {
                    if(sessions[i]->DoRead()) {
                        sessions[i] = NULL;
                    }
                }
            }
            if(fds[i].revents & POLLOUT) {
                Log("POLLOUT: %d\n", fds[i].fd);
                if(fds[i].fd != m_listenSock && sessions[i]) {
                    if(sessions[i]->DoWrite()) {
                        sessions[i] = NULL;
                    }
                }
            }
        }
    }
    return ret;
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

void EmberCtx::Log(const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);

    // FIXME should be possible to use this with no stdio, get a user provided log function
    // (But provide a default shim to stdio)
    vfprintf(stdout, fmt, vl);
    va_end(vl);
}

void EmberCtx::ExecuteCommand(EmberSession *session, const char *str)
{
    // Count maximum possible number of args quickly by counting spaces, which should always equal or exceed
    // the actual number of arguments.  Just need to make sure our array is big enough.
    size_t len = 0;
    int maxargs = 1;
    for(const char *c = str; *c; c++) {
        len++;
        if(isspace(*c)) {
            maxargs++;
            while(isspace(*(c+1))) {
                c++;
            }
        }
    }

    if(m_argvBufferSize < len + 1) {
        if(m_argvBuffer) {
            Free(m_argvBuffer);
        }
        m_argvBuffer = (char *)Alloc(len + 1);
        m_argvBufferSize = len + 1;
    }
    strcpy(m_argvBuffer, str);

    if(m_argvArraySize < maxargs) {
        if(m_argvArray) {
            Free(m_argvArray);
        }
        m_argvArray = (const char **)Alloc(sizeof(const char **) * maxargs);
        m_argvArraySize = maxargs;
    }

    // FIXME this does stupid string shifting for quotes and such, why didn't I just
    // copy it as I parsed it instead of using strcpy above?

    int state = 0;
    int argc = 0;
    char *currArgStart = m_argvBuffer;

    // Ignore leading whitespace
    while(isspace(*currArgStart))
        currArgStart++;

    m_argvArray[argc] = currArgStart;
    argc = 1;
    for(char *c = currArgStart; *c; c++) {
        switch(state) {
            case 0:
                if(isspace(*c)) {
                    *c = 0;
                    while(isspace(*(c+1))) {
                        c++;
                    }
                    currArgStart = c + 1;
                    embASSERT(argc < m_argvArraySize);
                    m_argvArray[argc] = currArgStart;
                    argc++;
                } else if(*c == '"') {
                    state = 1;
                    strerase(c, 1);
                    c--;
                } else if(*c == '\'') {
                    state = 2;
                    strerase(c, 1);
                    c--;
                }
                break;
            case 1:
                // Reading double quoted argument
                if(*c == '"') {
                    state = 0;
                    strerase(c, 1);
                    c--;
                } else if(*c == '\\') {
                    state = 3;
                    strerase(c, 1);
                    c--;
                }
                break;
            case 2:
                // Reading single quoted argument
                if(*c == '\'') {
                    state = 0;
                    strerase(c, 1);
                    c--;
                } else if(*c == '\\') {
                    state = 4;
                    strerase(c, 1);
                    c--;
                }
                break;
            case 3:
                // Double quote, \\ seen
                // FIXME handle \n, \t, etc.
                state = 1;
                break;
            case 4:
                // Single quote, \\ seen
                // FIXME handle \n, \t, etc.
                state = 2;
                break;
            default:
                embASSERT(false);
                break;
        }
    }
                    
    Log("ExecuteCommand: %s\n", str);
    Log("ExecuteCommand: %d args\n", argc);
    for(int a = 0; a < argc; a++) {
        Log("Arg %d: \"%s\"\n", a, m_argvArray[a]);
    }
    
    EmberCommand *cmd = m_cmds->Find(m_argvArray[0]);
    if(!cmd) {
		if(m_options.errorFn) {
			m_options.errorFn(session, argc, m_argvArray);
		} else {
			if(m_argvArray[0][0]) {
				session->Print("Unknown command \"%s\"\n", m_argvArray[0]);
			}
		}
    } else {
        cmd->Execute(session, argc, m_argvArray);
    }
}

void *EmberCtx::Alloc(size_t size)
{
    return m_options.allocFn(size);
}

void EmberCtx::Free(void *ptr)
{
    m_options.freeFn(ptr);
}

char *EmberCtx::StrDup(const char *src)
{
    if(!src) {
        return NULL;
    }

    char *ptr = (char *)Alloc(strlen(src) + 1);
    strcpy(ptr, src);
    return ptr;
}

void EmberCtx::SendHelp(EmberSession *sess)
{
    EmberCommandMap::iterator it(m_cmds);
    for(it.First(); it.Current(); it.Next()) {
        sess->Print("%20s %20s - %s\n", it->GetKey(NULL), it->m_argFmt ? it->m_argFmt : "", it->m_helpText ? it->m_helpText : "");
    }
}

void EmberCtx::SendBroadcast(const char *text)
{
    for(EmberSession *sess = m_sessionsHead; sess; sess = sess->m_next) {
        sess->Send(text);
    }
}
