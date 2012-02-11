
#include "ember.h"

static int s_done = 0;

static int Hello(ember_cmd_ctx ctx, int argc, char **argv)
{
    ember_print(ctx, "Hello, world!\n");
    return 0;
}

static int Broadcast(ember_cmd_ctx ctx, int argc, char **argv)
{
    if(argc < 2) {
        ember_print(ctx, "broadcast requires an argument\n");
    }

    ember_broadcast(ctx, "Broadcast message: %s\n", argv[1]);
    return 0;
}

static int Quit(ember_cmd_ctx, int argc, char **argv)
{
    s_done = 1;
    return 1;
}

int main(int, char **)
{
    ember_opt options;
    ember_opt_init(&options);
    options.greeting = "Welcome to the ember example\n";
    options.prompt   = "emb> ";
    options.port     = 10000;
    
    ember_ctx ctx = ember_init(&options);
    ember_add_command(ctx, "Hello", Hello, NULL, "Print the standard greeting");
    ember_add_command(ctx, "Broadcast", Broadcast, "<s/text>", "Send a message to all connected terminals");
    ember_add_command(ctx, "Quit", Quit, NULL, "Quit the example program");

    while(!s_done) {
        // Wait one second for events
        ember_poll(ctx, 1000);
        
    }
}
