import os
import sys
import zipfile
import shutil
from os.path import join as path_join


def main():
    # add_copyright()
    # save_as_utf8()
    if os.path.exists("./dist"):
        shutil.rmtree("./dist")

    args = "scons"
    debug_and_relaese = True
    custom_api_file_defined = False
    platform = ""
    for arg in sys.argv:
        if arg == "-h" or arg == "--help":
            os.system("scons -h")
            print('\nIf you have not specify "target" argument, this tool will build both debug and relaese.')
            return

        if arg.startswith("python"):
            continue
        if arg == sys.argv[0]:
            continue
        if arg.count("scons") > 0:
            print('Should not use "scons" argument, skip it.')
            continue
        if arg.startswith("custom_api_file"):
            custom_api_file_defined = True
        if arg.startswith("platform") or arg.startswith("p="):
            platform = arg.split("=", 1)[1]

        args += " " + arg

        if arg.startswith("target"):
            debug_and_relaese = False

    # Use 4.2 dev mono apis as default.
    if not custom_api_file_defined:
        args += " custom_api_file=gdextension_dependencies/extension_api.4.2.dev.mono.json"

    bin_dir = "./bin"
    # Remove all last build files.
    if os.path.exists(bin_dir):
        for f in os.listdir(bin_dir):
            os.remove(path_join(bin_dir, f))

    # Buiild.
    if debug_and_relaese:
        command = args + " target=template_debug"
        print("Building debug version: ", command)
        os.system(command)
        print("")

        command = args + " target=template_release"
        print("Building release version: ", command)
        os.system(command)
    else:
        print("Building...")
        os.system(args)

    print("Build finished, post processiong...")

    # Post process
    plugin_dir = "demo/addons/com.daylily_zeleen.hfsm2"
    dynamic_lib_suffixs = [".so", ".dylib", ".wasm", ".dll"]

    # Copy dynamic library.
    dst_dir = path_join(plugin_dir, "bin")
    if platform != "macos":
        for f in os.listdir(bin_dir):
            for suffix in dynamic_lib_suffixs:
                if not f.endswith(suffix):
                    continue
                shutil.copyfile(path_join(bin_dir, f), path_join(dst_dir, f.replace(".dev.", ".")))

    # Copy readme and license.
    if os.path.exists("README.md"):
        shutil.copyfile("README.md", path_join(plugin_dir, "README.md"))
        shutil.copyfile("README.md", path_join("demo", "README.md"))

    if os.path.exists("README_zh_cn.md"):
        shutil.copyfile("README_zh_cn.md", path_join(plugin_dir, "README_zh_cn.md"))
        shutil.copyfile("README_zh_cn.md", path_join("demo", "README_zh_cn.md"))

    if os.path.exists("LICENSE"):
        shutil.copyfile("LICENSE", path_join(plugin_dir, "LICENSE"))
        shutil.copyfile("LICENSE", path_join("demo", "LICENSE"))

    # Copy to dist
    dist_dir = "dist/addons"
    if os.path.exists(dist_dir):
        shutil.rmtree(dist_dir)
    shutil.copytree("demo/addons/", dist_dir)

    # Zip files.
    zip_file_path = "bin\com.daylily_zeleen.hfsm2.zip"
    if os.path.exists(zip_file_path):
        os.remove(zip_file_path)
    zip_file = zipfile.ZipFile(zip_file_path, "w")
    zip_files_recursively(zip_file, "demo/addons")
    zip_file.close()

    print("Done!")


def zip_files_recursively(zip_file: zipfile.ZipFile, dir: str):
    for f in os.listdir(dir):
        path = path_join(dir, f)
        if os.path.isdir(path):
            zip_files_recursively(zip_file, path)
        else:
            dst = path
            if dst.startswith("demo/"):
                dst = dst.replace("demo/", "", 1)
            zip_file.write(path, dst)


def save_as_utf8(dir: str = "."):
    for f in os.listdir(dir):
        path = path_join(dir, f)
        if f == "gdextension_dependencies":
            continue
        if os.path.isdir(path):
            save_as_utf8(path)
        else:
            if not f.endswith(".h") and not f.endswith(".cpp"):
                continue
            rfile = open(path, "rb")
            data = rfile.read().decode("utf-8-sig")
            rfile.close()

            # os.remove(path)

            text_utf8 = data.encode("utf-8")
            wfile = open(path, "wb")
            wfile.write(text_utf8)
            wfile.close()


def add_copyright_to_file(file: str):
    from misc.scripts.copyright_headers import generate_header_text as gen_header_text

    header_text = gen_header_text(file)

    rf = open(file, "r", encoding="utf-8")
    text = rf.read()
    rf.close()

    if text.startswith(header_text):
        return

    wf = open(file, "w", encoding="utf-8")
    wf.write(header_text + text)
    wf.close()


def add_copyright(dir: str = "."):
    for f in os.listdir(dir):
        path = path_join(dir, f)
        if f == "gdextension_dependencies":
            continue
        if os.path.isdir(path):
            add_copyright(path)
        else:
            if not f.endswith(".h") and not f.endswith(".cpp"):
                continue

            add_copyright_to_file(path)


def camel_to_snake(name: str):
    import re

    name = re.sub("(.)([A-Z][a-z]+)", r"\1_\2", name)
    name = re.sub("([a-z0-9])([A-Z])", r"\1_\2", name)
    return name.replace("2_D", "2D").replace("3_D", "3D").lower()


def generate_singleton_helper(extension_api_file: str):
    import json

    api = {}
    with open(extension_api_file, encoding="utf-8") as api_file:
        api = json.load(api_file)
    singletons = api["singletons"]

    valid_singletons = []
    # Step1: Collect valid singletons.
    for singleton in singletons:
        singleton_type = singleton["type"]
        if singleton["name"] in ["GDExtensionManager", "ResourceUID", "IP"]:
            continue
        if os.path.exists(
            path_join(
                "gdextension_dependencies/godot-cpp/gen/include/godot_cpp/classes",
                camel_to_snake(singleton_type) + ".hpp",
            )
        ):
            valid_singletons.append(singleton)

    lines = []
    lines.append("// This file is generated, any changes of this file may be lost.\n")
    lines.append("\n")

    for singleton in valid_singletons:
        lines.append(f"#include <godot_cpp/classes/{camel_to_snake(singleton['type']) + '.hpp'}>\n")
    lines.append("#include <godot_cpp/variant/utility_functions.hpp>\n")
    lines.append("\n")

    lines.append("using namespace godot;\n")
    lines.append("\n")

    lines.append("PackedStringArray get_singleton_name_list(){\n")
    lines.append("\tPackedStringArray ret;\n")
    for singleton in valid_singletons:
        lines.append(f'\tif (Engine::get_singleton()->has_singleton("{singleton["name"]}")) ' + "{\n")
        lines.append(f'\t\tret.push_back("{singleton["name"]}");\n')
        lines.append("\t}\n")
    lines.append("\treturn ret;\n")
    lines.append("}\n")
    lines.append("\n")

    lines.append("Array get_singleton_list(){\n")
    lines.append("\tArray ret;\n")
    for singleton in valid_singletons:
        lines.append(f'\tif (Engine::get_singleton()->has_singleton("{singleton["name"]}")) ' + "{\n")
        lines.append(f'\t\tret.push_back({singleton["type"]}::get_singleton());\n')
        lines.append("\t}\n")
    lines.append("\treturn ret;\n")
    lines.append("}\n")
    lines.append("\n")

    gen_file = "hfsm_global.gen.h"
    f = open(gen_file, "w")
    f.writelines(lines)
    f.close()

    add_copyright_to_file(gen_file)


if __name__ == "__main__":
    main()
