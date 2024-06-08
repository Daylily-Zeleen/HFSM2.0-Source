#!/usr/bin/env python
import os
import sys
import build_version


# from build import generate_singleton_helper
env = SConscript("gdextension_dependencies/godot-cpp/SConstruct")

lib_name = "libhfsm2"

# Generate Singleton helper
# api_path = env.get("custom_api_file", "gdextension_dependencies/godot-cpp/gdextension/extension_api.json")
# generate_singleton_helper(api_path)

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
sources = Glob("*.cpp") + Glob("src/*.cpp") + Glob("src/transitions/*.cpp") + Glob("src/transitions/variable_expressions/*.cpp")

if env["target"] == "editor" or env["target"] == "template_debug":
    cpp_paths.append("editor/")

    sources = sources + Glob("editor/*.cpp")

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=cpp_paths)

env.Append(LIBS=[])
env.Append(LIBPATH=[])

env.Append(CPPDEFINES=["GDEXTENSION_BUILD", "GDE_COMPATIBILITY_ENABLED"])

if "debug" in env["target"]:
    env.Append(CPPDEFINES=["TOOLS_ENABLED"])


# # Require C++20
# if env.get("is_msvc", False):
#     env["CXXFLAGS"]=["/std:c++20"]
# else:
#     env["CXXFLAGS"]=["-std:c++20"]


# TODO:: 目前以 dev_build 确定是否为完整版本
if env["dev_build"]:
    pass


def get_bin_file(env):
    output_bin_folder = "bin"
    if env["platform"] == "macos":
        output_bin_folder = "demo/addons/com.daylily_zeleen.hfsm2/bin"
        return f'{output_bin_folder}/{lib_name}.{env["platform"]}.{env["target"]}.framework/{lib_name}.{env["platform"]}.{env["target"]}'
    elif env["platform"] == "ios":
        if env["ios_simulator"]:
            return f'{output_bin_folder}/{lib_name}.{env["platform"]}.{env["target"]}.simulator.a'
        else:
            return f'{output_bin_folder}/{lib_name}.{env["platform"]}.{env["target"]}.a'
    else:
        return f'{output_bin_folder}/{lib_name}{env["suffix"]}{env["SHLIBSUFFIX"]}'


bin_file = get_bin_file(env)

library = env.SharedLibrary(bin_file, source=sources)

extension_file = "demo/addons/com.daylily_zeleen.hfsm2/hfsm2.gdextension"


def on_complete(target, source, env):
    # 更新版本号
    f = open(extension_file, "r", encoding="utf8")
    lines = f.readlines()
    f.close()

    for i in range(len(lines)):
        if lines[i].startswith('version = "') and lines[i].endswith('"\n'):
            lines[i] = f'version = "{build_version.version}"\n'
            break

    f = open(extension_file, "w", encoding="utf8")
    f.writelines(lines)
    f.close()


complete_command = Command("complete", library, on_complete)
Depends(complete_command, library)
Default(complete_command)
