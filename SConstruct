#!/usr/bin/env python
import tools.post_action as post_action
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


cpp_paths = ["./",
             "src/",
             "src/transitions/",
             "src/transitions/variable_expressions/",
             ]
sources = Glob("*.cpp") + \
    Glob("src/*.cpp") + \
    Glob("src/transitions/*.cpp") + \
    Glob("src/transitions/variable_expressions/*.cpp")

if env["target"] == "editor":
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

if not env.dev_build:
    env["bin_file_name_base"] = bin_file_name_base

    # def finish(target, source, env):
    #     if os.path.exists("bin/" + env["bin_file_name_base"]):
    #         os.remove("bin/" + env["bin_file_name_base"])

    #     os.rename("build/" + env["bin_file_name_base"],
    #               "bin/" + env["bin_file_name_base"])
    #     print("=== POST")

    # finish_command = env.Command('finish', [], finish)

    # BUILD_TARGETS
    import tools.post_action as post_action
    finish_command = env.Command('finish', [], post_action.finish)
    env.AddPostAction(finish_command, BUILD_TARGETS)

    if 'finish' not in BUILD_TARGETS:
        BUILD_TARGETS.append('finish')

Default(library)
