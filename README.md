# Star Trek Generations: XP and Wine fixes

Compatibility fixes for the German and English editions of **Star Trek
Generations (1997)**. Supply your own installed game and matching-language data;
no game files are included.

The original game probes VGA hardware directly, which faults on Windows XP.
Under Wine, its mixed DirectDraw/GDI drawing and overlapping popup windows also
cause broken graphics, fullscreen and mouse input. Both fixes change installation
lookup from the original machine-wide registry key (HKLM) to the current user's
key (HKCU), allowing setup without administrator rights.

| Folder | Solution |
| --- | --- |
| [xp/](xp/README.md) | Minimal four-byte EXE patch; uses Windows' system DirectDraw. Also tested on Windows 11. |
| [wine/](wine/README.md) | Six-byte EXE patch, Generations shim and cnc-ddraw; supports resized windows, borderless fullscreen and optional shader filtering on Wine and Windows. |

Each folder contains its instructions, patcher, registry sample and source.
Patch a supported original `sgens.exe`; the patchers verify its SHA-256 and
create a separate executable. Keep German and English data and saves separate.
Only one installation can be registered per Windows user or Wine prefix at a
time; edit and import the appropriate REG file when switching.

[LICENSE](LICENSE) contains the MIT terms and copyright notices for the project
and bundled **cnc-ddraw 7.1**. Its unmodified x86 DLL is renamed
`ddraw-backend.dll`. Include this license when redistributing either the project
or that backend. The license does not cover the game itself.
