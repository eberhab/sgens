/* Game-local primary-surface presenter for Star Trek Generations.
 * Copyright (c) 2026. MIT license (including the supplied handover shim).
 * Requires the verified child-window EXE patch and a renamed DirectDraw backend.
 * Supports 8-bit palette, RGB16, RGB32. Never patches Wine or CD checks. */
#if !defined(_M_IX86) && !defined(__i386__)
#error This is a 32-bit x86 DLL
#endif

typedef unsigned long DWORD;
typedef unsigned long ULONG;
typedef long HRESULT;
typedef void *HANDLE;
typedef void *HWND;
typedef void *HMODULE;
typedef int BOOL;
typedef unsigned char BYTE;
typedef const char *LPCSTR;

#define WINAPI __stdcall
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)

/* Minimal Windows API imports. No Windows SDK or C runtime is required. */
DLLIMPORT HMODULE WINAPI LoadLibraryA(LPCSTR);
DLLIMPORT BOOL WINAPI GetModuleHandleExA(DWORD,LPCSTR,HMODULE*);
DLLIMPORT unsigned WINAPI GetPrivateProfileIntA(LPCSTR,LPCSTR,int,LPCSTR);
DLLIMPORT DWORD WINAPI GetPrivateProfileStringA(LPCSTR,LPCSTR,LPCSTR,char*,DWORD,LPCSTR);
DLLIMPORT BOOL WINAPI PostMessageA(HWND,unsigned,DWORD,long);
DLLIMPORT void *WINAPI GetProcAddress(HMODULE, LPCSTR);
DLLIMPORT DWORD WINAPI GetEnvironmentVariableA(LPCSTR, char *, DWORD);
DLLIMPORT HANDLE WINAPI CreateFileA(LPCSTR, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
DLLIMPORT DWORD WINAPI SetFilePointer(HANDLE, long, long *, DWORD);
DLLIMPORT BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);
DLLIMPORT BOOL WINAPI CloseHandle(HANDLE);
DLLIMPORT HWND WINAPI GetDesktopWindow(void);
DLLIMPORT DWORD WINAPI GetCurrentProcessId(void);
DLLIMPORT long WINAPI GetWindowLongA(HWND,int);
DLLIMPORT long WINAPI SetWindowLongA(HWND,int,long);
DLLIMPORT long WINAPI SendMessageA(HWND,unsigned,DWORD,long);
DLLIMPORT long WINAPI CallWindowProcA(void*,HWND,unsigned,DWORD,long);
DLLIMPORT DWORD WINAPI GetWindowThreadProcessId(HWND, DWORD *);
DLLIMPORT int WINAPI GetClassNameA(HWND, char *, int);
DLLIMPORT int WINAPI GetWindowTextA(HWND, char *, int);
DLLIMPORT BOOL WINAPI GetWindowRect(HWND, void *);
DLLIMPORT BOOL WINAPI GetClientRect(HWND, void *);
DLLIMPORT BOOL WINAPI EnumWindows(void *, long);
DLLIMPORT HWND WINAPI GetWindow(HWND,unsigned);
DLLIMPORT BOOL WINAPI IsWindowVisible(HWND);
DLLIMPORT BOOL WINAPI IsWindow(HWND);
DLLIMPORT DWORD WINAPI GetTickCount(void);
DLLIMPORT void *WINAPI GetDCEx(HWND,void*,DWORD);
DLLIMPORT int WINAPI MapWindowPoints(HWND,HWND,void*,unsigned);
DLLIMPORT BOOL WINAPI SetWindowPos(HWND,HWND,int,int,int,int,unsigned);
DLLIMPORT HWND WINAPI GetFocus(void);
DLLIMPORT HWND WINAPI SetFocus(HWND);
DLLIMPORT HWND WINAPI GetForegroundWindow(void);
DLLIMPORT BOOL WINAPI IsChild(HWND,HWND);
DLLIMPORT int WINAPI ReleaseDC(HWND, void *);
DLLIMPORT int WINAPI StretchDIBits(void *, int, int, int, int, int, int, int, int, const void *, const void *, unsigned int, DWORD);
DLLIMPORT DWORD WINAPI SetTimer(HWND, DWORD, DWORD, void *);
DLLIMPORT BOOL WINAPI KillTimer(HWND, DWORD);

typedef struct _GUID {
    unsigned long Data1;
    unsigned short Data2, Data3;
    BYTE Data4[8];
} GUID;

#define S_OK ((HRESULT)0)
#define E_FAIL ((HRESULT)0x80004005ul)
#define MAX_HOOKS 128
#define MAX_VTABLE 30
#define GENERIC_WRITE 0x40000000ul
#define FILE_SHARE_READ 1ul
#define FILE_SHARE_WRITE 2ul
#define OPEN_ALWAYS 4ul
#define CREATE_ALWAYS 2ul
#define FILE_ATTRIBUTE_NORMAL 0x80ul
#define FILE_END 2ul
#define INVALID_HANDLE_VALUE ((HANDLE)(-1))
#define DLL_PROCESS_ATTACH 1ul

typedef HRESULT (WINAPI *PFN_DRAWCREATE)(const GUID *, void **, void *);
typedef HRESULT (WINAPI *PFN_DRAWCREATEEX)(const GUID *, void **, const GUID *, void *);
typedef HRESULT (WINAPI *PFN_CREATECLIPPER)(DWORD, void **, void *);
typedef HRESULT (WINAPI *PFN_ENUM)(void *, void *);
typedef HRESULT (WINAPI *PFN_ENUMEX)(void *, void *, DWORD);
typedef HRESULT (WINAPI *PFN_QUERY)(void *, const GUID *, void **);
typedef ULONG (WINAPI *PFN_RELEASE)(void *);
typedef HRESULT (WINAPI *PFN_COOP)(void *, HWND, DWORD);
typedef HRESULT (WINAPI *PFN_MODE1)(void *, DWORD, DWORD, DWORD);
typedef HRESULT (WINAPI *PFN_MODE2)(void *, DWORD, DWORD, DWORD, DWORD, DWORD);
typedef HRESULT (WINAPI *PFN_PALETTE)(void *, DWORD, void *, void **, void *);
typedef struct { DWORD dwSize,dwFlags,dwHeight,dwWidth; long lPitch; DWORD back,mip,alpha,reserved; void *pixels; DWORD keys[8]; DWORD pixfmt[8]; DWORD caps; } SDESC;
typedef struct { long left,top,right,bottom; } RECT;
typedef struct { BYTE red,green,blue,flags; } PALENTRY;
typedef HRESULT (WINAPI *PFN_CREATESURF)(void *,SDESC *,void **,void *);
typedef HRESULT (WINAPI *PFN_SURF_PALETTE)(void *,void *);
typedef HRESULT (WINAPI *PFN_SURF_FLIP)(void *,void *,DWORD);
typedef HRESULT (WINAPI *PFN_SURF_BLT)(void *,RECT *,void *,RECT *,DWORD,void *);
typedef HRESULT (WINAPI *PFN_SURF_BLTFAST)(void *,DWORD,DWORD,void *,RECT *,DWORD);
typedef HRESULT (WINAPI *PFN_SURF_ATTACHED)(void *,void *,void **);
typedef HRESULT (WINAPI *PFN_SURF_LOCK)(void *,RECT *,SDESC *,DWORD,void *);
typedef HRESULT (WINAPI *PFN_SURF_UNLOCK)(void *,void *);
typedef HRESULT (WINAPI *PFN_SURF_GETDC)(void *,void **);
typedef HRESULT (WINAPI *PFN_SURF_RELEASEDC)(void *,void *);
typedef HRESULT (WINAPI *PFN_PALETTE_GETENTRIES)(void *, DWORD, DWORD, DWORD, PALENTRY *);
_Static_assert(sizeof(SDESC) == 108, "32-bit DDSURFACEDESC must be exactly 108 bytes");
#define MAX_SURFS 256
#define MAX_CAPTURES 64
#define SURF_VTBL 36
#define DDSCAPS_PRIMARYSURFACE 0x00000200ul
#define MAX_WIDTH 1024
#define MAX_HEIGHT 768
#define DIB_RGB_COLORS 0
#define SRCCOPY 0x00CC0020ul
#define BI_RGB 0

typedef struct {
    void *iface;
    void *original[MAX_VTABLE];
    void *proxy[MAX_VTABLE];
    unsigned int nmethods;
    unsigned int version;
} HOOK;

static HOOK g_hooks[MAX_HOOKS];
static HMODULE g_backend;
static unsigned int g_backend_failed;
static HWND g_game_hwnd;
static DWORD g_timer;
typedef struct { DWORD size; long w,h; unsigned short planes,bpp; DWORD compression,imagebytes; long xppm,yppm; DWORD used,important; BYTE colors[1024]; } BMPINFO;
typedef struct { void *iface; void *original[SURF_VTBL]; void *proxy[SURF_VTBL]; DWORD caps; DWORD width,height; void *palette; unsigned int called_flip,called_blt,called_unlock,called_releasedc,slot,locks,dcs,generation; void *draw; PFN_CREATESURF make_surface; } SURFHOOK;
static SURFHOOK g_surfs[MAX_SURFS];
static unsigned g_surface_generation;


static int guid_eq(const GUID *a, const GUID *b)
{
    unsigned int i;
    if (!a || !b || a->Data1 != b->Data1 || a->Data2 != b->Data2 || a->Data3 != b->Data3)
        return 0;
    for (i = 0; i != 8; ++i) if (a->Data4[i] != b->Data4[i]) return 0;
    return 1;
}

static const GUID iid_draw1 = {0x6c14db80ul, 0xa733, 0x11ce, {0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60}};
static const GUID iid_draw2 = {0xb3a6f3e0ul, 0x2b43, 0x11cf, {0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56}};
static const GUID iid_draw3 = {0x618f8ad4ul, 0x8b7a, 0x11d0, {0x8f,0xcc,0,0xc0,0x4f,0xd9,0x18,0x9d}};
static const GUID iid_draw4 = {0x9c59509aul, 0x39bd, 0x11d1, {0x8c,0x4a,0,0xc0,0x4f,0xd9,0x30,0xc5}};
static const GUID iid_draw7 = {0x15e65ec0ul, 0x3b9c, 0x11d2, {0xb9,0x2f,0,0x60,0x97,0x97,0xea,0x5b}};

static unsigned int draw_version(const GUID *iid)
{
    if (guid_eq(iid, &iid_draw1)) return 1;
    if (guid_eq(iid, &iid_draw2)) return 2;
    if (guid_eq(iid, &iid_draw3)) return 3;
    if (guid_eq(iid, &iid_draw4)) return 4;
    if (guid_eq(iid, &iid_draw7)) return 7;
    return 0;
}

static unsigned int vtbl_count(unsigned int version)
{
    switch (version) {
        case 1: return 23;
        case 2: return 24;
        case 3: return 25;
        case 4: return 28;
        case 7: return 30;
        default: return 0;
    }
}

static void concat(char *buf, unsigned *idx, const char *s)
{
    while (*s && *idx < 430) buf[(*idx)++] = *s++;
    buf[*idx] = 0;
}
static void hex32(char *buf, unsigned *idx, unsigned long v)
{
    unsigned shift;
    const char *digits = "0123456789ABCDEF";
    concat(buf, idx, "0x");
    for (shift = 28; ; shift -= 4) {
        if (*idx < 430) buf[(*idx)++] = digits[(v >> shift) & 15];
        if (!shift) break;
    }
    buf[*idx] = 0;
}
static void append_log(const char *line)
{
    HANDLE file;
    unsigned len = 0;char complete[502];
    DWORD written;
    /* Relative to the game's working directory, not to the prefix root. */
    file = CreateFileA("sgens-ddraw-shim.log", 4, /* FILE_APPEND_DATA */
                       FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE) return;
    while (line[len] && len < 500) {complete[len]=line[len];++len;}
    complete[len++]='\r';complete[len++]='\n';
    WriteFile(file, complete, len, &written, 0);
    CloseHandle(file);
}

/* Present the CURRENT primary surface, never an arbitrary uploaded asset.
 * A per-window timer covers palette-only changes and expose events. All reads
 * use the saved backend vtable, avoiding recursion through our own hooks. */
/* Borderless is a window operation, never a DirectDraw mode change. Keep the
 * backend's render target/mode intact while our presenter scales the primary.
 * In particular, never invoke a backend switch from a WH_KEYBOARD callback:
 * Generations peeks with PM_NOREMOVE and can deliver that callback repeatedly.
 * These are documented Win32 APIs; no cnc-ddraw messages/addresses are used. */
DLLIMPORT BOOL WINAPI AdjustWindowRectEx(void*,DWORD,BOOL,DWORD);
DLLIMPORT long WINAPI DefWindowProcA(HWND,unsigned,DWORD,long);
DLLIMPORT unsigned WINAPI RegisterWindowMessageA(LPCSTR);
DLLIMPORT HANDLE WINAPI MonitorFromWindow(HWND,DWORD);
DLLIMPORT BOOL WINAPI GetMonitorInfoA(HANDLE,void*);
typedef struct { DWORD size; RECT monitor,work; DWORD flags; } MONITORINFO;
static void *g_window_proc;
static HWND g_proc_hwnd;
static unsigned g_fullscreen_message;
static int g_borderless,g_changing_window;
static unsigned g_geometry_checks;
static RECT g_saved_window;
static long g_saved_style,g_saved_exstyle;

/* Set the client size while resizing is enabled, then remove the frame.
 * Otherwise Wine/X11 can keep fixed-size hints from the old 640/960 window
 * and the window manager shrinks the new borderless window back afterwards. */
static BOOL place_borderless(const RECT *monitor)
{
    long style=GetWindowLongA(g_game_hwnd,-16),exstyle=GetWindowLongA(g_game_hwnd,-20);
    RECT framed=*monitor;
    style=(style|0x00040000ul)&~0x21000000ul; /* allow resize; clear min/max state */
    SetWindowLongA(g_game_hwnd,-16,style);
    AdjustWindowRectEx(&framed,style,0,exstyle);
    if(!SetWindowPos(g_game_hwnd,0,framed.left,framed.top,
                     framed.right-framed.left,framed.bottom-framed.top,0x30))return 0;
    SetWindowLongA(g_game_hwnd,-16,style&~0x00cf0000ul);
    SetWindowLongA(g_game_hwnd,-20,exstyle&~0x00000300ul);
    return SetWindowPos(g_game_hwnd,0,monitor->left,monitor->top,
                        monitor->right-monitor->left,monitor->bottom-monitor->top,0x30);
}
static void set_borderless(int enable)
{
    if(!g_game_hwnd || g_changing_window || enable==g_borderless)return;
    MONITORINFO mi={0};mi.size=sizeof(mi);
    if(enable && !GetMonitorInfoA(MonitorFromWindow(g_game_hwnd,2),&mi))return;
    g_changing_window=1;
    if(enable) {
        GetWindowRect(g_game_hwnd,&g_saved_window);
        g_saved_style=GetWindowLongA(g_game_hwnd,-16);
        g_saved_exstyle=GetWindowLongA(g_game_hwnd,-20);
        g_borderless=1;
        if(!place_borderless(&mi.monitor)) {
            SetWindowLongA(g_game_hwnd,-16,g_saved_style);
            SetWindowLongA(g_game_hwnd,-20,g_saved_exstyle);
            SetWindowPos(g_game_hwnd,0,g_saved_window.left,g_saved_window.top,
                         g_saved_window.right-g_saved_window.left,g_saved_window.bottom-g_saved_window.top,0x30);
            g_borderless=0;g_changing_window=0;
            append_log("ERROR: borderless placement failed; window restored");return;
        }
        append_log("Borderless entered: Win32 window geometry only");
    } else {
        SetWindowLongA(g_game_hwnd,-16,g_saved_style);
        SetWindowLongA(g_game_hwnd,-20,g_saved_exstyle);
        SetWindowPos(g_game_hwnd,0,g_saved_window.left,g_saved_window.top,
                     g_saved_window.right-g_saved_window.left,g_saved_window.bottom-g_saved_window.top,0x30);
        g_borderless=0;
        append_log("Borderless left: previous window restored");
    }
    RECT actual={0};GetClientRect(g_game_hwnd,&actual);
    char line[440];unsigned o=0;concat(line,&o,"Presentation client width/height=");hex32(line,&o,actual.right);hex32(line,&o,actual.bottom);append_log(line);
    g_geometry_checks=enable?8:0;
    g_changing_window=0;
}
static void restore_borderless_geometry(void)
{
    if(!g_borderless || g_changing_window)return;
    MONITORINFO mi={0};mi.size=sizeof(mi);
    if(!GetMonitorInfoA(MonitorFromWindow(g_game_hwnd,2),&mi))return;
    g_changing_window=1;
    if(!place_borderless(&mi.monitor))append_log("ERROR: restoring borderless geometry failed");
    g_geometry_checks=8;
    g_changing_window=0;
}
/* Wine's X11 window manager applies size hints asynchronously. A transient
 * work-area constraint (e.g. the desktop panel) can arrive after SetWindowPos
 * reported the full monitor size. Recheck a bounded number of settled frames;
 * this is only window geometry, never a monitor mode request. */
static void check_borderless_geometry(void)
{
    if(!g_borderless || !g_geometry_checks || g_changing_window)return;
    unsigned remaining=--g_geometry_checks;
    RECT actual={0};MONITORINFO mi={0};mi.size=sizeof(mi);
    if(GetWindowRect(g_game_hwnd,&actual) &&
       GetMonitorInfoA(MonitorFromWindow(g_game_hwnd,2),&mi) &&
       (actual.left!=mi.monitor.left || actual.top!=mi.monitor.top ||
        actual.right!=mi.monitor.right || actual.bottom!=mi.monitor.bottom)) {
        restore_borderless_geometry();g_geometry_checks=remaining;
        append_log("Corrected deferred borderless window geometry");
    }
}
static long WINAPI game_window_proc(HWND hwnd,unsigned msg,DWORD wp,long lp)
{
    /* The adapter owns the native frame. The game's MFC non-client painter
     * queries logical GetWindowRect/GetSystemMetrics and draws stale frame
     * pixels on XP after a fullscreen transition. Keep non-client layout,
     * painting, activation and hit-testing in USER32's real coordinate space. */
    if(msg==0x83 || msg==0x84 || msg==0x85 || msg==0x86)
        return DefWindowProcA(hwnd,msg,wp,lp);

    if(msg==g_fullscreen_message) {
        set_borderless(wp==2?1:!g_borderless);
        return 0;
    }
    /* Our scaling does not resize the game's logical 640x480 canvas or ask
     * the backend to tear down/restart its render thread. Normal windowed
     * resizes still go through the original window-procedure chain. */
    if(g_borderless || g_changing_window) {
        if(msg==5)return 0; /* WM_SIZE: logical canvas is unchanged */
        if(msg==0x24 || msg==0x46 || msg==0x47)
            return DefWindowProcA(hwnd,msg,wp,lp); /* Native sizing, no game/backend layout */
    }
    return CallWindowProcA(g_window_proc,hwnd,msg,wp,lp);
}
static void install_window_proc(HWND hwnd)
{
    if(!hwnd)return; /* NULL cooperative level is the normal shutdown path. */
    if(!g_fullscreen_message)
        g_fullscreen_message=RegisterWindowMessageA("SGENS_SHIM_BORDERLESS_V1");
    if(g_proc_hwnd && g_proc_hwnd!=hwnd && IsWindow(g_proc_hwnd) &&
       (void*)GetWindowLongA(g_proc_hwnd,-4)==(void*)game_window_proc)
        SetWindowLongA(g_proc_hwnd,-4,(long)g_window_proc);
    if(g_fullscreen_message && (void*)GetWindowLongA(hwnd,-4)!=(void*)game_window_proc) {
        g_window_proc=(void*)SetWindowLongA(hwnd,-4,(long)game_window_proc);
        if(g_window_proc)g_proc_hwnd=hwnd;
        else append_log("ERROR: main window subclass could not be installed");
    }
}

static DWORD g_frame_tick;
static int g_presenting;
/* Never read back while a forwarded backend call may pump messages. */
static unsigned g_backend_depth;
/* Renderer/decoder calls can overlap the game's thread. A plain shared ++/--
 * loses updates and can underflow, permanently disabling presentation. Atomic
 * operations preserve this process-wide exclusion count across those calls;
 * they do not claim to serialize all COM object bookkeeping. */
static void backend_enter(void){__atomic_fetch_add(&g_backend_depth,1,__ATOMIC_SEQ_CST);}
static void backend_leave(void){__atomic_fetch_sub(&g_backend_depth,1,__ATOMIC_SEQ_CST);}
static unsigned backend_busy(void){return __atomic_load_n(&g_backend_depth,__ATOMIC_SEQ_CST);}

#include "com_proxy.h"
static unsigned g_mode_changing;
static SURFHOOK *g_primary;
static BYTE g_pixels[MAX_WIDTH*MAX_HEIGHT*4];
static BMPINFO g_primary_bmi;
/* Present only from the game thread's timer dispatch. Surface calls can occur
 * midway through a game bitmap upload or UI transition; synchronous readback
 * used to call sync_screen_order/WM_PAINT from inside that unfinished operation.
 * Defer both readback and logical repaint until the game yields to its queue. */
static void capture_primary(void);
static void WINAPI primary_timer(HWND hwnd,DWORD msg,DWORD id,DWORD tick)
{
    (void)msg;(void)tick;
    /* Re-arm after painting, not on a repeating wall-clock deadline. A 4K
     * software blit can take longer than 33 ms. A continuously overdue timer
     * then keeps the message queue busy and starves the game's MFC idle loop:
     * the shim repaints the same frame forever, while simulation/input stop.
     * Leave a real idle interval after each repaint. The timer coalesces the
     * completed surface writes; it is the sole capture/readback entry point. */
    KillTimer(hwnd,id);
    check_borderless_geometry();
    if(g_primary)capture_primary();
    if(hwnd==g_game_hwnd && IsWindow(hwnd))
        g_timer=SetTimer(hwnd,0x5347,33,(void*)primary_timer);
}
/* The game keeps a 640x480 logical canvas at every presentation size.
 * EXE-only IAT adapters own logical rectangles, cursor queries and message
 * routing. Backend hooks must leave this DLL's USER32 calls usable for the
 * visible game HWND; the separate backend HWND isolates DXGL's mouse hooks.
 * This is a Generations adapter, not a general Win32 virtualization layer. */
DLLIMPORT HMODULE WINAPI GetModuleHandleA(LPCSTR);
DLLIMPORT BOOL WINAPI VirtualProtect(void*,DWORD,DWORD,DWORD*);
DLLIMPORT BOOL WINAPI GetCursorPos(void*);
DLLIMPORT BOOL WINAPI SetCursorPos(int,int);
DLLIMPORT DWORD WINAPI GetMessagePos(void);
DLLIMPORT BOOL WINAPI UnhookWindowsHookEx(HANDLE);
static void screen_to_game(long *pt)
{
    RECT r={0};GetClientRect(g_game_hwnd,&r);
    MapWindowPoints(0,g_game_hwnd,pt,1);
    if(r.right>0 && r.bottom>0){pt[0]=pt[0]*640/r.right;pt[1]=pt[1]*480/r.bottom;}
}
static BOOL WINAPI game_cursor(void *point)
{
    if(!GetCursorPos(point))return 0;
    if(g_game_hwnd)screen_to_game((long*)point);
    return 1;
}
static BOOL WINAPI game_set_cursor(int x,int y)
{
    if(!g_game_hwnd)return SetCursorPos(x,y);
    RECT r={0};long pt[2]={x,y};GetClientRect(g_game_hwnd,&r);
    pt[0]=pt[0]*r.right/640;pt[1]=pt[1]*r.bottom/480;
    MapWindowPoints(g_game_hwnd,0,pt,1);
    return SetCursorPos(pt[0],pt[1]);
}
static DWORD WINAPI game_message_pos(void)
{
    DWORD p=GetMessagePos();long pt[2]={(short)p,(short)(p>>16)};
    if(g_game_hwnd)screen_to_game(pt);
    return ((DWORD)(unsigned short)pt[1]<<16)|(unsigned short)pt[0];
}
/* EXE-only coordinate contract: virtual screen origin is the game's
 * 640x480 canvas. Child windows retain their original logical geometry.
 * Backend/shim IATs are not rewritten by these EXE-only adapters. A backend
 * that globally detours USER32 still needs a compatible window/input profile. */
DLLIMPORT BOOL WINAPI ScreenToClient(HWND,void*);
DLLIMPORT BOOL WINAPI ClientToScreen(HWND,void*);
DLLIMPORT int WINAPI GetSystemMetrics(int);
static int logical_window(HWND hwnd)
{
    return g_game_hwnd && (hwnd==g_game_hwnd || IsChild(g_game_hwnd,hwnd));
}
static BOOL WINAPI game_client_rect(HWND hwnd,RECT *rect)
{
    if(g_game_hwnd && rect && (hwnd==g_game_hwnd || hwnd==GetDesktopWindow())){
        *rect=(RECT){0,0,640,480};return 1;
    }
    return GetClientRect(hwnd,rect);
}
static BOOL WINAPI game_window_rect(HWND hwnd,RECT *rect)
{
    if(g_game_hwnd && rect && (hwnd==g_game_hwnd || hwnd==GetDesktopWindow())){
        *rect=(RECT){0,0,640,480};return 1;
    }
    BOOL ok=GetWindowRect(hwnd,rect);
    if(ok && logical_window(hwnd))MapWindowPoints(0,g_game_hwnd,rect,2);
    return ok;
}
static BOOL WINAPI game_to_client(HWND hwnd,void *point)
{
    if(logical_window(hwnd) && point){MapWindowPoints(g_game_hwnd,hwnd,point,1);return 1;}
    return ScreenToClient(hwnd,point);
}
static BOOL WINAPI game_to_screen(HWND hwnd,void *point)
{
    if(logical_window(hwnd) && point){MapWindowPoints(hwnd,g_game_hwnd,point,1);return 1;}
    return ClientToScreen(hwnd,point);
}
static int WINAPI game_map_points(HWND from,HWND to,void *points,unsigned count)
{
    if((!from || from==GetDesktopWindow()) && logical_window(to))from=g_game_hwnd;
    if((!to || to==GetDesktopWindow()) && logical_window(from))to=g_game_hwnd;
    return MapWindowPoints(from,to,points,count);
}
static int WINAPI game_metrics(int index)
{
    if(g_game_hwnd && index==0)return 640;
    if(g_game_hwnd && index==1)return 480;
    return GetSystemMetrics(index);
}
static int same_name(const char *a,const char *b)
{
    while(*a && *a==*b){++a;++b;}return *a==*b;
}
static BOOL WINAPI game_peek(void*,HWND,unsigned,unsigned,unsigned);
static BOOL WINAPI game_get(void*,HWND,unsigned,unsigned);
static BOOL WINAPI game_invalidate(HWND,const RECT*,BOOL);
static BOOL WINAPI game_update_rect(HWND,RECT*,BOOL);
static void *WINAPI game_begin_paint(HWND,void*);
static void hook_game_imports(void)
{
    BYTE *image=(BYTE*)GetModuleHandleA(0);
    if(!image || *(unsigned short*)image!=0x5a4d)return;
    BYTE *nt=image+*(DWORD*)(image+0x3c);
    if(*(DWORD*)nt!=0x4550 || *(unsigned short*)(nt+24)!=0x10b)return;
    DWORD import_rva=*(DWORD*)(nt+24+104);
    if(!import_rva)return;
    for(DWORD *desc=(DWORD*)(image+import_rva);desc[3];desc+=5) {
        if(!desc[0])continue;
        DWORD *names=(DWORD*)(image+desc[0]),*iat=(DWORD*)(image+desc[4]);
        for(unsigned i=0;names[i];i++) {
            if(names[i]&0x80000000)continue;
            const char *name=(const char*)(image+names[i]+2);void *target=0;
            if(same_name(name,"GetClientRect"))target=(void*)game_client_rect;
            if(same_name(name,"GetWindowRect"))target=(void*)game_window_rect;
            if(same_name(name,"ScreenToClient"))target=(void*)game_to_client;
            if(same_name(name,"ClientToScreen"))target=(void*)game_to_screen;
            if(same_name(name,"MapWindowPoints"))target=(void*)game_map_points;
            if(same_name(name,"GetSystemMetrics"))target=(void*)game_metrics;
            if(same_name(name,"GetCursorPos"))target=(void*)game_cursor;
            if(same_name(name,"SetCursorPos"))target=(void*)game_set_cursor;
            if(same_name(name,"GetMessagePos"))target=(void*)game_message_pos;
            if(same_name(name,"PeekMessageA"))target=(void*)game_peek;
            if(same_name(name,"GetMessageA"))target=(void*)game_get;
            if(same_name(name,"InvalidateRect"))target=(void*)game_invalidate;
            if(same_name(name,"GetUpdateRect"))target=(void*)game_update_rect;
            if(same_name(name,"BeginPaint"))target=(void*)game_begin_paint;
            if(target && iat[i]!=(DWORD)target) {
                DWORD old=0,ignored=0;
                if(VirtualProtect(iat+i,4,0x04,&old)) {
                    iat[i]=(DWORD)target;VirtualProtect(iat+i,4,old,&ignored);
                } else append_log("ERROR: game import protection failed");
            }
        }
    }
}
static HWND g_present_hwnd;
static HWND g_active_screen;

/* The presenter intentionally covers the original game windows. Windows can
 * discard their physical update regions, but Generations' WM_PAINT handlers
 * require GetUpdateRect/BeginPaint rectangles before rebuilding DirectDraw UI.
 * Retain logical invalidations and replay them into the game's surfaces. These
 * EXE-only adapters never change backend/system functions or expose hidden DCs.
 */
typedef struct { HWND hwnd; RECT rect; int dirty; } LOGICALPAINT;
static LOGICALPAINT g_logical_paints[128];
static HWND g_forced_paint;
static RECT g_forced_rect;
DLLIMPORT BOOL WINAPI GetUpdateRect(HWND,RECT*,BOOL);
DLLIMPORT BOOL WINAPI InvalidateRect(HWND,const RECT*,BOOL);
DLLIMPORT void *WINAPI BeginPaint(HWND,void*);
typedef struct { void *dc; BOOL erase; RECT paint; BOOL restore,update; BYTE reserved[32]; } PAINTSTRUCT;
static void remember_paint(HWND hwnd,const RECT *rect)
{
    if(!hwnd || hwnd==g_present_hwnd || !g_game_hwnd || !IsChild(g_game_hwnd,hwnd))return;
    RECT client,r;if(!GetClientRect(hwnd,&client))return;
    r=rect?*rect:client;
    if(r.left<client.left)r.left=client.left;
    if(r.top<client.top)r.top=client.top;
    if(r.right>client.right)r.right=client.right;
    if(r.bottom>client.bottom)r.bottom=client.bottom;
    if(r.left>=r.right || r.top>=r.bottom)return;
    LOGICALPAINT *slot=0;
    for(unsigned i=0;i<128;i++) {
        LOGICALPAINT *p=&g_logical_paints[i];
        if(p->hwnd==hwnd){slot=p;break;}
        if(!slot && (!p->hwnd || !IsWindow(p->hwnd)))slot=p;
    }
    if(!slot)return;
    if(slot->hwnd!=hwnd || !slot->dirty){slot->hwnd=hwnd;slot->rect=r;}
    else {
        if(r.left<slot->rect.left)slot->rect.left=r.left;
        if(r.top<slot->rect.top)slot->rect.top=r.top;
        if(r.right>slot->rect.right)slot->rect.right=r.right;
        if(r.bottom>slot->rect.bottom)slot->rect.bottom=r.bottom;
    }
    slot->dirty=1;
}
static BOOL WINAPI game_invalidate(HWND hwnd,const RECT *rect,BOOL erase)
{
    BOOL result=InvalidateRect(hwnd,rect,erase);
    if(result)remember_paint(hwnd,rect);
    return result;
}
static BOOL WINAPI game_update_rect(HWND hwnd,RECT *rect,BOOL erase)
{
    if(g_forced_paint && hwnd==g_forced_paint){if(rect)*rect=g_forced_rect;return 1;}
    return GetUpdateRect(hwnd,rect,erase);
}
static void *WINAPI game_begin_paint(HWND hwnd,void *paint)
{
    void *dc=BeginPaint(hwnd,paint);
    if(dc && g_forced_paint && hwnd==g_forced_paint)((PAINTSTRUCT*)paint)->paint=g_forced_rect;
    return dc;
}
static void invalidate_logical_tree(HWND hwnd,unsigned depth)
{
    if(depth>16 || !IsWindowVisible(hwnd))return;
    remember_paint(hwnd,0);
    for(HWND child=GetWindow(hwnd,5);child;child=GetWindow(child,2))
        invalidate_logical_tree(child,depth+1);
}
static void repaint_logical_tree(HWND hwnd,unsigned depth)
{
    if(depth>16 || !IsWindowVisible(hwnd))return;
    for(unsigned i=0;i<128;i++)if(g_logical_paints[i].hwnd==hwnd && g_logical_paints[i].dirty) {
        g_forced_rect=g_logical_paints[i].rect;g_logical_paints[i].dirty=0;
        g_forced_paint=hwnd;SendMessageA(hwnd,15,0,0);g_forced_paint=0;
        break;
    }
    for(HWND child=GetWindow(hwnd,5);child;child=GetWindow(child,2))
        repaint_logical_tree(child,depth+1);
}


DLLIMPORT BOOL WINAPI PeekMessageA(void*,HWND,unsigned,unsigned,unsigned);
DLLIMPORT BOOL WINAPI GetMessageA(void*,HWND,unsigned,unsigned);
typedef struct { long x,y; } POINT;
DLLIMPORT HWND WINAPI ChildWindowFromPointEx(HWND,POINT,unsigned);
DLLIMPORT HWND WINAPI GetCapture(void);
typedef struct { HWND hwnd; unsigned message; DWORD wp; long lp; DWORD time; long pt[2]; } INPUTMSG;
static long scale_mouse(HWND child,long lp)
{
    RECT root={0};GetClientRect(g_game_hwnd,&root);
    long pt[2]={(short)(lp&0xffff),(short)((DWORD)lp>>16)};
    MapWindowPoints(child,g_game_hwnd,pt,1);
    if(root.right>0 && root.bottom>0){pt[0]=pt[0]*640/root.right;pt[1]=pt[1]*480/root.bottom;}
    return (long)(((DWORD)(unsigned short)pt[1]<<16)|(unsigned short)pt[0]);
}
static void translate_message(INPUTMSG *m,int removed)
{
    /* Only removing the initial keydown may change window state. Peeking the
     * same event must have no side effects, and repeats must not toggle again. */
    if(g_fullscreen_message && (m->hwnd==g_game_hwnd || IsChild(g_game_hwnd,m->hwnd)) && m->wp==13 && (m->message==0x104 || m->message==0x105) && (m->lp&0x20000000ul)) {
        if(removed && m->message==0x104 && !(m->lp&0x40000000ul))
            PostMessageA(g_game_hwnd,g_fullscreen_message,0,0);
        m->message=0;return;
    }
    /* Run after real Get/PeekMessage, rather than ahead of cnc-ddraw's
     * HandleMessage: otherwise it rewrites MSG.pt using a different scale.
     * PeekMessage copies the queue entry, so PM_NOREMOVE is safe too. */
    if(g_game_hwnd)screen_to_game(m->pt);
    if(g_active_screen && IsWindow(g_active_screen) && IsWindowVisible(g_active_screen)) {
        if(m->message>=0x200 && m->message<=0x209 &&
           (m->hwnd==g_game_hwnd || IsChild(g_game_hwnd,m->hwnd))) {
            HWND original_hwnd=m->hwnd;long original_lp=m->lp;
            long lp=scale_mouse(m->hwnd,m->lp);
            POINT pt={(short)lp,(short)((DWORD)lp>>16)};
            HWND target=g_active_screen,capture=GetCapture();
            if(capture && (capture==g_active_screen || IsChild(g_active_screen,capture))) {
                target=capture;MapWindowPoints(g_game_hwnd,target,&pt,1);
            } else {
                MapWindowPoints(g_game_hwnd,target,&pt,1);
                /* Menus contain real child buttons. A scaled click must hit
                 * the logical button, not just the screen behind that button. */
                for(unsigned depth=0;depth<8;depth++) {
                    HWND child=ChildWindowFromPointEx(target,pt,3);
                    if(!child || child==target)break;
                    MapWindowPoints(target,child,&pt,1);target=child;
                }
            }
            m->lp=(long)(((DWORD)(unsigned short)pt.y<<16)|(unsigned short)pt.x);
            m->hwnd=target;
            char trace[4]={0};GetEnvironmentVariableA("SGENS_DDRAW_TRACE_INPUT",trace,4);
            if(trace[0]=='1' && m->message==0x201) {
                char line[440];unsigned off=0;RECT r={0};long pos[2]={0};
                GetWindowRect(target,&r);MapWindowPoints(0,g_game_hwnd,&r,2);game_cursor(pos);
                concat(line,&off,"INPUT source,raw,target,scaled=");hex32(line,&off,(DWORD)original_hwnd);hex32(line,&off,original_lp);hex32(line,&off,(DWORD)target);hex32(line,&off,m->lp);
                concat(line,&off," cursor=");hex32(line,&off,pos[0]);hex32(line,&off,pos[1]);concat(line,&off," targetrect=");hex32(line,&off,r.left);hex32(line,&off,r.top);hex32(line,&off,r.right);hex32(line,&off,r.bottom);append_log(line);
            }

        }
    }
}
static BOOL WINAPI game_peek(void *message,HWND hwnd,unsigned first,unsigned last,unsigned flags)
{
    BOOL result=PeekMessageA(message,hwnd,first,last,flags);
    if(result)translate_message((INPUTMSG*)message,flags&1);
    return result;
}
static BOOL WINAPI game_get(void *message,HWND hwnd,unsigned first,unsigned last)
{
    BOOL result=GetMessageA(message,hwnd,first,last);
    if(result>0)translate_message((INPUTMSG*)message,1);
    return result;
}
static int register_screen(HWND hwnd,RECT *r)
{
    char cl[64]={0};GetClassNameA(hwnd,cl,64);
    return r->right>=640 && r->bottom>=480 && cl[0]=='A' && cl[1]=='f' && cl[2]=='x' && cl[3]==':';
}
/* Track logical screen identity independently of the physical window size.
 * Raising and focusing newly shown screens restores the former popup behavior,
 * including Escape/Space skip input after resizing or entering fullscreen. */
static void sync_screen_order(void)
{
    static HWND previous[64];static unsigned previous_count;
    HWND current[64],newest=0;unsigned count=0;
    for(HWND child=GetWindow(g_game_hwnd,5);child && count<64;child=GetWindow(child,2)) {
        RECT r={0};GetClientRect(child,&r);
        if(!register_screen(child,&r))continue;
        if(!IsWindowVisible(child))continue;
        unsigned i;for(i=0;i<previous_count;i++)if(previous[i]==child)break;
        if(i==previous_count)newest=child;
        current[count++]=child;
    }
    for(unsigned i=0;i<count;i++)previous[i]=current[i];
    previous_count=count;
    if(newest)SetWindowPos(newest,0,0,0,0,0,0x13);
    HWND active=newest?newest:(count?current[0]:0),focus=GetFocus();
    if(active && active!=g_active_screen)invalidate_logical_tree(active,0);
    g_active_screen=active;
    if(active)repaint_logical_tree(active,0);
    if(active && GetForegroundWindow()==g_game_hwnd && focus!=active && !IsChild(active,focus))
        SetFocus(active);
}
/* One visible presentation target. The backend and game's original child
 * windows retain their logical 640x480 geometry underneath this window. They
 * must not race our scaled blit into the same DC (small flashing image), nor
 * clip our repaint to the old child area after a fullscreen resize.
 * WS_CLIPCHILDREN on the parent and WS_CLIPSIBLINGS on the patched game
 * screens give normal USER32 clipping; no renderer hooks or backend internals.
 * Queued mouse events for this child follow the existing game-coordinate path.
 */
DLLIMPORT unsigned short WINAPI RegisterClassA(const void*);
DLLIMPORT HWND WINAPI CreateWindowExA(DWORD,LPCSTR,LPCSTR,DWORD,int,int,int,int,HWND,HANDLE,HMODULE,void*);
DLLIMPORT void *WINAPI BeginPaint(HWND,void*);
DLLIMPORT BOOL WINAPI EndPaint(HWND,const void*);
DLLIMPORT BOOL WINAPI GdiFlush(void);
typedef struct { unsigned style; void *proc; int cls_extra,win_extra; HMODULE instance; HANDLE icon,cursor,background; LPCSTR menu,name; } WINDOWCLASS;


#include "presentation_filter.h"
static int g_frame_ready;
static int paint_frame(HWND hwnd,void *dc)
{
    RECT r={0};GetClientRect(hwnd,&r);
    if(!g_frame_ready || !dc || r.right<=0 || r.bottom<=0)return 0;
    if(g_filter_failed && g_filter_reset)return 0;
    int filtered=filter_present(hwnd,g_pixels,g_primary_bmi.w,-g_primary_bmi.h,r.right,r.bottom);
    if(filtered>0)return -g_primary_bmi.h;
    if(filtered<0){PostMessageA(hwnd,FILTER_RESET_MESSAGE,0,0);return 0;}
    int lines=StretchDIBits(dc,0,0,r.right,r.bottom,0,0,g_primary_bmi.w,-g_primary_bmi.h,
                          g_pixels,&g_primary_bmi,0,SRCCOPY);
    GdiFlush();
    return lines;
}
static long WINAPI presenter_proc(HWND hwnd,unsigned msg,DWORD wp,long lp)
{
    if(msg==FILTER_RESET_MESSAGE){DestroyWindow(hwnd);return 0;}
    if(msg==0x82){filter_destroy(hwnd);if(hwnd==g_present_hwnd)g_present_hwnd=0;}
    if(msg==15) { /* WM_PAINT: also repaint after expose/compositor damage. */
        PAINTSTRUCT ps={0};void *dc=BeginPaint(hwnd,&ps);
        paint_frame(hwnd,dc);EndPaint(hwnd,&ps);return 0;
    }
    if(msg==20)return 1; /* WM_ERASEBKGND: every frame covers the whole client. */
    if(msg==0x21)return 3; /* MA_NOACTIVATE: focus stays with the game screen. */
    return DefWindowProcA(hwnd,msg,wp,lp);
}
/* A backend may legitimately own window styles and virtualize USER32 for
 * its cooperative HWND. Keep those effects on a separate top-level HWND; the
 * game root and visible presenter remain owned by this adapter. No backend
 * code offsets, private messages or function detours are needed. */
static HWND g_backend_hwnd;
typedef struct {HWND hwnd,after;int x,y,cx,cy;unsigned flags;} BACKEND_WINDOWPOS;
typedef struct {DWORD old_style,new_style;} BACKEND_STYLE;
static long WINAPI backend_window_proc(HWND hwnd,unsigned msg,DWORD wp,long lp)
{
    if(msg==0x21)return 3; /* MA_NOACTIVATE: input belongs to the game. */
    /* SetCooperativeLevel requires a top-level HWND, not a child. Our private
     * render target remains hidden/nonactivating; its pixels are read through
     * DirectDraw, never through the target window's on-screen contents. */
    if(msg==0x46 && lp){BACKEND_WINDOWPOS *p=(BACKEND_WINDOWPOS*)lp;p->flags=(p->flags&~0x40ul)|0x94ul;}
    if(msg==0x7c && wp==(DWORD)-16 && lp)((BACKEND_STYLE*)lp)->new_style&=~0x10000000ul;
    if(msg==0x82 && hwnd==g_backend_hwnd)g_backend_hwnd=0;
    return DefWindowProcA(hwnd,msg,wp,lp);
}
DLLIMPORT BOOL WINAPI ShowWindow(HWND,int);
DLLIMPORT BOOL WINAPI SetForegroundWindow(HWND);
static HWND g_sized_game;
static void prepare_game_window(HWND hwnd)
{
    if(g_sized_game==hwnd)return;
    unsigned w=GetPrivateProfileIntA("sgens-shim","width",640,".\\ddraw.ini");
    unsigned h=GetPrivateProfileIntA("sgens-shim","height",480,".\\ddraw.ini");
    if(w<640 || w>7680)w=640;
    if(h<480 || h>4320)h=480;
    MONITORINFO mi={0};mi.size=sizeof(mi);RECT r={0,0,(long)w,(long)h};
    long style=(GetWindowLongA(hwnd,-16)&~0xa1000000ul)|0x02cf0000ul;
    long exstyle=GetWindowLongA(hwnd,-20);
    g_changing_window=1;
    SetWindowLongA(hwnd,-16,style);AdjustWindowRectEx(&r,style,0,exstyle);
    int x=0,y=0;
    if(GetMonitorInfoA(MonitorFromWindow(hwnd,2),&mi)){
        x=mi.work.left+((mi.work.right-mi.work.left)-(r.right-r.left))/2;
        y=mi.work.top+((mi.work.bottom-mi.work.top)-(r.bottom-r.top))/2;
    }
    SetWindowPos(hwnd,0,x,y,r.right-r.left,r.bottom-r.top,0x30);
    g_changing_window=0;g_sized_game=hwnd;
    ShowWindow(hwnd,1);SetForegroundWindow(hwnd);
    append_log("Shim owns visible game window; size from [sgens-shim] width/height");
}
static HWND backend_window(HWND game)
{
    static unsigned short atom;
    if(g_backend_hwnd && IsWindow(g_backend_hwnd))return g_backend_hwnd;
    HMODULE instance=GetModuleHandleA("ddraw.dll");
    if(!atom){WINDOWCLASS wc={0};wc.proc=(void*)backend_window_proc;wc.instance=instance;
        wc.name="SGENS_BACKEND_HOST";atom=RegisterClassA(&wc);}
    if(!atom)return 0;
    g_backend_hwnd=CreateWindowExA(0x08000080ul,"SGENS_BACKEND_HOST","",0x86000000ul,
                                  0,0,640,480,game,0,instance,0);
    return g_backend_hwnd;
}
static BOOL sync_presenter(void)
{
    static unsigned short atom;
    if(!g_present_hwnd || !IsWindow(g_present_hwnd)) {
        HMODULE instance=GetModuleHandleA("ddraw.dll");
        if(!atom) {
            WINDOWCLASS wc={0};wc.proc=(void*)presenter_proc;wc.instance=instance;
            if(filter_choice())wc.style=0x20; /* CS_OWNDC for the optional GL context. */
            wc.name="SGENS_FRAME_PRESENTER";atom=RegisterClassA(&wc);
        }
        if(!atom)return 0;
        /* Backend GetDC calls must clip the presentation child, including a
         * cached renderer DC whose visible region USER32 updates on resize. */
        long style=GetWindowLongA(g_game_hwnd,-16);
        if(!(style&0x02000000ul))SetWindowLongA(g_game_hwnd,-16,style|0x02000000ul);
        g_present_hwnd=CreateWindowExA(0,"SGENS_FRAME_PRESENTER","",0x54000000ul,
                                      0,0,1,1,g_game_hwnd,0,instance,0);
        if(!g_present_hwnd)return 0;
        append_log("Single presentation child created; backend output clipped behind it");
    }
    RECT r={0},p={0};GetClientRect(g_game_hwnd,&r);GetClientRect(g_present_hwnd,&p);
    if(r.right<=0 || r.bottom<=0)return 0;
    if(r.right!=p.right || r.bottom!=p.bottom || GetWindow(g_game_hwnd,5)!=g_present_hwnd)
        SetWindowPos(g_present_hwnd,0,0,0,r.right,r.bottom,0x10); /* top, no activate */
    return 1;
}
static void debug_frame(DWORD w,DWORD h)
{
    char enabled[4]={0};GetEnvironmentVariableA("SGENS_DDRAW_CAPTURE",enabled,4);
    if(enabled[0]!='1')return;
    BYTE head[14]={0x42,0x4d};DWORD size=54+w*h*4,written;
    for(unsigned i=0;i<4;i++)head[2+i]=(BYTE)(size>>(i*8));
    head[10]=54;
    HANDLE f=CreateFileA("sgens-current.bmp",GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
    if(f!=INVALID_HANDLE_VALUE) {
        WriteFile(f,head,14,&written,0);WriteFile(f,&g_primary_bmi,40,&written,0);
        WriteFile(f,g_pixels,w*h*4,&written,0);CloseHandle(f);
    }
}
/* Outstanding access matters between calls, not only while the backend
 * is on the stack. Replaying WM_PAINT here can re-enter game drawing while
 * an unrelated offscreen upload or GDI DC is still owned by the caller. */
static int game_surface_access_active(void)
{
    for(unsigned i=0;i<MAX_SURFS;i++)
        if(g_surfs[i].iface && (g_surfs[i].locks || g_surfs[i].dcs))return 1;
    return 0;
}
/* Read through a private system-memory surface. This avoids Lock/Unlock
 * on the game's primary: some backends present or upload on primary Unlock,
 * even when our caller only wanted readback. Only documented COM methods. */
static void *g_staging;
static SURFHOOK *g_staging_owner;
static unsigned g_staging_generation;
static SDESC g_staging_desc;
static void log_stage_error(const char *what,HRESULT hr)
{
    static unsigned count;
    if(count++<16){char line[180];unsigned off=0;concat(line,&off,what);hex32(line,&off,(DWORD)hr);append_log(line);}
}
/* The cache owns exactly one staging reference. Captures take a separate
 * reference before Blt/Lock, so nested mode/release notifications may safely
 * retire the cache without destroying a locked readback surface. */
static void discard_staging(void)
{
    void *old=g_staging;
    g_staging=0;g_staging_owner=0;g_staging_generation=0;
    if(old){void **v=*(void***)old;backend_enter();((PFN_RELEASE)v[2])(old);backend_leave();}
}
static void *stage_primary(SURFHOOK *rec)
{
    SDESC actual={0};actual.dwSize=sizeof(actual);
    backend_enter();
    HRESULT hr=((HRESULT(WINAPI*)(void*,SDESC*))rec->original[22])(rec->iface,&actual);
    backend_leave();
    if(hr<0 || !rec->draw || !rec->make_surface)return 0;
    if(!actual.dwWidth || !actual.dwHeight || actual.dwWidth>MAX_WIDTH || actual.dwHeight>MAX_HEIGHT)return 0;
    int different=g_staging_owner!=rec || g_staging_generation!=rec->generation || actual.dwWidth!=g_staging_desc.dwWidth || actual.dwHeight!=g_staging_desc.dwHeight;
    for(unsigned i=0;i<8;i++)if(actual.pixfmt[i]!=g_staging_desc.pixfmt[i])different=1;
    if(g_staging && different)discard_staging();
    if(!g_staging){
        SDESC create={0};void *created=0;
        create.dwSize=sizeof(create);create.dwFlags=0x1007;
        create.dwWidth=actual.dwWidth;create.dwHeight=actual.dwHeight;
        for(unsigned i=0;i<8;i++)create.pixfmt[i]=actual.pixfmt[i];
        create.caps=0x840; /* DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY */
        backend_enter();hr=rec->make_surface(rec->draw,&create,&created,0);backend_leave();
        if(hr<0 || !created){log_stage_error("Staging CreateSurface hr=",hr);return 0;}
        g_staging=created;g_staging_owner=rec;g_staging_generation=rec->generation;g_staging_desc=actual;
        append_log("Created private readback staging surface");
    }
    void *staging=g_staging;void **v=*(void***)staging;
    /* Blt may pump messages too: pin before calling it, not only before Lock. */
    backend_enter();((PFN_RELEASE)v[1])(staging);
    hr=((PFN_SURF_BLT)v[5])(staging,0,rec->iface,0,0x01000000,0); /* DDBLT_WAIT */
    backend_leave();
    if(hr<0){
        log_stage_error("Staging Blt hr=",hr);
        if(g_staging==staging)discard_staging();
        backend_enter();((PFN_RELEASE)v[2])(staging);backend_leave();return 0;
    }
    return staging; /* Caller owns the temporary reference, including on cache retirement. */
}
static ULONG WINAPI surf_release(void *);
static void capture_primary(void)
{
    SURFHOOK *rec=g_primary; SDESC d={0}; DWORD now=GetTickCount();
    if(!rec || backend_busy() || g_presenting || g_changing_window || g_mode_changing || !g_game_hwnd || !IsWindow(g_game_hwnd))return;
    void *iface=rec->iface;
    if(game_surface_access_active())return;
    if(g_frame_tick && now-g_frame_tick<16)return;
    g_presenting=1;
    unsigned generation=rec->generation;
    void *readback=0;void **readvt=0;
    /* Pin the source across logical repaint and backend message pumping. */
    backend_enter();((PFN_RELEASE)rec->original[1])(iface);backend_leave();
    sync_screen_order();
    /* Window/focus notifications may synchronously replace the primary. */
    if(rec!=g_primary || rec->iface!=iface || rec->generation!=generation || game_surface_access_active())goto capture_done;
    d.dwSize=sizeof(d);
    readback=stage_primary(rec);
    if(!readback)goto capture_done;
    readvt=*(void***)readback;
    backend_enter();
    HRESULT readhr=((PFN_SURF_LOCK)readvt[25])(readback,0,&d,0x11,0); /* WAIT | READONLY */
    backend_leave();
    if(readhr<0){log_stage_error("Staging Lock hr=",readhr);discard_staging();goto capture_done;}
    DWORD w=d.dwWidth,h=d.dwHeight,bpp=d.pixfmt[3];
    if(!d.pixels || !w || !h || w>MAX_WIDTH || h>MAX_HEIGHT ||
       (bpp!=8 && bpp!=16 && bpp!=32) || d.lPitch<(long)(w*bpp/8)) {
        backend_enter();
        HRESULT capture_unlock_hr=((PFN_SURF_UNLOCK)readvt[32])(readback,d.pixels);
        backend_leave();
        if(capture_unlock_hr<0){log_stage_error("Staging Unlock hr=",capture_unlock_hr);discard_staging();}
        goto capture_done;
    }
    PALENTRY palette[256]={0};void *pal=0;
    if(bpp==8 && ((HRESULT(WINAPI*)(void*,void**))rec->original[20])(iface,&pal)>=0 && pal) {
        void **vt=*(void***)pal;
        ((PFN_PALETTE_GETENTRIES)vt[4])(pal,0,0,256,palette);
        ((PFN_RELEASE)vt[2])(pal);
    }
    DWORD masks[3]={d.pixfmt[6],d.pixfmt[5],d.pixfmt[4]}, shifts[3]={0};
    if(bpp!=8)for(unsigned c=0;c<3;c++)while(masks[c] && !(masks[c]&1)){masks[c]>>=1;shifts[c]++;}
    for(unsigned y=0;y<h;y++) {
        const BYTE *row=(const BYTE*)d.pixels+y*d.lPitch;
        BYTE *out=g_pixels+y*w*4;
        for(unsigned x=0;x<w;x++,out+=4) {
            if(bpp==8) {PALENTRY p=palette[row[x]];out[0]=p.blue;out[1]=p.green;out[2]=p.red;}
            else {
                DWORD v=bpp==16?((const unsigned short*)row)[x]:((const DWORD*)row)[x];
                for(unsigned c=0;c<3;c++)out[c]=masks[c]?((v>>shifts[c])&masks[c])*255/masks[c]:0;
            }
            out[3]=0;
        }
    }
    backend_enter();
    HRESULT capture_unlock_hr=((PFN_SURF_UNLOCK)readvt[32])(readback,d.pixels);
    backend_leave();
    if(capture_unlock_hr<0){log_stage_error("Staging Unlock hr=",capture_unlock_hr);discard_staging();goto capture_done;}
    if(rec!=g_primary || rec->generation!=generation || g_staging_owner!=rec || g_staging_generation!=generation)goto capture_done;
    g_primary_bmi.size=40;g_primary_bmi.w=w;g_primary_bmi.h=-(long)h;
    g_primary_bmi.planes=1;g_primary_bmi.bpp=32;g_primary_bmi.compression=0;
    g_primary_bmi.imagebytes=w*h*4;g_frame_ready=1;
    HWND dst=g_present_hwnd;int painted=0;
    if(sync_presenter()) {
        dst=g_present_hwnd;void *dc=GetDCEx(dst,0,0x12);
        if(dc) {painted=paint_frame(dst,dc);ReleaseDC(dst,dc);}
    }
    g_frame_tick=now;
    static DWORD frames;
    if(frames%120==0)debug_frame(w,h);
    char trace_frames[4]={0};GetEnvironmentVariableA("SGENS_DDRAW_TRACE_FRAMES",trace_frames,4);
    if(++frames==1 || frames==60 || frames==600 || (trace_frames[0]=='1' && frames%120==0) || !painted) {
        char line[220];unsigned off=0;
        concat(line,&off,"Primary frame tick=");hex32(line,&off,now);concat(line,&off," bpp=");hex32(line,&off,bpp);
        concat(line,&off," hwnd=");hex32(line,&off,(DWORD)dst);
        concat(line,&off," lines=");hex32(line,&off,(DWORD)painted);append_log(line);
    }
    /* The game's startup geometry assumes a normal 640x480 client. Request
     * borderless mode only after that primary has begun producing frames. */
    static int startup_done;
    if(!startup_done && w==640 && h==480 && frames>=10 && IsWindowVisible(g_game_hwnd)) {
        startup_done=1;
        if(GetPrivateProfileIntA("sgens-shim","fullscreen",0,".\\ddraw.ini")) {
            PostMessageA(g_game_hwnd,g_fullscreen_message,2,0);
            append_log("Deferred borderless fullscreen requested");
        }
    }
capture_done:
    if(readback){backend_enter();((PFN_RELEASE)readvt[2])(readback);backend_leave();}
    surf_release(iface);
    g_presenting=0;
}

/* Guard only physical mode changes. CDS_TEST is a documented read-only query:
 * forward it unchanged, preserving the backend's mode-selection decisions.
 * Match resolved public USER32 API addresses, not private backend code/data.
 * This covers static imports in the loaded backend, not dynamically resolved
 * calls or other modules. Such backends require their own safety validation. */
typedef long (WINAPI *DISPLAY_FN)(void*,DWORD);
typedef long (WINAPI *DISPLAY_EX_FN)(void*,void*,HWND,DWORD,void*);
static DISPLAY_FN g_display_a,g_display_w;
static DISPLAY_EX_FN g_display_ex_a,g_display_ex_w;
static long blocked_display(void *mode)
{
    append_log(mode ? "Blocked physical display-mode change" : "Suppressed physical display reset");
    return mode ? -2 : 0; /* DISP_CHANGE_BADMODE / DISP_CHANGE_SUCCESSFUL */
}
static long WINAPI guarded_display_a(void *mode,DWORD flags)
{
    if(flags&2)return g_display_a(mode,flags); /* CDS_TEST */
    return blocked_display(mode);
}
static long WINAPI guarded_display_w(void *mode,DWORD flags)
{
    if(flags&2)return g_display_w(mode,flags);
    return blocked_display(mode);
}
static long WINAPI guarded_display_ex_a(void *dev,void *mode,HWND hwnd,DWORD flags,void *extra)
{
    if(flags&2)return g_display_ex_a(dev,mode,hwnd,flags,extra);
    return blocked_display(mode);
}
static long WINAPI guarded_display_ex_w(void *dev,void *mode,HWND hwnd,DWORD flags,void *extra)
{
    if(flags&2)return g_display_ex_w(dev,mode,hwnd,flags,extra);
    return blocked_display(mode);
}
#include "import_guard.h"
static void guard_backend_display(HMODULE module)
{
    HMODULE user=GetModuleHandleA("user32.dll");
    g_display_a=(DISPLAY_FN)GetProcAddress(user,"ChangeDisplaySettingsA");
    g_display_w=(DISPLAY_FN)GetProcAddress(user,"ChangeDisplaySettingsW");
    g_display_ex_a=(DISPLAY_EX_FN)GetProcAddress(user,"ChangeDisplaySettingsExA");
    g_display_ex_w=(DISPLAY_EX_FN)GetProcAddress(user,"ChangeDisplaySettingsExW");
    IMPORT_PATCH patches[]={
        {(void*)g_display_a,(void*)guarded_display_a},
        {(void*)g_display_w,(void*)guarded_display_w},
        {(void*)g_display_ex_a,(void*)guarded_display_ex_a},
        {(void*)g_display_ex_w,(void*)guarded_display_ex_w}
    };
    unsigned installed=0;
    if(!patch_import_addresses(module,patches,4,&installed)){
        g_backend_failed=1;append_log("ERROR: backend import table or display guard update failed");return;
    }
    char line[180];unsigned off=0;
    concat(line,&off,"Display guard: static backend slots adapted=");hex32(line,&off,installed);append_log(line);
    if(!installed)append_log("Display guard: no matching static slots; backend display coverage unverified");
}

static HMODULE backend(void)
{
    if (g_backend) return g_backend;
    if (g_backend_failed) return 0;
    /* Generations frees ddraw.dll before destroying all its windows on XP.
     * Presenter/window procedures and installed IAT entries still point here:
     * unloading at that point caused an execute fault in presenter_proc.
     * Keep this process-local adapter resident until process termination.
     * This documented XP API does not change backend COM/rendering behavior.
     * Pin before publishing any callbacks, outside DllMain's loader lock.
     */
    HMODULE self=0;
    if(!GetModuleHandleExA(0x00000005ul /* PIN | FROM_ADDRESS */,
                          (LPCSTR)&g_backend,&self)) {
        g_backend_failed=1;
        append_log("ERROR: cannot retain shim for game-window lifetime");
        return 0;
    }
    g_backend = LoadLibraryA("ddraw-backend.dll");
    if (!g_backend) {
        g_backend_failed = 1;
        append_log("ERROR: ddraw-backend.dll cannot be loaded (copy/rename 32-bit cnc-ddraw ddraw.dll)");
    } else {
        /* Only an explicit 0 disables protection; missing/invalid values
         * stay protected. Read once before any backend DirectDraw entry point.
         * Disabled means no IAT changes: original API results/side effects.
         */
        char protect[16]={0};
        GetPrivateProfileStringA("sgens-shim","block_display_changes","1",protect,sizeof(protect),".\\ddraw.ini");
        if(protect[0]=='0' && !protect[1])
            append_log("Display protection disabled by configuration: backend APIs untouched");
        else {
            guard_backend_display(g_backend);
            append_log("Display guard requested: matched static calls restricted; dynamic/dependency calls not covered");
        }
        if(g_backend_failed){g_backend=0;return 0;}
        append_log("Backend loaded: ddraw-backend.dll; shim v12; logical 60 Hz timing; deferred presentation, logical UI repaint and optional final-image filtering");
    }
    return g_backend;
}
static HOOK *find_hook(void *iface)
{
    unsigned i;
    for (i = 0; i < MAX_HOOKS; i++) if (g_hooks[i].iface == iface && iface) return &g_hooks[i];
    return 0;
}

static HRESULT WINAPI proxy_query(void *, const GUID *, void **);
static HRESULT WINAPI proxy_create_surface(void *,SDESC *,void **,void *);
static int hook_surface(void *, const SDESC *);
static ULONG WINAPI proxy_release(void *);
static BOOL WINAPI log_process_window(HWND, long);
static HRESULT WINAPI proxy_coop(void *, HWND, DWORD);
static HRESULT WINAPI proxy_restore_mode(void *);
static HRESULT WINAPI proxy_mode1(void *, DWORD, DWORD, DWORD);
static HRESULT WINAPI proxy_mode2(void *, DWORD, DWORD, DWORD, DWORD, DWORD);
static HRESULT WINAPI proxy_palette(void *, DWORD, void *, void **, void *);

static int hook_interface(void *iface, unsigned int version)
{
    unsigned i, n = vtbl_count(version);
    HOOK *rec = 0;
    void **original;
    char line[128];
    unsigned off = 0;
    if (!iface || !n || find_hook(iface)) return 1;
    if (!(original = *(void ***)iface)) return 0;
    for (i = 0; i < MAX_HOOKS; ++i) if (!g_hooks[i].iface) {rec = &g_hooks[i]; break;}
    if (!rec) { append_log("ERROR: no free hook slots"); return 0; }

    /* Clone only our COM forwarding table. iface belongs to this shim,
     * never to the backend. Its objects and internal calls stay untouched. */
    for (i = 0; i < n; ++i) rec->proxy[i] = rec->original[i] = original[i];
    rec->nmethods = n;
    rec->version = version;
    rec->proxy[0] = (void *)proxy_query;
    rec->proxy[2] = (void *)proxy_release;
    rec->proxy[5] = (void *)proxy_palette;
    /* Generations uses IDirectDraw 1/2 and DDSURFACEDESC (108 bytes).
     * The COM boundary rejects newer interfaces instead of exposing unadapted layouts. */
    if(version<=3)rec->proxy[6] = (void *)proxy_create_surface;
    rec->proxy[19] = (void *)proxy_restore_mode;
    rec->proxy[20] = (void *)proxy_coop;
    rec->proxy[21] = version == 1 ? (void *)proxy_mode1 : (void *)proxy_mode2;
    rec->iface = iface;
    *(void ***)iface = rec->proxy;

    concat(line, &off, "Hooked IDirectDraw version=");
    hex32(line, &off, version);
    append_log(line);
    return 1;
}

static HRESULT WINAPI proxy_query(void *iface, const GUID *iid, void **out)
{
    HOOK *rec = find_hook(iface);
    HRESULT hr;
    if (!rec) return E_FAIL;
    backend_enter();
    hr = ((PFN_QUERY)rec->original[0])(iface, iid, out);
    backend_leave();
    if (hr >= 0 && out && *out && draw_version(iid)) hook_interface(*out, draw_version(iid));
    return hr;
}

static ULONG WINAPI proxy_release(void *iface)
{
    HOOK *rec = find_hook(iface);
    ULONG refs;
    if (!rec) return 0;
    backend_enter();
    refs = ((PFN_RELEASE)rec->original[2])(iface);
    backend_leave();
    if (!refs) {unsigned i;for(i=0;i<MAX_HOOKS;i++)if(g_hooks[i].iface && !com_find(g_hooks[i].iface))g_hooks[i].iface=0;}
    return refs;
}

static HRESULT WINAPI proxy_coop(void *iface, HWND hwnd, DWORD flags)
{
    HOOK *rec = find_hook(iface);
    HRESULT hr;
    char line[256];
    unsigned off = 0;
    if (!rec) return E_FAIL;
    if(g_game_hwnd && g_timer)KillTimer(g_game_hwnd,g_timer);
    g_timer=0;g_game_hwnd=hwnd;
    /* DDSCL_NORMAL with NULL HWND is valid at shutdown. Never create a
     * thread timer or try to subclass NULL during that transition. */
    if(hwnd) {
        g_timer=SetTimer(hwnd,0x5347,33,(void*)primary_timer);
        if(!g_timer)append_log("ERROR: primary repaint timer unavailable");
    }
    concat(line, &off, "SetCooperativeLevel original_hwnd=");
    hex32(line, &off, (unsigned long)hwnd);
    concat(line, &off, " flags=");
    hex32(line, &off, flags);
    HWND target=hwnd;
    if(hwnd) {
        hook_game_imports();install_window_proc(hwnd);prepare_game_window(hwnd);
        target=backend_window(hwnd);
        if(!target)return E_FAIL;
    }
    concat(line,&off," backend_hwnd=");hex32(line,&off,(DWORD)target);
    backend_enter();
    hr = ((PFN_COOP)rec->original[20])(iface, target, flags);
    backend_leave();
    hook_game_imports();
    install_window_proc(hwnd);
    concat(line, &off, " hr=");
    hex32(line, &off, (unsigned long)hr);
    append_log(line);
    EnumWindows((void *)log_process_window, 0);
    return hr;
}
static BOOL WINAPI log_process_window(HWND hwnd, long unused)
{
    DWORD pid = 0;
    char clazz[96], title[96], line[400];
    RECT rect = {0,0,0,0}; unsigned off=0;
    (void)unused;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid != GetCurrentProcessId()) return 1;
    clazz[0]=0; title[0]=0;
    GetClassNameA(hwnd, clazz, sizeof(clazz));
    GetWindowTextA(hwnd, title, sizeof(title));
    GetWindowRect(hwnd, &rect);
    concat(line,&off,"Game-window hwnd="); hex32(line,&off,(unsigned long)hwnd);
    concat(line,&off," class="); concat(line,&off,clazz);
    concat(line,&off," title="); concat(line,&off,title);
    concat(line,&off," left="); hex32(line,&off,(DWORD)rect.left);
    concat(line,&off," top="); hex32(line,&off,(DWORD)rect.top);
    concat(line,&off," right="); hex32(line,&off,(DWORD)rect.right);
    concat(line,&off," bottom="); hex32(line,&off,(DWORD)rect.bottom);
    append_log(line);
    return 1;
}

static void log_mode(DWORD w, DWORD h, DWORD bpp, DWORD refresh, HRESULT hr)
{
    char line[256];
    unsigned off = 0;
    concat(line, &off, "SetDisplayMode width="); hex32(line, &off, w);
    concat(line, &off, " height="); hex32(line, &off, h);
    concat(line, &off, " bpp="); hex32(line, &off, bpp);
    concat(line, &off, " refresh="); hex32(line, &off, refresh);
    concat(line, &off, " hr="); hex32(line, &off, (unsigned long)hr);
    append_log(line);
}
static HRESULT WINAPI proxy_restore_mode(void *iface)
{
    HOOK *rec=find_hook(iface);if(!rec)return E_FAIL;
    ++g_mode_changing;discard_staging();g_frame_ready=0;g_frame_tick=0;
    backend_enter();HRESULT hr=((HRESULT(WINAPI*)(void*))rec->original[19])(iface);backend_leave();
    --g_mode_changing;return hr;
}
static HRESULT WINAPI proxy_mode1(void *iface, DWORD w, DWORD h, DWORD bpp)
{
    HOOK *rec = find_hook(iface);
    HRESULT hr;
    if (!rec) return E_FAIL;
    ++g_mode_changing;
    discard_staging();g_frame_ready=0;g_frame_tick=0;
    backend_enter();
    hr = ((PFN_MODE1)rec->original[21])(iface, w, h, bpp);
    backend_leave();
    hook_game_imports();
    if(hr>=0)restore_borderless_geometry();
    --g_mode_changing;
    log_mode(w, h, bpp, 0, hr);
    EnumWindows((void *)log_process_window, 0);
    return hr;
}
static HRESULT WINAPI proxy_mode2(void *iface, DWORD w, DWORD h, DWORD bpp, DWORD refresh, DWORD flags)
{
    HOOK *rec = find_hook(iface);
    HRESULT hr;
    if (!rec) return E_FAIL;
    ++g_mode_changing;
    discard_staging();g_frame_ready=0;g_frame_tick=0;
    backend_enter();
    hr = ((PFN_MODE2)rec->original[21])(iface, w, h, bpp, refresh, flags);
    backend_leave();
    hook_game_imports();
    if(hr>=0)restore_borderless_geometry();
    --g_mode_changing;
    log_mode(w, h, bpp, refresh, hr);
    EnumWindows((void *)log_process_window, 0);
    return hr;
}
static HRESULT WINAPI proxy_palette(void *iface, DWORD flags, void *entries, void **palette, void *outer)
{
    HOOK *rec = find_hook(iface);
    HRESULT hr;
    char line[128]; unsigned off = 0;
    if (!rec) return E_FAIL;
    backend_enter();
    hr = ((PFN_PALETTE)rec->original[5])(iface, flags, entries, palette, outer);
    backend_leave();
    concat(line, &off, "CreatePalette flags="); hex32(line, &off, flags);
    concat(line, &off, " hr="); hex32(line, &off, (unsigned long)hr);
    append_log(line);
    return hr;
}


static SURFHOOK *find_surface(void *iface)
{
    unsigned i;
    for (i=0;i<MAX_SURFS;i++) if (g_surfs[i].iface==iface && iface) return &g_surfs[i];
    return 0;
}
/* Retain the exact draw interface used for creation until this surface dies.
 * It is legal for the game to Release its draw pointer before its surfaces.
 * Never retain only a borrowed function/this pair for a future CreateSurface. */
static void bind_surface_owner(SURFHOOK *surface,void *draw)
{
    HOOK *owner=find_hook(draw);
    if(!surface || surface->draw || !owner || owner->version>3)return;
    backend_enter();((PFN_RELEASE)owner->original[1])(draw);backend_leave();
    surface->draw=draw;surface->make_surface=(PFN_CREATESURF)owner->original[6];
}
/* Notifications for public surface outputs outside CreateSurface/GetAttached. */
static void com_surface_ready(void *surface,const SDESC *desc,void *owner)
{
    hook_surface(surface,desc);bind_surface_owner(find_surface(surface),owner);
}
static void *com_surface_owner(void *surface)
{
    SURFHOOK *rec=find_surface(surface);return rec?rec->draw:0;
}
static HRESULT WINAPI surf_restore(void *iface)
{
    SURFHOOK *rec=find_surface(iface);if(!rec)return E_FAIL;
    if(g_staging_owner==rec)discard_staging();
    backend_enter();HRESULT hr=((HRESULT(WINAPI*)(void*))rec->original[27])(iface);backend_leave();
    return hr;
}
static HRESULT WINAPI surf_query(void *iface, const GUID *iid, void **out)
{
    SURFHOOK *rec = find_surface(iface);
    HRESULT hr;
    if (!rec) return E_FAIL;
    backend_enter();
    hr = ((PFN_QUERY)rec->original[0])(iface,iid,out);
    backend_leave();
    /* Surface v2/3/4/7 have different vtable sizes; do not assume v1. */
    if (hr>=0 && out && *out && *out!=iface) append_log("Surface QueryInterface: returned different pointer (not hooked)");
    return hr;
}
static ULONG WINAPI surf_release(void *iface)
{
    SURFHOOK *rec=find_surface(iface);
    ULONG n;
    if (!rec) return 0;
    backend_enter();
    n=((PFN_RELEASE)rec->original[2])(iface);
    backend_leave();
    if (!n) {
        void *owner=rec->draw;
        if(g_staging_owner==rec && g_staging_generation==rec->generation)discard_staging();
        if(g_primary==rec){g_primary=0;g_frame_ready=0;}
        rec->iface=0;rec->draw=0;rec->make_surface=0;rec->locks=rec->dcs=0;
        if(owner)proxy_release(owner);
    }
    return n;
}
static void log_surf_event(SURFHOOK *rec, const char *name, DWORD val, HRESULT hr)
{
    char line[180]; unsigned off=0;
    concat(line,&off,name);
    concat(line,&off," surface="); hex32(line,&off,(DWORD)rec->iface);
    concat(line,&off," flags="); hex32(line,&off,val);
    concat(line,&off," hr="); hex32(line,&off,(DWORD)hr);
    append_log(line);
}
static HRESULT WINAPI surf_flip(void *iface,void *other,DWORD flags)
{
    SURFHOOK *rec=find_surface(iface); HRESULT hr;
    if (!rec) return E_FAIL;
    backend_enter();
    hr=((PFN_SURF_FLIP)rec->original[11])(iface,other,flags);
    backend_leave();
    rec->called_flip++;
    if (rec->called_flip<=3 || rec->called_flip==60)
        log_surf_event(rec,"Flip",flags,hr);
    if (rec->called_flip==1) EnumWindows((void*)log_process_window,0);
    return hr;
}
static HRESULT WINAPI surf_blt(void *iface,RECT *dst,void *src,RECT *sr,DWORD flags,void *fx)
{
    SURFHOOK *rec=find_surface(iface); HRESULT hr;
    if (!rec) return E_FAIL;
    backend_enter();
    hr=((PFN_SURF_BLT)rec->original[5])(iface,dst,src,sr,flags,fx);
    backend_leave();
    rec->called_blt++;
    if (rec->called_blt<=4 || rec->called_blt==10 || rec->called_blt==60) {
        char line[220]; unsigned off=0;
        concat(line,&off,"Blt dst=");hex32(line,&off,(DWORD)iface);
        concat(line,&off," src=");hex32(line,&off,(DWORD)src);
        concat(line,&off," flags=");hex32(line,&off,flags);
        concat(line,&off," colorfill=");concat(line,&off,(flags&0x00000400ul)?"yes":"no");
        concat(line,&off," hr=");hex32(line,&off,(DWORD)hr);
        append_log(line);
    }
    return hr;
}
static HRESULT WINAPI surf_bltfast(void *iface,DWORD x,DWORD y,void *src,RECT *sr,DWORD flags)
{
    SURFHOOK *rec=find_surface(iface); HRESULT hr;
    if (!rec) return E_FAIL;
    backend_enter();
    hr=((PFN_SURF_BLTFAST)rec->original[7])(iface,x,y,src,sr,flags);
    backend_leave();
    rec->called_blt++;
    if (rec->called_blt<=4 || rec->called_blt==10 || rec->called_blt==60) {
        char line[220]; unsigned off=0;
        concat(line,&off,"BltFast dst=");hex32(line,&off,(DWORD)iface);
        concat(line,&off," src=");hex32(line,&off,(DWORD)src);
        concat(line,&off," flags=");hex32(line,&off,flags);
        concat(line,&off," hr=");hex32(line,&off,(DWORD)hr);
        append_log(line);
    }
    return hr;
}
static HRESULT WINAPI surf_attached(void *iface,void *caps,void **out)
{
    SURFHOOK *rec=find_surface(iface); HRESULT hr;
    if (!rec) return E_FAIL;
    backend_enter();
    hr=((PFN_SURF_ATTACHED)rec->original[12])(iface,caps,out);
    backend_leave();
    if (hr>=0 && out && *out){hook_surface(*out,0);bind_surface_owner(find_surface(*out),rec->draw);}
    return hr;
}
static HRESULT WINAPI surf_setpalette(void *iface,void *palette)
{
    SURFHOOK *rec=find_surface(iface); HRESULT hr;
    if (!rec) return E_FAIL;
    backend_enter();
    hr=((PFN_SURF_PALETTE)rec->original[31])(iface,palette);
    backend_leave();
    if (hr>=0) rec->palette=palette;
    log_surf_event(rec,"SetPalette",(DWORD)palette,hr);
    return hr;
}
static HRESULT WINAPI surf_lock(void *iface,RECT *rect,SDESC *desc,DWORD flags,void *event)
{
    SURFHOOK *rec=find_surface(iface); HRESULT hr;
    if (!rec) return E_FAIL;
    backend_enter();
    hr=((PFN_SURF_LOCK)rec->original[25])(iface,rect,desc,flags,event);
    if(hr>=0)++rec->locks;
    backend_leave();
    /* Log only first lock so that heavy game rendering does not flood the file. */
    if (hr<0 || rec->called_unlock < 2) log_surf_event(rec,"Lock",flags,hr);
    return hr;
}
static HRESULT WINAPI surf_unlock(void *iface,void *pixels)
{
    SURFHOOK *rec=find_surface(iface); HRESULT hr;
    if (!rec) return E_FAIL;
    backend_enter();
    hr=((PFN_SURF_UNLOCK)rec->original[32])(iface,pixels);
    if(hr>=0 && rec->locks)--rec->locks;
    backend_leave();
    rec->called_unlock++;
    if (rec->called_unlock<=2) log_surf_event(rec,"Unlock",0,hr);
    return hr;
}
/* Record GDI painting into DirectDraw offscreen buffers. Some legacy
 * UI pipelines use IDirectDrawSurface::GetDC/ReleaseDC rather than Lock. */
static HRESULT WINAPI surf_getdc(void *iface,void **dc)
{
    SURFHOOK *rec=find_surface(iface); HRESULT hr;
    if (!rec) return E_FAIL;
    backend_enter();
    hr=((PFN_SURF_GETDC)rec->original[17])(iface,dc);
    if(hr>=0)++rec->dcs;
    backend_leave();
    if (rec->called_releasedc<2) log_surf_event(rec,"GetDC",0,hr);
    return hr;
}
static HRESULT WINAPI surf_releasedc(void *iface,void *dc)
{
    SURFHOOK *rec=find_surface(iface); HRESULT hr;
    if (!rec) return E_FAIL;
    backend_enter();
    hr=((PFN_SURF_RELEASEDC)rec->original[26])(iface,dc);
    if(hr>=0 && rec->dcs)--rec->dcs;
    backend_leave();
    rec->called_releasedc++;
    if (rec->called_releasedc<=2 || rec->called_releasedc==10 || rec->called_releasedc==60)
        log_surf_event(rec,"ReleaseDC",0,hr);
    return hr;
}
static int hook_surface(void *iface,const SDESC *desc)
{
    SURFHOOK *rec=0; void **vtable;
    unsigned i; char line[200]; unsigned off=0;
    if (!iface || find_surface(iface)) return 1;
    vtable=*(void ***)iface;
    if (!vtable) return 0;
    for (i=0;i<MAX_SURFS;i++) if (!g_surfs[i].iface) {rec=&g_surfs[i];break;}
    if (!rec) { append_log("ERROR: no surface hook slots");return 0;}
    for (i=0;i<SURF_VTBL;i++) rec->proxy[i]=rec->original[i]=vtable[i];
    rec->caps=desc?desc->caps:0;
    rec->width=desc?desc->dwWidth:0;
    rec->height=desc?desc->dwHeight:0;
    if (!desc || !rec->caps || !rec->width || !rec->height) {
        SDESC actual={0}; actual.dwSize=sizeof(actual);
        if (((HRESULT (WINAPI *)(void *,SDESC *))vtable[22])(iface,&actual)>=0) {
            rec->caps=actual.caps;
            rec->width=actual.dwWidth;
            rec->height=actual.dwHeight;
        }
    }
    rec->palette=0;rec->locks=rec->dcs=0;rec->draw=0;rec->make_surface=0;
    rec->generation=++g_surface_generation;
    rec->slot=(unsigned)(rec-g_surfs);
    rec->called_flip=rec->called_blt=rec->called_unlock=rec->called_releasedc=0;
    rec->proxy[0]=(void*)surf_query;
    rec->proxy[2]=(void*)surf_release;
    rec->proxy[5]=(void*)surf_blt;
    rec->proxy[7]=(void*)surf_bltfast;
    rec->proxy[11]=(void*)surf_flip;
    rec->proxy[17]=(void*)surf_getdc;
    rec->proxy[26]=(void*)surf_releasedc;
    rec->proxy[27]=(void*)surf_restore;
    rec->proxy[12]=(void*)surf_attached;
    rec->proxy[25]=(void*)surf_lock;
    rec->proxy[31]=(void*)surf_setpalette;
    rec->proxy[32]=(void*)surf_unlock;
    rec->iface=iface;
    *(void ***)iface=rec->proxy;
    if(rec->caps&DDSCAPS_PRIMARYSURFACE){discard_staging();g_primary=rec;g_frame_ready=0;g_frame_tick=0;}
    concat(line,&off,"CreateSurface/hook ptr=");hex32(line,&off,(DWORD)iface);
    concat(line,&off," caps=");hex32(line,&off,rec->caps);
    concat(line,&off," w=");hex32(line,&off,rec->width);
    concat(line,&off," h=");hex32(line,&off,rec->height);
    append_log(line);
    return 1;
}
static HRESULT WINAPI proxy_create_surface(void *iface,SDESC *desc,void **out,void *outer)
{
    HOOK *rec=find_hook(iface); HRESULT hr;
    if (!rec) return E_FAIL;
    backend_enter();
    /* Generations draws these plain bitmap buffers entirely with CPU/GDI.
     * Keep them in system memory instead of asking a GPU-backed implementation
     * to map video memory for every upload. This also avoids the dgVoodoo bitmap
     * upload failures. Primary, flip, texture and other specialized surfaces
     * retain their requested storage; no backend name or private state is used.
     * Never modify the caller's descriptor. Clear both video-pool hints too. */
    SDESC cpu;
    const DWORD video_bitmap=0x4040ul,video_hints=0x30000000ul;
    if(desc && desc->dwSize==sizeof(cpu) && (desc->dwFlags&1) &&
       (desc->caps&~video_hints)==video_bitmap){
        cpu=*desc;cpu.caps=(cpu.caps&~(0x4000ul|video_hints))|0x800ul;desc=&cpu;
    }
    hr=((PFN_CREATESURF)rec->original[6])(iface,desc,out,outer);
    backend_leave();
    if (hr>=0 && out && *out) {
        hook_surface(*out,desc);
        SURFHOOK *surface=find_surface(*out);
        bind_surface_owner(surface,iface);
    }
    else {char line[120]; unsigned off=0; concat(line,&off,"CreateSurface failed hr=");hex32(line,&off,(DWORD)hr);append_log(line);}
    return hr;
}
/* All imports of the game's dynamically loaded ddraw.dll resolve here.
 * The .def file ensures the exact UNDECORATED PE export names. */
HRESULT WINAPI DirectDrawCreate(const GUID *guid, void **out, void *outer)
{
    HMODULE mod = backend();
    PFN_DRAWCREATE fn;
    HRESULT hr;
    if (!mod) return E_FAIL;
    fn = (PFN_DRAWCREATE)GetProcAddress(mod, "DirectDrawCreate");
    if (!fn) {append_log("ERROR: backend lacks DirectDrawCreate"); return E_FAIL;}
    if(!out)return E_POINTER;*out=0;if(outer)return CLASS_E_NOAGGREGATION;
    void *inner=0;hr=fn(guid,&inner,0);
    if(hr>=0)hr=com_wrap_owned(inner,COM_DRAW1,out);
    if (hr >= 0 && out && *out) hook_interface(*out, 1);
    return hr;
}
HRESULT WINAPI DirectDrawCreateEx(const GUID *guid, void **out, const GUID *iid, void *outer)
{
    HMODULE mod = backend();
    PFN_DRAWCREATEEX fn;
    HRESULT hr;
    if (!mod) return E_FAIL;
    fn = (PFN_DRAWCREATEEX)GetProcAddress(mod, "DirectDrawCreateEx");
    if (!fn) {append_log("ERROR: backend lacks DirectDrawCreateEx"); return E_FAIL;}
    if(!out)return E_POINTER;*out=0;if(outer)return CLASS_E_NOAGGREGATION;
    unsigned kind=com_kind(iid);if(kind<COM_DRAW1 || kind>COM_DRAW3)return E_NOINTERFACE;
    void *inner=0;hr=fn(guid,&inner,iid,0);
    if(hr>=0)hr=com_wrap_owned(inner,kind,out);
    if (hr >= 0 && out && *out) hook_interface(*out, draw_version(iid));
    return hr;
}


/* Standard DLL entry points other than DirectDrawCreate are ordinary forwards.
 * The game is known to use DirectDrawCreate, but forwarding the others avoids
 * needlessly breaking a helper library that dynamically queries them. */
HRESULT WINAPI DirectDrawCreateClipper(DWORD flags, void **out, void *outer)
{
    HMODULE m = backend();
    PFN_CREATECLIPPER fn = m ? (PFN_CREATECLIPPER)GetProcAddress(m, "DirectDrawCreateClipper") : 0;
    if(!out)return E_POINTER;*out=0;if(outer)return CLASS_E_NOAGGREGATION;
    void *inner=0;HRESULT hr=fn?fn(flags,&inner,0):E_FAIL;
    return hr<0?hr:com_wrap_owned(inner,COM_CLIPPER,out);
}
HRESULT WINAPI DirectDrawEnumerateA(void *cb, void *ctx)
{
    HMODULE m = backend();
    PFN_ENUM fn = m ? (PFN_ENUM)GetProcAddress(m, "DirectDrawEnumerateA") : 0;
    return fn ? fn(cb, ctx) : E_FAIL;
}
HRESULT WINAPI DirectDrawEnumerateW(void *cb, void *ctx)
{
    HMODULE m = backend();
    PFN_ENUM fn = m ? (PFN_ENUM)GetProcAddress(m, "DirectDrawEnumerateW") : 0;
    return fn ? fn(cb, ctx) : E_FAIL;
}
HRESULT WINAPI DirectDrawEnumerateExA(void *cb, void *ctx, DWORD flags)
{
    HMODULE m = backend();
    PFN_ENUMEX fn = m ? (PFN_ENUMEX)GetProcAddress(m, "DirectDrawEnumerateExA") : 0;
    return fn ? fn(cb, ctx, flags) : E_FAIL;
}
HRESULT WINAPI DirectDrawEnumerateExW(void *cb, void *ctx, DWORD flags)
{
    HMODULE m = backend();
    PFN_ENUMEX fn = m ? (PFN_ENUMEX)GetProcAddress(m, "DirectDrawEnumerateExW") : 0;
    return fn ? fn(cb, ctx, flags) : E_FAIL;
}
void WINAPI AcquireDDThreadLock(void)
{
    HMODULE m = backend();
    void (WINAPI *fn)(void) = m ? (void (WINAPI *)(void))GetProcAddress(m, "AcquireDDThreadLock") : 0;
    if (fn) fn();
}
void WINAPI ReleaseDDThreadLock(void)
{
    HMODULE m = backend();
    void (WINAPI *fn)(void) = m ? (void (WINAPI *)(void))GetProcAddress(m, "ReleaseDDThreadLock") : 0;
    if (fn) fn();
}

BOOL WINAPI DllMain(HMODULE module, DWORD reason, void *reserved)
{
    (void)module; (void)reserved;
    if(reason==0){if(g_timer && g_game_hwnd)KillTimer(g_game_hwnd,g_timer);}
    return 1;
}

void *memset(void *dst,int value,unsigned long n){volatile BYTE *p=dst;while(n--)*p++=(BYTE)value;return dst;}
