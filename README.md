
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

Any number of commands can be registered.  Each command is a function
that takes an opaque session type and main() style argc/argv
arguments and returns an int, where 0 indicates "success" and anything
else is an error.
 
I don't have a particular application for which I am going to use
Ember at this point in time, but it's the kind of thing I (and others)
have written and rewritten in different forms for different projects
several times over the years.  Next time, I'll be ready.  Maybe you
will be too!

Building
--------
Ember uses scons as its build system, which you can get from http://scons.org

To build everything:

$ scons

Besides the library, that will build the examples in the examples
directory, as well as a simple curses based client in the client
directory.

Currently, Ember has been tested on MacOS 10.7 and Linux (Mint 12).  I
do intend to support Windows soon.

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

Currently, Ember opens a TCP listen port and manages connections
itself.  I plan to add more communication methods, as well as allowing
the program to provide alternate communication channels itself.

Known Bugs
----------

Does not yet handle non-blocking writes properly, data will be lost if
sending faster than the socket can accept data.

There's no way to stop log messages from going to stdout yet.

Example
-------
This is the simplest command that actually does something:

    static int Hello(ember_session ctx, int argc, const char **argv)
    {
        ember_print(ctx, "Hello, world!\n");
        return 0;
    }

That command could be registered like this:

    ember_ctx ctx = ember_init(&options);
    ember_add_command(ctx, "Hello", Hello, NULL, "Print the standard greeting");

For a full working example see examples/hello.c.  Build it ("scons example"), 
run it ("examples/hello"), then: telnet localhost 10000

    Welcome to the ember example
    emb> hello
    Hello, world!
    emb> echo "This is a quoted argument" and these are not.
    1: This is a quoted argument
    2: and
    3: these
    4: are
    5: not.
    emb> 

Future
------

One of ember's goals is to be accesible using plain old telnet or nc
or any plain text terminal.  However, a client such as the one in the
client directory may be able to provide more advanced input methods
and can make your life easier.  I would like to provide readline-like
functionality even when connected directly with telnet, but it was
turning out to be trickier than I thought, so I wrote the client
program as a workaround.  It doesn't yet have any actual readline like
functionality, but it using curses for screen I/O.  It's a start.
