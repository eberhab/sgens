#!/usr/bin/env bash
# Star Trek Generations: VGA, HKCU, movie decoding and orderly shutdown fixes.
# Bash + GNU coreutils only. Originals are never overwritten.
set -euo pipefail
fail() { printf 'sgens patch: %s\n' "$*" >&2; exit 1; }
identify=false
if [[ ${1:-} == --identify ]]; then identify=true; shift; fi
if [[ ${1:-} == --help || $# == 0 ]]; then
    echo 'Usage: bash patch-sgens.sh ORIGINAL.exe NEW.exe'
    echo '       bash patch-sgens.sh --identify FILE.exe'
    exit 0
fi
if $identify; then [[ $# == 1 ]] || fail 'Expected one file.'
else [[ $# == 2 ]] || fail 'Expected input and output files.'; fi
hash=$(sha256sum < "$1"); hash=${hash%% *}
state=original
case $hash in
    3e4329143247177c4415c06137bd5751e6b9de506405dfab4c15e3245a0e536a) lang=de ;;
    59497b6a94b02bb690a5fe0fe0552c41a4306a7bfcd53d8b18992c636ef5e5ff) lang=de; state=patched ;;
    e102fcb25ade3255cb0262578431c942c0f59de8cb8b8c16a8a8ceddb4618504) lang=en ;;
    c6e9a6b3adcb61ccf62ea32d7f4ecdf1a6ea4e131fe78126cb19b30b7320919a) lang=en; state=patched ;;
    *) fail "Unknown executable SHA-256: $hash" ;;
esac
if $identify; then printf '%s %s\n' "$lang" "$state"; exit 0; fi
[[ ! -e $2 && ! -L $2 ]] || fail 'Output already exists.'
tmp=$(mktemp -- "$2.tmp.XXXXXXXX")
trap 'rm -f -- "$tmp"' EXIT
cp -- "$1" "$tmp"
write() { printf '%b' "$2" | dd of="$tmp" bs=1 seek="$(($1))" conv=notrunc status=none; }
if [[ $state == original ]]; then
    if [[ $lang == de ]]; then
        probe=0xCDBB0; registry=0x87ADF; child=0x4ACAE; hidden=0x50C66
    else
        probe=0xDE940; registry=0x867CF; child=0x5F19E; hidden=0x8B0FB
    fi
    # Privileged VGA port probe: return false (XOR EAX,EAX; RET).
    write "$probe" '\x31\xc0\xc3'
    # Original HKLM PUSH 80000002 -> HKCU PUSH 80000001 for Path/CDPath.
    write "$registry" '\x01'
    # Internal WS_POPUP -> WS_CHILD|WS_CLIPSIBLINGS; keep WS_CLIPCHILDREN.
    write "$child" '\x46'
    # Hide the initial desktop-sized window until the shim sizes it.
    write "$hidden" '\x83'
    # Add one 512-byte RX section. No new imports or absolute stub addresses.
    # Movie adapter supplies compressed-frame size and DWORD-aligned RGB size.
    # Exit adapter stops/joins decoder workers, deletes app+110h and app+E8h
    # before graphics teardown; WM_CLOSE posts WM_QUIT without destroying HWND.
    # Annotated implementation: native patcher source/patch-xp.c.
    if [[ $lang == de ]]; then
        dd if=/dev/zero of="$tmp" bs=1 seek=1471488 count=512 conv=notrunc status=none
        # Seven PE sections.
        write 0x86 '\x07\x00'
        # SizeOfCode includes the new 512-byte section.
        write 0x9C '\x00\x84\x11\x00'
        # SizeOfImage includes its 4096-byte virtual page.
        write 0xD0 '\x00\x00\x1d\x00'
        # Read/execute .sgfix section header; no writable code.
        write 0x268 '\x2e\x73\x67\x66\x69\x78\x00\x00\xc7\x00\x00\x00\x00\xf0\x1c\x00\x00\x02\x00\x00\x00\x74\x16\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\x00\x60'
        # Movie header repair; stop/join decoder workers; ordered object cleanup; WM_CLOSE.
        write 0x167400 \
            '\x8b\x44\x24\x0c\x8b\x4d\xfc\x8b\x89\x80\x01\x00\x00\x8b\x49\x04\x89\x48\x14\x8b\x44\x24\x14\x0f\xb7\x48\x0e\x0f\xaf\x48\x04\x83\xc1\x1f\xc1\xe9\x05\xc1\xe1\x02'\
'\x8b\x50\x08\x85\xd2\x79\x02\xf7\xda\x0f\xaf\xca\x89\x48\x14\xe9\x12\xf8\xef\xff\x51\xe8\xb6\xad\xee\xff\x59\x51\x8b\x89\x10\x01\x00\x00\x85\xc9\x74\x14\x8b\x04'\
'\x24\xc7\x80\x10\x01\x00\x00\x00\x00\x00\x00\x8b\x01\x6a\x01\xff\x50\x04\x8b\x0c\x24\x8b\x89\xe8\x00\x00\x00\x85\xc9\x74\x14\x8b\x04\x24\xc7\x80\xe8\x00\x00\x00'\
'\x00\x00\x00\x00\x8b\x01\x6a\x01\xff\x50\x0c\x59\x55\x89\xe5\x83\xec\x1c\x53\x56\x57\xe9\xee\x13\xe8\xff\x83\x7c\x24\x04\x10\x75\x20\xe8\x00\x00\x00\x00\x58\x6a'\
'\x00\xff\x90\x1e\x5e\xfe\xff\x8b\x54\x24\x10\xc7\x02\x00\x00\x00\x00\xb8\x01\x00\x00\x00\xc2\x10\x00\x55\x89\xe5\x83\xec\x0c\x53\x56\x57\xe9\x50\x29\xe8\xff'
        # ICDecompress: CPU output buffer path.
        write 0xB9FF8 '\xe8\x03\x44\x11\x00'
        # ICDecompress: DirectDraw output buffer path.
        write 0xBA044 '\xe8\xb7\x43\x11\x00'
        # ExitInstance: join movies and destroy scenes before graphics.
        write 0x4F877 '\xe9\xc0\xeb\x17\x00'
        # Main OnWndMsg: WM_CLOSE posts WM_QUIT, preserving the HWND during cleanup.
        write 0x50E0E '\xe9\x7f\xd6\x17\x00'
    else
        dd if=/dev/zero of="$tmp" bs=1 seek=1571328 count=512 conv=notrunc status=none
        # Seven PE sections.
        write 0x86 '\x07\x00'
        # SizeOfCode includes the new 512-byte section.
        write 0x9C '\x00\xda\x12\x00'
        # SizeOfImage includes its 4096-byte virtual page.
        write 0xD0 '\x00\x90\x1e\x00'
        # Read/execute .sgfix section header; no writable code.
        write 0x268 '\x2e\x73\x67\x66\x69\x78\x00\x00\xc7\x00\x00\x00\x00\x80\x1e\x00\x00\x02\x00\x00\x00\xfa\x17\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\x00\x60'
        # Movie header repair; stop/join decoder workers; ordered object cleanup; WM_CLOSE.
        write 0x17FA00 \
            '\x8b\x44\x24\x0c\x8b\x4d\xfc\x8b\x89\x70\x02\x00\x00\x8b\x49\x04\x89\x48\x14\x8b\x44\x24\x14\x0f\xb7\x48\x0e\x0f\xaf\x48\x04\x83\xc1\x1f\xc1\xe9\x05\xc1\xe1\x02'\
'\x8b\x50\x08\x85\xd2\x79\x02\xf7\xda\x0f\xaf\xca\x89\x48\x14\xe9\xa2\x75\xef\xff\x51\xe8\xa2\x15\xee\xff\x59\x51\x8b\x89\x10\x01\x00\x00\x85\xc9\x74\x14\x8b\x04'\
'\x24\xc7\x80\x10\x01\x00\x00\x00\x00\x00\x00\x8b\x01\x6a\x01\xff\x50\x04\x8b\x0c\x24\x8b\x89\xe8\x00\x00\x00\x85\xc9\x74\x14\x8b\x04\x24\xc7\x80\xe8\x00\x00\x00'\
'\x00\x00\x00\x00\x8b\x01\x6a\x01\xff\x50\x0c\x59\x55\x89\xe5\x83\xec\x1c\x53\x56\x57\xe9\x7e\x28\xea\xff\x83\x7c\x24\x04\x10\x75\x20\xe8\x00\x00\x00\x00\x58\x6a'\
'\x00\xff\x90\x3a\x4e\xfe\xff\x8b\x54\x24\x10\xc7\x02\x00\x00\x00\x00\xb8\x01\x00\x00\x00\xc2\x10\x00\x55\x89\xe5\x83\xec\x0c\x53\x56\x57\xe9\xe5\x3d\xea\xff'
        # ICDecompress: CPU output buffer path.
        write 0xC97C3 '\xe8\x38\xdc\x11\x00'
        # ICDecompress: DirectDraw output buffer path.
        write 0xC980F '\xe8\xec\xdb\x11\x00'
        # ExitInstance: join movies and destroy scenes before graphics.
        write 0x89D07 '\xe9\x30\xd7\x15\x00'
        # Main OnWndMsg: WM_CLOSE posts WM_QUIT, preserving the HWND during cleanup.
        write 0x8B2A3 '\xe9\xea\xc1\x15\x00'
    fi
fi
if [[ $lang == de ]]; then expected=59497b6a94b02bb690a5fe0fe0552c41a4306a7bfcd53d8b18992c636ef5e5ff
else expected=c6e9a6b3adcb61ccf62ea32d7f4ecdf1a6ea4e131fe78126cb19b30b7320919a; fi
actual=$(sha256sum < "$tmp"); actual=${actual%% *}
[[ $actual == "$expected" ]] || fail 'Patched checksum mismatch.'
chmod 644 -- "$tmp"
# Atomic publication without replacing an existing file or symbolic link.
ln -T -- "$tmp" "$2" || fail 'Cannot create output; destination must support hard links.'
printf '%s patched: %s\nSHA-256: %s\n' "$lang" "$2" "$actual"
