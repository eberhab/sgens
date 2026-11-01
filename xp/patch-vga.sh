set -euo pipefail

[[ $# == 2 ]] || {
    echo 'Usage: bash patch-vga.sh ORIGINAL.exe NEW.exe' >&2
    exit 2
}

t=$(mktemp -- "$2.tmp.XXXXXX")
trap 'rm -f -- "$t"' EXIT
cp -- "$1" "$t"

h=$(sha256sum < "$t")
case ${h%% *} in
    3e4329143247177c4415c06137bd5751e6b9de506405dfab4c15e3245a0e536a)
        o=$((0xCDBB0))
        r=$((0x87ADF))
        ;;
    e102fcb25ade3255cb0262578431c942c0f59de8cb8b8c16a8a8ceddb4618504)
        o=$((0xDE940))
        r=$((0x867CF))
        ;;
    *)
        echo 'Unsupported or already patched EXE' >&2
        exit 1
        ;;
esac

# Bypass the original privileged VGA port probe on NT/Wine.
printf '\x31\xc0\xc3' | dd of="$t" bs=1 seek="$o" conv=notrunc status=none
# Original HKLM lookup: PUSH 80000002 -> PUSH 80000001 (HKCU).
# Path/CDPath now belong to the playing user; no administrator setup is needed.
printf '\x01' | dd of="$t" bs=1 seek="$r" conv=notrunc status=none
ln -T -- "$t" "$2"
