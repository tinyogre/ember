
#ifndef EMBER_CTX_H__
#define EMBER_CTX_H__

#include "ember.h"

class EmberSession;
template <class T> class StringMap;
class EmberCommand;
typedef StringMap<EmberCommand> EmberCommandMap;

class EmberCtx
{
private:
    ember_opt m_options;
    int m_listenSock;

    ember_err m_lastError;
    char *m_lastErrorText;

    EmberSession *m_sessionsHead;
    int m_numSessions;

    EmberCommandMap *m_cmds;

    const char **m_argvArray;
    size_t m_argvArraySize;
    char *m_argvBuffer;
    size_t m_argvBufferSize;

    void Init();
    ember_err StartListening(int port);
    ember_err DoAccept();

public:
    EmberCtx(ember_opt *options);
    ~EmberCtx();

    ember_err Start();

    void AddCommand(const char *name, ember_cmd_fcn fcn, const char *argFmt, const char *helpText);
    int Poll(int timeoutMS);

    const ember_opt &GetOptions() { return m_options; }

    ember_err SetLastError(ember_err err, const char *text);
    ember_err GetLastError(char *buf, size_t size);

    void Log(const char *fmt, ...);

    void ExecuteCommand(EmberSession *session, const char *str);

    void Disconnect(EmberSession *session);

    void *Alloc(size_t size);
    void Free(void *ptr);
    char *StrDup(const char *src);

    void SendHelp(EmberSession *sess);
};

#endif
