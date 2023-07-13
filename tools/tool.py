import os

for root, dirs, files in os.walk("./"):
    for file in files:
        if file.endswith(".h"):
            src_file = os.path.join(root, file)
            dst_file = src_file.removesuffix("pp")
            os.rename(src_file, dst_file)

print("finish")