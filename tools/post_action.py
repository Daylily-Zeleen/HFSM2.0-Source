import os

def finish(target, source, env):
    if os.path.exists("bin/" + env["bin_file_name_base"]):
        os.remove("bin/" + env["bin_file_name_base"])

    os.rename("build/" + env["bin_file_name_base"],
              "bin/" + env["bin_file_name_base"])
    print("== cpy")

def get_executor():
    return finish