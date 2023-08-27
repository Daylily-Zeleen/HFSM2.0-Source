#!/usr/bin/env python
import os
import sys
from build import generate_singleton_helper

env = SConscript("gdextension_dependencies/godot-cpp/SConstruct")

# Generate Singleton helper;
api_path = env.get("custom_api_file", "gdextension_dependencies/godot-cpp/gdextension/extension_api.json")
generate_singleton_helper(api_path)

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

cpp_paths = [
    "./",
    "src/",
    "src/transitions/",
    "src/transitions/variable_expressions/",
]
sources = (
    Glob("*.cpp")
    + Glob("src/*.cpp")
    + Glob("src/transitions/*.cpp")
    + Glob("src/transitions/variable_expressions/*.cpp")
)

if env["target"] == "editor" or env["target"] == "template_debug":
    cpp_paths.append("editor/")

    sources = sources + Glob("editor/*.cpp")

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=cpp_paths)

env.Append(LIBS=[])
env.Append(LIBPATH=[])

env.Append(CPPDEFINES=["GDEXTENSION_BUILD", "GDE_COMPATIBILITY_ENABLED"])

# # Require C++20
# if env.get("is_msvc", False):
#     env["CXXFLAGS"]=["/std:c++20"]
# else:
#     env["CXXFLAGS"]=["-std:c++20"]

# 以 dev_build 确定是否为完整版本
if env["dev_build"]:
    pass


def get_bin_file(env):
    if env["platform"] == "macos":
        return "demo/addons/com.daylily_zeleen.hfsm2/bin/libhfsm2.{}.{}.framework/libhfsm2.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        )
    else:
        return "bin/libhfsm2{}{}".format(env["suffix"], env["SHLIBSUFFIX"])


bin_file = get_bin_file(env)

library = env.SharedLibrary(bin_file, source=sources)

Default(library)
