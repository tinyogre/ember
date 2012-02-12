
env = Environment();
lib_srcs = Split("""
EmberCtx.cpp
EmberCmdCtx.cpp
ember.cpp
""")

lib = env.StaticLibrary('ember', lib_srcs);

example = env.Program('example.c', LIBS=['ember'], LIBPATH='.')
