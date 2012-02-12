
#ifndef EMBER_COMMAND_H__
#define EMBER_COMMAND_H__

#include "EmberMap.h"

class EmberCommand : public SimpleStringMapNode<EmberCommand>
{
public:
    EmberCtx *m_ctx;
    ember_cmd_fcn m_fcn;
    char *m_argFmt;
    char *m_helpText;

    EmberCommand(EmberCtx *ctx, const char *name, ember_cmd_fcn fcn, const char *argFmt, const char *helpText) :
        SimpleStringMapNode<EmberCommand>(ctx, name),
        m_ctx(ctx),
        m_fcn(fcn),
        m_argFmt(ctx->StrDup(argFmt)),
        m_helpText(ctx->StrDup(helpText))
    {
    }

    int Execute(EmberSession *session, int argc, const char **argv) {
        return m_fcn(session, argc, argv);
    }

};

typedef StringMap<EmberCommand> EmberCommandMap;

#endif
