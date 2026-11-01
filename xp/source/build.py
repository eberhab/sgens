#!/usr/bin/env python3
"""Build the XP x86 patcher with Clang/LLD; no SDK or target C runtime.

Usage: python3 source/build.py /path/to/new/patch-xp.exe
CLANG, LLD_LINK and LLVM_AR may name specific compiler tool executables.
Temporary objects/import libraries are removed automatically.
Python is a developer build dependency, never an XP runtime dependency.
SPDX-License-Identifier: MIT
"""
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

if len(sys.argv) != 2:
    sys.exit(__doc__)
output = Path(sys.argv[1]).resolve()
if output.exists():
    sys.exit("Refusing to replace an existing build output")
source = Path(__file__).resolve().with_name("patch-xp.c")
imports = {
    "kernel32.dll": [("CreateFileW",28),("GetFileSize",8),("GetLastError",0),("ReadFile",20),
        ("WriteFile",20),("SetFilePointer",16),("FlushFileBuffers",4),
        ("CloseHandle",4),("DeleteFileW",4),("GetStdHandle",4),
        ("GetCommandLineW",0),("LocalFree",4),("ExitProcess",4)],
    "shell32.dll": [("CommandLineToArgvW",8)],
}
with tempfile.TemporaryDirectory(prefix="sgens-xp-build-") as temporary:
    root = Path(temporary)
    libraries = []
    for dll, functions in imports.items():
        members = []
        for name, stack_bytes in functions:
            member = root / (name + ".obj")
            payload = f"_{name}@{stack_bytes}\0{dll}\0".encode("ascii")
            # COFF short import; IMPORT_NAME_UNDECORATE resolves stdcall APIs.
            member.write_bytes(struct.pack("<HHHHIIHH",0,0xffff,0,0x14c,0,len(payload),0,12) + payload)
            members.append(str(member))
        library = root / (dll + ".lib")
        subprocess.run([os.environ.get("LLVM_AR", "llvm-ar"), "rcs", str(library), *members], check=True)
        libraries.append(str(library))
    obj = root / "patch.obj"
    subprocess.run([os.environ.get("CLANG", "clang"), "-target", "i686-pc-windows-msvc",
        "-c", "-Os", "-ffreestanding", "-fno-builtin", "-fno-stack-protector",
        "-mno-sse", "-mno-sse2", "-Wall", "-Wextra", "-Werror",
        str(source), "-o", str(obj)], check=True)
    subprocess.run([os.environ.get("LLD_LINK", "lld-link"), "/timestamp:0", "/machine:x86",
        "/entry:entry@0", "/subsystem:console,5.01", "/osversion:5.1", "/nodefaultlib",
        # Suppress Windows installer detection for the name patch-xp.exe.
        # XP ignores requestedExecutionLevel; newer Windows runs without elevation.
        "/manifest:embed", "/manifestuac:level='asInvoker' uiAccess='false'",
        "/dynamicbase", "/nxcompat", "/out:" + str(output), str(obj), *libraries], check=True)
print(output)
