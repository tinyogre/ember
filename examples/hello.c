
#include "ember.h"
#include <stdio.h>

static int s_done = 0;

static int Hello(ember_session ses, int argc, const char **argv)
{
    ember_print(ses, "Hello, world!");
    return 0;
}

static int Broadcast(ember_session ses, int argc, const char **argv)
{
    if(argc < 2) {
        ember_print(ses, "broadcast requires an argument");
    }

    ember_broadcast(ses, "Broadcast message: %s", argv[1]);
    return 0;
}

static int Quit(ember_session ses, int argc, const char **argv)
{
    s_done = 1;
    return 1;
}

static int Error(ember_session ses, int argc, const char **argv)
{
	ember_print(ses, "%s isn't a valid command, don't do that again", argv[0]);
	return 0;
}

int main(int argc, char **argv)
{
    ember_opt options;
    ember_opt_init(&options, sizeof(options));
	// Octal junk is telnet-ese for forcing character mode
	// http://stackoverflow.com/questions/273261/force-telnet-client-into-character-mode
    //options.greeting   = "\377\375\042\377\373\001Welcome to the ember example\n";
	options.greeting   = "Welcome to the ember example\n";
    options.prompt     = "emb> ";
    options.listenPort = 10000;
	options.errorFn    = Error;

    ember_ctx ctx = ember_init(&options);
    ember_add_command(ctx, "Hello", Hello, NULL, "Print the standard greeting");
    ember_add_command(ctx, "Broadcast", Broadcast, "<s/text>", "Send a message to all connected terminals");
    ember_add_command(ctx, "Quit", Quit, NULL, "Quit the example program");

    ember_err err = ember_start(ctx);
    if(err != EMBER_OK) {
        char buf[1024];
        err = ember_last_error(ctx, buf, sizeof(buf));
        printf("Error %d: %s\n", err, buf);
        return err;
    }

    while(!s_done) {
        // Wait one second for events
        ember_poll(ctx, 1000);
        
    }

    ember_destroy(ctx);
}
