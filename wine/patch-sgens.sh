#!/usr/bin/env bash
# Star Trek Generations: version-checked, six-byte graphics and per-user registry patch.
# Bash + coreutils only. Original input is never modified. Unknown builds fail.
set -euo pipefail
fail() { printf 'patch-sgens: %s\n' "$*" >&2; exit 1; }
if [[ ${1:-} == --help || $# == 0 ]]; then
    printf '%s\n' 'Usage: patch-sgens.sh INPUT.exe OUTPUT.exe' \
        '       patch-sgens.sh --identify INPUT.exe' \
        'Recognizes the German/English originals and their fully patched copies.'
    exit 0
fi
identify=false
if [[ ${1:-} == --identify ]]; then identify=true; shift; fi
if $identify; then [[ $# == 1 ]] || fail 'Expected one input file.'
else [[ $# == 2 ]] || fail 'Expected input and output filenames.'; fi
[[ -f $1 ]] || fail "Input not found: $1"
hash=$(sha256sum -- "$1"); hash=${hash%% *}
state=original
case $hash in
    3e4329143247177c4415c06137bd5751e6b9de506405dfab4c15e3245a0e536a) lang=de ;;
    e102fcb25ade3255cb0262578431c942c0f59de8cb8b8c16a8a8ceddb4618504) lang=en ;;
    06dbbd6e2efd20d9ba0f4b41367e948c5d21224b3fc40e3edd8835ee36259849) lang=de; state=patched ;;
    982838a50c97f403916e7ce43fb29ec3741620ce38dfa8b2e39d5aeba5d9e40e) lang=en; state=patched ;;
    *) fail "Unknown executable SHA-256: $hash" ;;
esac
if $identify; then printf '%s %s\n' "$lang" "$state"; exit 0; fi
[[ ! -e $2 && ! -L $2 ]] || fail "Output already exists: $2"
if [[ $lang == de ]]; then
    probe=$((0xCDBB0)); child=$((0x4ACAE)); hidden=$((0x50C66)); registry_root=$((0x87ADF))
    expected=06dbbd6e2efd20d9ba0f4b41367e948c5d21224b3fc40e3edd8835ee36259849
else
    probe=$((0xDE940)); child=$((0x5F19E)); hidden=$((0x8B0FB)); registry_root=$((0x867CF))
    expected=982838a50c97f403916e7ce43fb29ec3741620ce38dfa8b2e39d5aeba5d9e40e
fi
tmp=$(mktemp -- "$(dirname -- "$2")/.sgens-patch.XXXXXXXX")
trap 'rm -f -- "$tmp"' EXIT
cp -- "$1" "$tmp"
if [[ $state == original ]]; then
    # Direct VGA I/O (ports 03B4/03B5) faults under Wine/NT user mode.
    # Return false immediately: MOV EDX,03B4... -> XOR EAX,EAX; RET.
    printf '\x31\xc0\xc3' | dd of="$tmp" bs=1 seek="$probe" conv=notrunc status=none
    # Internal full-screen popup screens cover/conflict with the main window.
    # WS_POPUP -> WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS; retain other flags.
    # The companion DLL restores presentation, input scaling, z-order and focus.
    printf '\x46' | dd of="$tmp" bs=1 seek="$child" conv=notrunc status=none
    # The main window is initially desktop-sized. Remove WS_VISIBLE at creation
    # so that its black startup rectangle is not displayed before final sizing.
    printf '\x83' | dd of="$tmp" bs=1 seek="$hidden" conv=notrunc status=none
fi
# Original installation lookup used HKLM (PUSH 80000002), requiring machine-wide
# registration. Select HKCU (PUSH 80000001) for the playing user's Path/CDPath.
# This changes only the root handle, not the key name or resource lookup.
printf '\x01' | dd of="$tmp" bs=1 seek="$registry_root" conv=notrunc status=none

actual=$(sha256sum -- "$tmp"); actual=${actual%% *}
[[ $actual == "$expected" ]] || fail 'Patched checksum mismatch.'
chmod 644 -- "$tmp"
# Atomic no-clobber publication in the destination filesystem.
ln -- "$tmp" "$2" || fail 'Cannot create output (already exists or unsupported filesystem).'
printf '%s patched: %s\nSHA-256: %s\n' "$lang" "$2" "$actual"
