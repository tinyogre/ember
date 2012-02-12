
env = Environment();
lib_srcs = Split("""
EmberCtx.cpp
EmberSession.cpp
ember.cpp
EmberString.cpp
""")

env.Append(CPPFLAGS=['-g'], LINKFLAGS=['-g'])

lib = env.StaticLibrary('ember', lib_srcs);

example = env.Program('example.c', LIBS=['ember'], LIBPATH='.')
