#!/usr/bin/env bash
# Fetch the tested x86 backend, without replacing the Generations shim or INI.
set -euo pipefail
fail() { printf 'download-cnc-ddraw: %s\n' "$*" >&2; exit 1; }
if [[ ${1:-} == --help ]]; then
    printf '%s\n' 'Usage: download-cnc-ddraw.sh [OUTPUT_DIRECTORY]' \
        'Default: the folder containing this script. The directory must exist.' \
        'Downloads cnc-ddraw 7.1 as ddraw-backend.dll; refuses different existing files.'
    exit 0
fi
[[ $# -le 1 ]] || fail 'Expected at most one output directory.'
command -v sha256sum >/dev/null || fail 'Install GNU coreutils (sha256sum).'
target_dir=$(cd -- "${1:-$(dirname -- "${BASH_SOURCE[0]}")}" && pwd -P) \
    || fail 'Output directory does not exist or is inaccessible.'
output="$target_dir/ddraw-backend.dll"
url='https://github.com/FunkyFr3sh/cnc-ddraw/releases/download/v7.1.0.0/cnc-ddraw.zip'
zip_sha=0b13ab89a64c9918189b1dadd449ef6ed3cb3b7b19cabd96d8adbd95505bb908
dll_sha=85e0f7d530dfda134793a57cb3e76b0287dcc96892ee57162dd68f47283b03a9
matches() {
    local actual
    actual=$(sha256sum < "$1") || return 1
    [[ ${actual%% *} == "$2" ]]
}

# An already verified backend needs neither network access nor download tools.
if [[ -e $output || -L $output ]]; then
    if [[ -f $output ]] && matches "$output" "$dll_sha"; then
        printf 'Verified cnc-ddraw 7.1: %s\n' "$output"
        exit 0
    fi
    fail "Existing output differs; move it aside first: $output"
fi
for tool in curl unzip; do
    command -v "$tool" >/dev/null || fail "Required command not found: $tool"
done
tmp=$(mktemp -d -- "$target_dir/.cnc-ddraw.XXXXXXXX")
trap 'rm -rf -- "$tmp"' EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

printf 'Downloading cnc-ddraw 7.1...\n'
curl --fail --location --silent --show-error --proto '=https' --proto-redir '=https' \
    --connect-timeout 15 --max-time 120 --retry 2 --output "$tmp/cnc-ddraw.zip" "$url" \
    || fail 'Download failed.'
matches "$tmp/cnc-ddraw.zip" "$zip_sha" || fail 'Downloaded archive checksum mismatch.'
# Extract only the backend; upstream ddraw.ini must not replace our game profile.
unzip -p "$tmp/cnc-ddraw.zip" ddraw.dll > "$tmp/ddraw-backend.dll" \
    || fail 'Cannot extract ddraw.dll.'
matches "$tmp/ddraw-backend.dll" "$dll_sha" || fail 'Extracted DLL checksum mismatch.'
chmod 644 -- "$tmp/ddraw-backend.dll"
# Publish the complete verified file without overwriting a concurrent output.
ln -T -- "$tmp/ddraw-backend.dll" "$output" \
    || fail 'Cannot create output (already exists or filesystem lacks hard links).'
printf 'Installed cnc-ddraw 7.1: %s\nSHA-256: %s\n' "$output" "$dll_sha"
