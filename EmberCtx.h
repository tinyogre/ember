
#ifndef EMBER_CTX_H__
#define EMBER_CTX_H__

#include "ember.h"

class EmberCtx
{
private:
    ember_opt m_options;
    int m_listenSock;

    ember_err m_lastError;
    char *m_lastErrorText;

    void Init();
    ember_err StartListening(int port);

public:
    EmberCtx(ember_opt *options);
    ~EmberCtx();

    ember_err Start();

    void AddCommand(const char *name, ember_cmd_fcn fcn, const char *argFmt, const char *helpText);
    int Poll(int timeoutMS);

    const ember_opt &GetOptions() { return m_options; }

    ember_err SetLastError(ember_err err, const char *text);
    ember_err GetLastError(char *buf, size_t size);
};

#endif
