
env = Environment();
lib_srcs = Split("""
ember.cpp
EmberCtx.cpp
EmberSession.cpp
EmberString.cpp
StandardCommands.cpp
""")

env.Append(CPPFLAGS=['-g'], LINKFLAGS=['-g'], CPPPATH=['#'])

lib = env.StaticLibrary('ember', lib_srcs);

env.Alias('all', lib)
env.Default(lib)

env.Export(['env'])
env.SConscript('examples/SConscript')
env.SConscript('client/SConscript')
