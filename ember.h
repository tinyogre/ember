
#ifndef EMBER_H__
#define EMBER_H__

// size_t
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "ember_err.h"

    typedef void *ember_ctx;
    typedef void *ember_session;
    typedef void *ember_socket;

    // All commands are implemented by a function of this form
    typedef int (*ember_cmd_fcn)(ember_session ctx, int argc, const char **argv);

    typedef void *(*ember_alloc_fcn)(size_t size);
    typedef void (*ember_free_fcn)(void *ptr);

    typedef size_t (*ember_read_fcn)(void *conn, char *buf, size_t size);
    typedef size_t (*ember_write_fcn)(void *conn, char *buf, size_t size);
    typedef int (*ember_poll_fcn)(void **conns, size_t count);

    typedef struct {
        const char *greeting;   // Sent to all clients on initial connect
        const char *prompt;     // Sent whenever ready to read a new command
        int listenPort;         // If ember is managing connections, port to listen on
        int noStandardCommands; // If true, don't add standard commands like "help"

		// Memory management functions provided by application (Optional, malloc and free used if not specified)
        ember_alloc_fcn allocFn;
        ember_free_fcn freeFn;

		// Command handler called in case of unknown command (Optional)
		ember_cmd_fcn errorFn;

		// Functions ember should call to read or write from application managed connections
		// (Not Yet Implemented)
        ember_read_fcn readFn;
        ember_write_fcn writeFn;
        ember_poll_fcn pollFn;

		
    } ember_opt;
    
    // Initialize options structure to default values
    void ember_opt_init(ember_opt *options, size_t size);

    // Create a new ember context with the specified options
    ember_ctx ember_init(ember_opt *options);

    // Start the console up
    ember_err ember_start(ember_ctx ctx);

    void ember_destroy(ember_ctx ctx);

    // Add a new command
    void ember_add_command(ember_ctx ctx, const char *name, ember_cmd_fcn fcn, const char *argFmt, const char *helpText);

    // Call periodically to process I/O
    int ember_poll(ember_ctx ctx, int timeoutMS);

    // Send text to whoever issued the command
    void ember_print(ember_session sess, const char *fmt, ...);

    // Send text to every connected terminal
    void ember_broadcast(ember_session sess, const char *fmt, ...);

    // Send list of all commands
    void ember_send_help(ember_session sess);

    ember_err ember_last_error(ember_ctx, char *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
