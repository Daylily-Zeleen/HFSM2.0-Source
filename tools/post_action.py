import os
import sys


def move_dynamic_library(build_dir, target_dir, suffixs=[".so", ".dylib", ".wasm", ".dll"]):
    for file in os.listdir(build_dir):
        for suffix in suffixs:
            if file.endswith(suffix):
                target_file = os.path.join(target_dir, file)
                if os.path.exists(target_file):
                    os.remove(target_file)
                os.rename(os.path.join(build_dir, file), target_file)
                print("== move: ", file)
                break


move_dynamic_library(sys.argv[1], sys.argv[2])
