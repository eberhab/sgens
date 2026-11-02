# Native Windows fix

This patch bypasses the privileged VGA probe and makes the game read
installation paths from **HKCU** instead of the original **HKLM**. It uses
Windows' system DirectDraw, without a graphics wrapper. The patcher runs on XP
and Windows 11, checks the original and output hashes, and refuses to overwrite
files. No administrator rights are needed; use a writable game folder.

| File | Purpose |
| --- | --- |
| `patch-xp.exe` | Windows patcher; native and full-shim modes. |
| `patch-xp.sh` | Bash alternative for the native patch. |
| `sgens.reg` | Editable installation paths for the playing user. |
| `source/patch-xp.c`, `source/pool-fix.s`, `source/build.py` | Annotated implementation and reproducible build. |

Both patched variants also correct the input/output image sizes supplied to the
Windows movie codec and repair shutdown order. Movie workers stop before their
surfaces are freed; active scene objects are destroyed while their graphics
services still exist. Closing the main window requests orderly application exit
instead of destroying that window before cleanup. These repairs live in an EXE
`.sgfix` code section (199 code bytes, 512 bytes added to each file); the original
EXE, game data and save format are preserved. No codec installation is required.

Both modes also repair the resource string pool: the original copies a path
before checking available space, so long installation/DATA paths can overwrite
heap metadata and crash later on menu Quit. The replacement checks the full
string length including its NUL before copying, allocates a sufficiently large
block when needed, and retains the original ownership/freeing rules. This is a
103-byte in-place EXE repair, separate from `.sgfix`; the shim DLL is unchanged.
It does not remove other original path-length limits or add Unicode-path support.

The missing Windows movies are a codec-header compatibility issue: the game
passes a static compressed-size value and a zero RGB565 output size. XP/Windows
11's native Cinepak path rejects decoding until those sizes are corrected;
Wine 9.0's builtin decoder tolerates them. The exact reason the original Win95
codec worked remains unverified. Both native and shim EXEs contain the repair;
no replacement codec or additional Wine setting is needed.

Shutdown is a game-side ordering repair validated with native and shim setups.
Its placement does not prove that the shim was uninvolved in triggering the
original close failure: native XP menu Quit passed before the repair, while
shim title-bar close failed. The patch shares explicit worker/scene cleanup
with native play; a demonstrated shim defect still belongs in the shim.

## Install and play

1. Close the game. Keep the original `sgens.exe`, matching-language `DATA/`,
   preferences and writable `SAVEGAME/` and `WORLD/` directories.
2. **Move game-local `ddraw*.dll` and `dxwrapper.dll` out of the game folder.**
   Native mode must use system DirectDraw. Leave Windows system DLLs alone;
   the patcher does not move DLLs or check the launch environment.
3. From a Command Prompt in this repository's root, create the native EXE:

   ```bat
   xp\patch-xp.exe --native "C:\Games\Generations\sgens.exe" "C:\Games\Generations\sgens-xp.exe"
   ```

4. Edit both paths in [sgens.reg](sgens.reg), then import and launch as the
   same Windows user:

   ```bat
   reg.exe import xp\sgens.reg
   cd /d "C:\Games\Generations"
   sgens-xp.exe
   ```

`Path` is the game directory; `CDPath` is its resource directory (normally
`DATA`, or the actual directory on a mounted disc, such as `D:\GENS`). Complete
local resources worked without a mounted CD in the checked scenes; media checks
are unchanged. REG files contain literal paths, not relative paths or variables.
Edit and reimport after moving the game or switching DE/EN: both editions and
graphics modes share `HKCU\SOFTWARE\MicroProse\Star Trek Generations\1.0`.
Launching another EXE alone does not update these paths. Originals still use HKLM.

Native graphics request exclusive 640×480 fullscreen; scaling depends on the
driver or VM. For resizing and borderless Alt+Enter, use the [shim](../wine/README.md).
Focused checks cover movies, cartography and exit on XP SP3 and Windows 11;
complete campaigns and every graphics driver are not qualified.

## Bash and source

With Bash and GNU coreutils, from the repository root:

```bash
bash xp/patch-xp.sh /path/to/sgens.exe /path/to/sgens-xp.exe
```

The destination must not exist and its filesystem must support hard links.
The native patch is not the complete Wine fix. Its VGA replacement is
`31 C0 C3` (`XOR EAX,EAX; RET`) at DE `0xCDBB0` / EN `0xDE940`; the HKCU byte
is `02 -> 01` at DE `0x87ADF` / EN `0x867CF`. These are file offsets for the two
originals identified in the patchers, not generic offsets for other editions.
The annotated C source records the additional decoder, main-window close and
ExitInstance adapters, the string-pool repair and the edition-specific offsets. Both Bash and C tools
verify the complete patched hash, including the appended code section.
Bash also accepts `--identify INPUT` and can copy its recognized native output;
the Windows tool requires an untouched original for either mode.

Rebuild with Python 3, Clang/LLD and `llvm-ar`; no Windows SDK is needed:

```bash
build_dir=$(mktemp -d)
python3 xp/source/build.py "$build_dir/patch-xp.exe"
cmp "$build_dir/patch-xp.exe" xp/patch-xp.exe
```

Clang/LLD 18.1.3 reproduces the supplied binary. `CLANG`, `LLD_LINK` and
`LLVM_AR` select tool paths. Python is needed only to build, not to patch on XP.
For the full shim patch, the EXE also accepts `--shim INPUT OUTPUT`;
follow the Wine folder's setup for that output. See the root [license](../LICENSE).
