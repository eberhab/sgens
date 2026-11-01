#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
: "${CLANG:=clang}"
: "${LLD_LINK:=lld-link}"
python3 make_import_libs.py
"$CLANG" -target i686-pc-windows-msvc -c -O2 -ffreestanding -fno-builtin -fno-stack-protector \
  -Werror -Wall -Wextra ddraw_shim.c -o ddraw_shim.obj
"$LLD_LINK" /dll /timestamp:0 /machine:x86 /entry:DllMain@12 /subsystem:windows,5.01 \
  /def:ddraw.def /out:../ddraw.dll ddraw_shim.obj kernel32.lib user32.lib gdi32.lib /nodefaultlib
rm -f kernel32.lib user32.lib gdi32.lib ddraw_shim.obj
rm -f ../ddraw.lib
