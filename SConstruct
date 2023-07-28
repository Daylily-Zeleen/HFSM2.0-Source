#!/usr/bin/env python
import os
import sys

env = SConscript("../godot-cpp/SConstruct")

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

cpp_paths = ["./",
             "src/",
             "src/transitions/",
             "src/transitions/variable_expressions/",
             ]
sources = Glob("*.cpp") + \
    Glob("src/*.cpp") + \
    Glob("src/transitions/*.cpp") + \
    Glob("src/transitions/variable_expressions/*.cpp")

if env["target"] == "editor" or env["target"] == "template_debug":
    cpp_paths.append("editor/")

    sources = sources + Glob("editor/*.cpp")

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=cpp_paths)

env.Append(LIBS=[])
env.Append(LIBPATH=[])

env.Append(CPPDEFINES=["GDEXTENSION_BUILD"])

# # Require C++20
# if env.get("is_msvc", False):
#     env["CXXFLAGS"]=["/std:c++20"]
# else:
#     env["CXXFLAGS"]=["-std:c++20"]

# 以 dev_build 确定是否为完整版本
if env["dev_build"]:
    pass


lib_name = "hfsm"


def get_bin_file_name_base(env):
    if env["platform"] == "macos":
        return "lib_name.{}.{}.framework/lib_name.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ).replace("lib_name", lib_name)
    else:
        return "lib_name{}{}".format(
            env["suffix"], env["SHLIBSUFFIX"]).replace("lib_name", lib_name)


bin_file_name_base = get_bin_file_name_base(env)

library = env.SharedLibrary("build/" + bin_file_name_base, source=sources)


Default(library)
