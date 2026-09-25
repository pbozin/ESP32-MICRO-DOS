import os
import subprocess
import sys
from SCons.Script import Import

Import("env")

def extract_text_section(source, target, env):
    elf_file = str(target[0])
    bin_file = os.path.splitext(elf_file)[0] + "_mdos.bin"
    
    print(f"\n[MDB Loader] Adjusting segment partitions for: {bin_file}")
    
    objcopy = "/home/bozin/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-objcopy"
    
    # 🛠️ Extract elements matching our updated layout order:
    # Header -> Instructions Only (.text) -> Data Block (.literal, .rodata, .data, .got)
    # 🛠️ Match the updated section footprint:
    cmd = [
        objcopy, "-O", "binary",
        "--only-section=.mdb_header",
        "--only-section=.text",
        "--only-section=.rodata",
        "--only-section=.data",
        "--only-section=.got",
        elf_file, bin_file
    ]
    
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode != 0:
        sys.stderr.write(f"Error packing binary:\n{result.stderr.decode('utf-8')}\n")
        env.Exit(1)
        
    print(f"[MDB Loader] SUCCESS. Clean binary package size: {os.path.getsize(bin_file)} bytes.\n")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", extract_text_section)
