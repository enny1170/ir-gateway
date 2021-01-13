Import("env")

my_flags = env.ParseFlags(env['BUILD_FLAGS'])
defines = {k: v for (k, v) in my_flags.get("CPPDEFINES")}
print(defines)

env.Replace(PROGNAME="%s_V%s_%s_%s" % (defines.get("DEVTYPE"), defines.get("VERSION_MAJOR"),defines.get("VERSION_MINOR"),defines.get("VERSION_BUILD")))