"""Create minimal COFF short-import libraries from the Windows API declarations.

No proprietary SDK or MinGW libraries are needed; exported symbol names are
stdcall-decorated in COFF, then undecorated for Windows DLL lookup.
"""
from pathlib import Path
import struct
import subprocess

HERE = Path(__file__).resolve().parent
IMPORTS = {
    'KERNEL32.dll': [('VirtualQuery',12),('QueryPerformanceCounter',4),('QueryPerformanceFrequency',4),('Sleep',4),('GetProcessHeap',0),('HeapAlloc',12),('HeapFree',12),('GetPrivateProfileStringA',24),('GetPrivateProfileIntA',16),('GetModuleHandleA',4),('VirtualProtect',16),('GetCurrentThreadId',0),('LoadLibraryA', 4), ('GetProcAddress', 8),
       ('GetModuleHandleExA', 12), ('GetEnvironmentVariableA', 12), ('CreateFileA', 28),
       ('SetFilePointer', 16), ('WriteFile', 20), ('CloseHandle', 4),('GetCurrentProcessId', 0),('GetTickCount',0)],
    'USER32.dll': [('ShowWindow',8),('SetForegroundWindow',4),('ScreenToClient',8),('ClientToScreen',8),('GetSystemMetrics',4),('GetUpdateRect',12),('InvalidateRect',12),('RegisterClassA',4),('CreateWindowExA',48),('BeginPaint',8),('EndPaint',8),('AdjustWindowRectEx',16),('DefWindowProcA',16),('RegisterWindowMessageA',4),('MonitorFromWindow',8),('GetMonitorInfoA',8),('PostMessageA',16),('WindowFromPoint',8),('GetParent',4),('GetAsyncKeyState',4),('DispatchMessageA',4),('PeekMessageA',20),('GetMessageA',16),('ChildWindowFromPointEx',16),('GetCapture',0),('GetCursorPos',4),('SetCursorPos',8),('GetMessagePos',0),('UnhookWindowsHookEx',4),('SetWindowsHookExA',16),('CallNextHookEx',16),('SendMessageA',16),('CallWindowProcA',20),('GetWindowLongA',8),('SetWindowLongA',12),('GetWindow',8),('IsWindowVisible',4),('IsWindow',4),('GetDesktopWindow', 0),('GetWindowThreadProcessId',8),('GetClassNameA',12),('GetWindowTextA',12),('GetWindowRect',8),('GetClientRect',8),('EnumWindows',8),('GetDCEx',12),('MapWindowPoints',16),('SetWindowPos',28),('GetFocus',0),('SetFocus',4),('GetForegroundWindow',0),('IsChild',8),('ReleaseDC',8),('SetTimer',16),('KillTimer',8)],
    'GDI32.dll': [('GdiFlush',0),('StretchDIBits',52)],
}

def create_import(name, bytes_stack, dllname, output):
    symbol = f'_{name}@{bytes_stack}'
    payload = symbol.encode('ascii')+b'\0'+dllname.encode('ascii')+b'\0'
    # IMPORT_OBJECT_HEADER: TypeInfo=NAME_UNDECORATE << 2 = 3 << 2 = 12.
    header = struct.pack('<HHHHIIHH', 0, 0xFFFF, 0, 0x14c,
                         0, len(payload), 0, 12)
    output.write_bytes(header+payload)

for dllname, funcs in IMPORTS.items():
    stem = dllname.split('.')[0].lower()
    members = []
    for name, bytes_stack in funcs:
        fn = HERE / f'{stem}_{name}.obj'
        create_import(name, bytes_stack, dllname, fn)
        members.append(fn)
    subprocess.run(['llvm-ar', 'rcs', str(HERE / f'{stem}.lib'),
                    *map(str, members)], check=True)
    for fn in members: fn.unlink()
