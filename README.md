
Ember - Embedded Terminal
=========================

Introduction
------------

Ember is a C++ library with a plain C API that makes it easy to embed
a text mode terminal in anything.

Example
-------

See example.c.  Build it, run it, then:
telnet localhost 10000

Welcome to the ember example
emb> hello
Hello, world!
emb> brodacast "This message will get sent to everyone"
Broadcast message: This message will get sent to everyone
