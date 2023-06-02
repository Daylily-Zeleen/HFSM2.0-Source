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


def scan_files(directory, prefix=None, postfix=None):
    files_list = []

    for root, sub_dirs, files in os.walk(directory):
        for special_file in files:
            if postfix:
                if special_file.endswith(postfix):
                    files_list.append(os.path.join(root, special_file))
            elif prefix:
                if special_file.startswith(prefix):
                    files_list.append(os.path.join(root, special_file))
            else:
                files_list.append(os.path.join(root, special_file))

    return files_list


# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["src/",
                    "src/core/",
                    "src/core/transitions/",
                    "src/core/transitions/variable_expressions/",
                    "src/editor/",
                    "src/editor/inspector_plugin/",
                    "./"
                    ])
env.Append(LIBS=[])
env.Append(LIBPATH=[])

sources = Glob("src/*.cpp") + \
    Glob("src/core/*.cpp") + \
    Glob("src/core/transitions/*.cpp") + \
    Glob("src/core/transitions/variable_expressions/*.cpp") + \
    Glob("src/editor/*.cpp") + \
    Glob("src/editor/inspector_plugin/*.cpp")

# # Require C++20
# if env.get("is_msvc", False):
#     env["CXXFLAGS"]=["/std:c++20"]
# else:
#     env["CXXFLAGS"]=["-std:c++20"]

# 以 dev_build 确定是否为完整版本
if env["dev_build"]:
    pass

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "demo/bin/libgdexample.{}.{}.framework/libgdexample.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "demo/bin/libgdexample{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
