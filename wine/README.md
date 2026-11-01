# Wine and enhanced Windows graphics

The six-byte game patch, **Generations shim v12** and **cnc-ddraw 7.1 x86**
repair presentation, scaled mouse input and Alt+Enter fullscreen. All runtime
components are included; no download, DXVK or Winetricks is required.

```text
sgens-wine.exe -> ddraw.dll (Generations shim) -> ddraw-backend.dll (cnc-ddraw)
```

| File | Purpose |
| --- | --- |
| `patch-sgens.sh` | Checksum-verified DE/EN patcher; creates `sgens-wine.exe`. |
| `wine.reg` | Game paths, prefix-wide Wine DLL override and monitor safeguards. |
| `ddraw.dll`, `ddraw-backend.dll` | Game-specific shim and unmodified, renamed cnc-ddraw DLL. |
| `ddraw.ini`, `sgens.ini` | Shim/backend profile and the game's GDI cursor setting. |
| `download-cnc-ddraw.sh` | Optional, verified download of the bundled backend version. |
| `source/` | Shim C/headers, export definition and build scripts. |

## Linux setup

Use Wine with 32-bit support, Bash and GNU coreutils. The example uses a writable
game folder ending in `en`, with the original `sgens.exe`, complete English
`DATA/`, `SAVEGAME/` and `WORLD/`. Keep German files and saves in a separate `de`
folder. Both editions can share one dedicated Wine prefix.

Close the game and back up existing graphics DLLs/INIs. From the repository root:

```bash
repo=$(pwd -P)
game_dir=$(cd -- '/path/to/Generations/en' && pwd -P)
bash wine/patch-sgens.sh "$game_dir/sgens.exe" "$game_dir/sgens-wine.exe"
cp -- wine/ddraw.dll wine/ddraw-backend.dll wine/ddraw.ini "$game_dir/"
if [[ ! -e "$game_dir/sgens.ini" ]]; then cp -- wine/sgens.ini "$game_dir/"; fi
```

If `sgens.ini` exists, set `UseGDI=1` in `[CURSOR]`. Keep the EXE name
`sgens-wine.exe`: it selects the supplied backend profile. The patcher refuses
unknown inputs and existing outputs; its output filesystem needs hard links.

Create a new prefix once, then map G: to the parent of the language folders:

```bash
export WINEPREFIX="$HOME/.local/share/wineprefixes/sgens" WINEARCH=win32
mkdir -p -- "$(dirname -- "$WINEPREFIX")"
wine wineboot.exe --init
wineserver -w
ln -sT -- "$(dirname -- "$game_dir")" "$WINEPREFIX/dosdevices/g:"
```

An existing prefix cannot be converted with `WINEARCH`. The mapping command
refuses an existing G:; check it points to the intended parent before continuing.
Use a short subdirectory such as `G:\en`, not the drive root. Edit the literal
`Path` and `CDPath` values in [wine.reg](wine.reg) if needed; its defaults are
`G:\en` and `G:\en\DATA`. Import and launch:

```bash
wine reg.exe import "$(winepath -w "$repo/wine/wine.reg")"
wineserver -w
(cd -- "$game_dir" && wine ./sgens-wine.exe)
```

For later runs, set the same `WINEPREFIX` and launch from the game folder.
Reimport the edited REG file after moving the game or switching DE/EN. Both
editions share HKCU `Path`/`CDPath`; launching another EXE alone does not change
them. Run one game at a time. The REG file also sets prefix-wide
`ddraw=native,builtin` and `UseXRandR=N`, `UseXVidMode=N` to prevent Wine X11
monitor-mode changes; no per-EXE override is needed.

## Windows XP / Windows 11

Copy `ddraw.dll`, `ddraw-backend.dll`, `ddraw.ini` and the cursor setting described
above into the game folder. From the repository root, create the full-shim EXE:

```bat
xp\patch-xp.exe --shim "C:\Games\Generations\sgens.exe" "C:\Games\Generations\sgens-wine.exe"
```

Edit `Path` and `CDPath` in `wine\wine.reg` to your Windows directories, then:

```bat
reg.exe import wine\wine.reg
cd /d "C:\Games\Generations"
sgens-wine.exe
```

Run as the playing user; HKCU needs no administrator rights. Windows does not
use the REG file's Wine settings. `CDPath` points to the resource directory;
complete local DATA works without a mounted CD in the checked scenes, but media
checks are unchanged. For [native Windows graphics](../xp/README.md), use the
four-byte patch and move the local wrapper DLLs out of the game folder.

## Settings and fixes

In `[sgens-shim]` in the installed `ddraw.ini`, `width`/`height` set the window
size (for example 1280×960); `fullscreen=1` starts borderless fullscreen.
Alt+Enter toggles it. The 640×480 image is stretched, so use 4:3 window dimensions.
Keep `block_display_changes=1` for the shim's monitor guard; Wine's registry
safeguards are independent. The backend profile uses `renderer=gdi`, `hook=0`,
`fixchilds=0`, nonexclusive windowed mode and disabled backend fullscreen hotkeys
so the shim owns the visible window and input. Backend shaders cannot filter the
shim's final image; use the optional shim filter below. `SGENS.CFG` stores binary game preferences; edit them in Settings
and keep that file and saves separate for DE/EN.

The EXE patch bypasses the VGA probe (three bytes), selects HKCU instead of HKLM
(one byte), and fixes two window styles: **82 -> 46** makes internal screens
clipped children required for drawing, focus and input; **93 -> 83** removes
initial `WS_VISIBLE` until the shim sizes and shows the main window. The latter
controls startup visibility; later gameplay does not depend on it. Exact offsets,
hashes and explanations are in `patch-sgens.sh`. Original EXEs still read HKLM.

The shim uses public DirectDraw/Win32 interfaces. `ddraw_shim.c` handles GDI
presentation/input, `presentation_filter.h` owns optional final-image filtering,
`com_proxy.h` forwards COM calls, `logical_display.h` handles
timing, and `import_guard.h` adapts resolved display API imports. Alternative
backends need their own configuration and compatibility checks. Focused checks
cover Wine 9.0, XP SP3 and Windows 11; complete campaigns and every driver are
not qualified. The game folder's `sgens-ddraw-shim.log` provides diagnostics;
`SGENS_DDRAW_CAPTURE=1` enables frame captures.

## Optional shader filtering

The shim can filter its completed image, including GDI menus and the lower
panel, independently of the DirectDraw backend. In `[sgens-shim]` in the game's
`ddraw.ini`, choose:

```ini
width=1280
height=960
filter=bicubic
```

Restart the game after editing. `none` (default) keeps the existing GDI
presentation; `bilinear` softly blends pixels; `bicubic` uses a sharper
Catmull-Rom interpolation shader. Smoothing can soften small text, so choose by
preference. The game still renders at 640×480: this changes scaling, not scene
detail. Use a larger window or Alt+Enter to see the effect.

Keep both existing DLLs and the backend's `renderer=gdi` profile in place.
Filtering uses the shim's own OpenGL 2.0/GLSL 1.20 context; it needs no shader
files, downloads, DXVK, new game patch or cnc-ddraw development build. It reads
this option from the INI beside the shim, even while the game changes working
directory. Select `filter=none` and restart to turn it off.

`source/presentation_filter.h` uploads the already composed, top-down BGRA frame
and applies a built-in shader only to the existing presentation child. It
restores the thread's previous OpenGL context after each frame and frees its
resources when that child is destroyed. Shader/driver failures log a fallback
to GDI; if a pixel format was assigned, the shim recreates the child first,
because Windows does not allow changing a window's pixel format. No backend
COM calls, hooks or display-mode settings are changed by filtering.

The optional filter is runtime-checked under Wine 9.0 with software OpenGL;
XP/Windows 11 shader operation is not yet verified. Check `sgens-ddraw-shim.log`
in the game folder or `WORLD/` for `Presentation filter active` or a fallback
message. These are startup settings, not live shader-switching controls.

## Build and backend download

Build in a scratch copy using Python 3, Clang/LLD and `llvm-ar` (no Windows SDK):

```bash
build_dir=$(mktemp -d)
cp -a wine/source "$build_dir/source"
bash "$build_dir/source/build.sh"
cmp "$build_dir/ddraw.dll" wine/ddraw.dll
```

Clang/LLD 18.1.3 reproduces the supplied DLL. `CLANG` and `LLD_LINK` select tool
paths. An in-place build overwrites `wine/ddraw.dll`.

`bash wine/download-cnc-ddraw.sh` verifies the bundled DLL or fetches the same
[cnc-ddraw 7.1 release](https://github.com/FunkyFr3sh/cnc-ddraw/releases/tag/v7.1.0.0)
if absent. An optional directory argument selects another destination. It checks
ZIP/DLL hashes and refuses a different existing file. Downloads need `curl`,
`unzip` and HTTPS; the shim and INIs are untouched.

The root [LICENSE](../LICENSE) includes cnc-ddraw's original copyright and MIT
notice. Retain it when redistributing the backend, including a downloaded copy.
