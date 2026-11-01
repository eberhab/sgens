# Native Windows fix

This four-byte patch bypasses the privileged VGA probe and makes the game read
installation paths from **HKCU** instead of the original **HKLM**. It uses
Windows' system DirectDraw, without a graphics wrapper. The patcher runs on XP
and Windows 11, checks the original and output hashes, and refuses to overwrite
files. No administrator rights are needed; use a writable game folder.

| File | Purpose |
| --- | --- |
| `patch-xp.exe` | Windows patcher; native and full-shim modes. |
| `patch-vga.sh` | Bash alternative for the native patch. |
| `sgens.reg` | Editable installation paths for the playing user. |
| `source/patch-xp.c`, `source/build.py` | Annotated implementation and reproducible build. |

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
Focused native checks cover English on XP SP3 and German on Windows 11; complete
campaigns and every graphics driver are not qualified.

## Bash and source

With Bash and GNU coreutils, from the repository root:

```bash
bash xp/patch-vga.sh /path/to/sgens.exe /path/to/sgens-xp.exe
```

The destination must not exist and its filesystem must support hard links.
The native patch is not the complete Wine fix. Its VGA replacement is
`31 C0 C3` (`XOR EAX,EAX; RET`) at DE `0xCDBB0` / EN `0xDE940`; the HKCU byte
is `02 -> 01` at DE `0x87ADF` / EN `0x867CF`. These are file offsets for the two
originals identified in the patchers, not generic offsets for other editions.

Rebuild with Python 3, Clang/LLD and `llvm-ar`; no Windows SDK is needed:

```bash
build_dir=$(mktemp -d)
python3 xp/source/build.py "$build_dir/patch-xp.exe"
cmp "$build_dir/patch-xp.exe" xp/patch-xp.exe
```

Clang/LLD 18.1.3 reproduces the supplied binary. `CLANG`, `LLD_LINK` and
`LLVM_AR` select tool paths. Python is needed only to build, not to patch on XP.
For the full six-byte patch, the EXE also accepts `--shim INPUT OUTPUT`;
follow the Wine folder's setup for that output. See the root [license](../LICENSE).
