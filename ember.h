
#ifndef EMBER_H__
#define EMBER_H__

#ifdef cplusplus__
extern "C" {
#endif

    typedef void *ember_ctx;
    typedef void *ember_cmd_ctx;
    typedef void *ember_socket;

    // All commands are implemented by a function of this form
    typedef int (*ember_cmd_fcn)(ember_cmd_ctx ctx, int argc, char **argv);

    typedef size_t (*ember_read_fcn)(void *conn, char *buf, size_t size);
    typedef size_t (*ember_write_fcn)(void *conn, char *buf, size_t size);
    typedef int (ember_poll_fcn)(void **conns, size_t count);

    typedef struct {
        const char *greeting;   // Sent to all clients on initial connect
        const char *prompt;     // Sent whenever ready to read a new command
        int listenPort;         // If ember is managing connections, port to listen on
        ember_read_fcn readFn;  // Functions ember should call to read or write from application managed connections 
        ember_write_fcn writeFn;
        ember_poll_fcn pollFn;
    } ember_opt;
    
    // Initialize options structure to default values
    void ember_opt_init(ember_opt *options);

    // Create a new ember context with the specified options
    ember_ctx ember_init(ember_opt *options);

    // Add a new command
    void ember_add_command(ember_ctx ctx, const char *name, ember_cmd_fcn fcn, const char *argFmt, const char *helpText);
    // Call periodically to process I/O
    void ember_poll(ember_ctx ctx, int timeoutMS);

    // Send text to whoever issued the command
    void ember_print(ember_cmd_ctx ctx, const char *fmt, ...);

    // Send text to every connected terminal
    void ember_broadcast(ember_cmd_ctx ctx, const char *fmt, ...);

#ifdef cplusplus__
}
#endif

#endif
