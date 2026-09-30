import os
import subprocess
import sys
import struct
from SCons.Script import Import

Import("env")

def check_microdos_binary(file_path):
    if not os.path.exists(file_path):
        sys.exit(1)

    with open(file_path, "rb") as f:
        data = f.read()

    dram_size, iram_size, entry_off, got_off = struct.unpack("<IIII", data[:16])

    misalignments = 0

    iram_start = 16
    iram_end = iram_start + iram_size
    dram_start = iram_end
    dram_end = dram_start + dram_size

    for offset in range(16, entry_off, 4):
        word = struct.unpack("<I", data[offset:offset+4])[0]

        if word == 0 or word == 0xFFFFFFFF:
            continue

        if (word >= iram_start and word < iram_end) or (word >= dram_start and word <= dram_end):
            if word % 4 == 0:
                continue

            pool_index = (offset - 16) // 4
            print(f"  ❌ [TRUE ALIGNMENT FAULT] Pool [{pool_index}] @ offset 0x{offset:04X}: "
                  f"Unaligned address reference = 0x{word:08X}!")
            misalignments += 1

    if misalignments > 0:
        print("-" * 76)
        print(f"❌ COMPONENT ERROR: Found {misalignments} true unaligned variable data allocation faults.")
        print("-" * 76)
        return False
    else:
        print(f"[MDB Loader] SUCCESS. Clean binary package size: {os.path.getsize(file_path)} bytes.\n")
        return True

def extract_text_section(source, target, env):
    elf_file = str(target[0])
    bin_file = os.path.splitext(elf_file)[0] + "_mdos.bin"

    print(f"\n[MDB Loader] Adjusting segment partitions for: {bin_file}")

    objcopy = "/home/bozin/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-objcopy"

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
    check_microdos_binary(bin_file)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", extract_text_section)
