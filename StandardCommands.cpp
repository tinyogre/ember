
#include "ember.h"

static int Help(ember_session sess, int argc, const char **argv)
{
    ember_send_help(sess);
    return 0;
}

static int Ping(ember_session sess, int argc, const char **argv)
{
    ember_print(sess, "Pong\n");
    return 0;
}

static int Echo(ember_session sess, int argc, const char **argv)
{
    for(int i = 1; i < argc; i++) {
        ember_print(sess, "%d: %s", i, argv[i]);
    }
    return 0;
}

void AddStandardCommands(ember_ctx ctx)
{
    ember_add_command(ctx, "Help", Help, NULL, "List all commands");
    ember_add_command(ctx, "Ping", Ping, NULL, "Respond with \"Pong\"");
    ember_add_command(ctx, "Echo", Echo, NULL, "Echo all arguments back to sender, one per line");
}
