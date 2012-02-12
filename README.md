
Ember - Embedded Terminal
=========================

v0.1 
----

Copyright (c) 2012, Joe Rumsey

LICENSE: BSD, see LICENSE file

Introduction
------------

Ember's premise is that it should be easy to add a plain text terminal
to any program.

Ember is a C++ library with a plain C API.  It is desined to have no
external dependencies other than the C standard library.  Memory
allocation functions can be supplied and will be used for all memory
management, or malloc/free will be used if the application doesn't
care.  These features are often requirements for use in games.  The
lack of STL and even new and delete may turn some people off from
working with this code, but it has been carefully constructed to be
usable as widely and as simply as possible.

Currently, Ember opens a TCP listen port and manages connections
itself.  I plan to add more communication methods, as well as allowing
the program to provide alternate communication channels itself.

Any number of commands can be registered.  Each command is a function
that takes an opaque session pointer and main() style argc/argv
arguments and returns an int, where 0 indicates "success" and anything
else is an error.
 
Building
--------
Ember uses scons as its build system, which you can get from http://scons.org

To build the library:

$ scons

To build the examples:

$ scons example

Currently, Ember has only been tested on MacOS 10.7, but I intend to
support Linux and Windows soon.

Limitations
-----------

Will not scale well to large numbers of sessions, it is intended to
give a window into a running program for diagnostic and debugging
purposes, and perhaps lightweight automation.  It is not intended as
the primary interface for networked apps.  That said, a primary goal
is also to have a negligible impact on performance of the host app.
As long as it's used sensibly, it should sit quietly out of the way
until you need it.

Ember currently has no threading model and API calls using the same
context are not thread-safe.  All calls using a given context should
either be made from a single thread, or synchronized by the
application such that only one thread is inside any API call at a
time.  It IS safe to use multiple contexts from multiple threads.
Each context is entirely independent, Ember keeps no global state.

Example
-------
This is the simplest command that actually does something:

    static int Hello(ember_session ctx, int argc, const char **argv)
    {
        ember_print(ctx, "Hello, world!\n");
        return 0;
    }

This command could be registered like this:

    ember_ctx ctx = ember_init(&options);
    ember_add_command(ctx, "Hello", Hello, NULL, "Print the standard greeting");

For a full working example see examples/hello.c.  Build it, run it, then:
telnet localhost 10000

Welcome to the ember example
emb> hello
Hello, world!
emb> brodacast "This message will get sent to everyone"
Broadcast message: This message will get sent to everyone
