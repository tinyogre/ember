
#include "ember.h"
#include "EmberCtx.h"
#include "EmberSession.h"

#include <string.h>
#include <stdlib.h>

// For placement new

// Define this ourselves rather than #include <new>, so that we don't force programs
// linking with this library to also link with stdc++
void* operator new(size_t size, void* p)
{
    return p;
}

#define DEFINE_EMBER_STRINGS
#include "ember_err.h"
#undef DEFINE_EMBER_STRINGS

// Initialize options structure to default values
void ember_opt_init(ember_opt *options, size_t size)
{
    memset(options, 0, size);
    options->prompt = "> ";
}

// Create a new ember context with the specified options
ember_ctx ember_init(ember_opt *options)
{
    ember_alloc_fcn alloc = options->allocFn;
    if(!alloc) {
        alloc = malloc;
    }
    EmberCtx *em = static_cast<EmberCtx *>(alloc(sizeof(EmberCtx)));
    new(em) EmberCtx(options);
    return em;
}

ember_err ember_start(ember_ctx ctx)
{
    EmberCtx *em = static_cast<EmberCtx *>(ctx);
    return em->Start();
}

void ember_destroy(ember_ctx ctxIn)
{
    EmberCtx *ctx = static_cast<EmberCtx *>(ctxIn);
    const ember_opt &options = ctx->GetOptions();
    ember_free_fcn efree = options.freeFn;
    ctx->~EmberCtx();
    // options is no longer valid after the destructor call

    efree(ctx);
}

// Add a new command
void ember_add_command(ember_ctx ctx, const char *name, ember_cmd_fcn fcn, const char *argFmt, const char *helpText)
{
    EmberCtx *em = static_cast<EmberCtx *>(ctx);
    em->AddCommand(name, fcn, argFmt, helpText);
}

// Call periodically to process I/O
int ember_poll(ember_ctx ctx, int timeoutMS)
{
    EmberCtx *em = static_cast<EmberCtx *>(ctx);
    em->Poll(timeoutMS);
}

// Send text to whoever issued the command
void ember_print(ember_session ctx, const char *fmt, ...)
{
    EmberSession *cmdCtx = static_cast<EmberSession *>(ctx);
    va_list vl;
    va_start(vl, fmt);
    cmdCtx->VPrint(fmt, vl);
    va_end(vl);
}

// Send text to every connected terminal
void ember_broadcast(ember_session ctx, const char *fmt, ...)
{
    EmberSession *cmdCtx = static_cast<EmberSession *>(ctx);
    va_list vl;
    va_start(vl, fmt);
    cmdCtx->VBroadcast(fmt, vl);
    va_end(vl);
}

ember_err ember_last_error(ember_ctx ctx, char *buf, size_t size)
{
    EmberCtx *em = static_cast<EmberCtx *>(ctx);
    return em->GetLastError(buf, size);
}
