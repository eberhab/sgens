/* Game-facing public-API boundary, MIT license.
 * Only COMOBJ belongs to this DLL. Never write to a backend COM object.
 * This game adapter exposes Draw 1/2/3, Surface 1, Palette and Clipper.
 * State has the same game-thread requirement as the presentation layer.
 */
#define E_POINTER ((HRESULT)0x80004003ul)
#define E_NOINTERFACE ((HRESULT)0x80004002ul)
#define E_OUTOFMEMORY ((HRESULT)0x8007000eul)
#define E_INVALIDARG ((HRESULT)0x80070057ul)
#define E_NOTIMPL ((HRESULT)0x80004001ul)
#define CLASS_E_NOAGGREGATION ((HRESULT)0x80040110ul)
DLLIMPORT HANDLE WINAPI GetProcessHeap(void);
DLLIMPORT void *WINAPI HeapAlloc(HANDLE,DWORD,DWORD);
DLLIMPORT BOOL WINAPI HeapFree(HANDLE,DWORD,void*);
static const GUID com_iunknown={0,0,0,{0xc0,0,0,0,0,0,0,0x46}};
static const GUID com_surface_iid={0x6c14db81,0xa733,0x11ce,{0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60}};
static const GUID com_palette_iid={0x6c14db84,0xa733,0x11ce,{0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60}};
static const GUID com_clipper_iid={0x6c14db85,0xa733,0x11ce,{0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60}};
enum { COM_DRAW1=1,COM_DRAW2=2,COM_DRAW3=3,COM_SURFACE=4,COM_PALETTE=5,COM_CLIPPER=6 };
#define COM_LIMIT 512
struct COMGROUP;
typedef struct {void **vtable;void *inner;struct COMGROUP *group;unsigned kind;} COMOBJ;
typedef struct COMGROUP {void *identity;COMOBJ *canonical;ULONG refs;} COMGROUP;
static COMOBJ com_objects[COM_LIMIT];
static COMGROUP com_groups[COM_LIMIT];
static unsigned com_identity_fallback_logged;
static void **com_vtable(unsigned);
static void com_surface_ready(void *,const SDESC *,void *);
static void *com_surface_owner(void *);
static COMOBJ *com_find(void *p)
{
    unsigned i;for(i=0;i<COM_LIMIT;i++)if(p==&com_objects[i] && com_objects[i].inner)return &com_objects[i];return 0;
}
static void *com_unwrap(void *p){COMOBJ *o=com_find(p);return o?o->inner:p;}
static ULONG com_inner_release(void *p)
{
    backend_enter();ULONG n=((PFN_RELEASE)(*(void***)p)[2])(p);backend_leave();return n;
}
/* Consume exactly one backend-owned reference, including on failure. Cached
 * inner interface references protect the backend identity for the group life.
 * Backend Release's diagnostic return value never controls our own lifetime. */
static HRESULT com_wrap_owned_in_group(void *inner,unsigned kind,void **out,COMGROUP *known)
{
    void *identity=0;COMGROUP *g=known;COMOBJ *o=0;unsigned i;HRESULT hr;unsigned identity_owned=0;
    if(!out){if(inner)com_inner_release(inner);return E_POINTER;}*out=0;
    if(!inner)return E_FAIL;
    if(!com_vtable(kind)){com_inner_release(inner);return E_NOINTERFACE;}
    /* Repeated outputs of the same live interface need no identity probe. */
    for(i=0;i<COM_LIMIT;i++)if(com_objects[i].inner==inner && com_objects[i].kind==kind && (!known || com_objects[i].group==known)){
        ++com_objects[i].group->refs;*out=&com_objects[i];com_inner_release(inner);return S_OK;
    }
    /* QI explicitly relates interfaces of the same object. Trust that
     * relationship without requiring the backend to implement IUnknown.
     * Standalone outputs use canonical identity when available; an explicit
     * E_NOINTERFACE falls back to the still-owned interface pointer. This is
     * capability-based, never a backend-name/version exception. */
    if(known)identity=known->identity;
    else {
        backend_enter();hr=((PFN_QUERY)(*(void***)inner)[0])(inner,&com_iunknown,&identity);backend_leave();
        if(hr==E_NOINTERFACE){identity=inner;if(!com_identity_fallback_logged++){append_log("COM boundary: IUnknown unavailable; retained interface identity");}}
        else if(hr<0 || !identity){com_inner_release(inner);return hr<0?hr:E_NOINTERFACE;}
        else identity_owned=1;
        for(i=0;i<COM_LIMIT;i++)if(com_groups[i].identity==identity && com_groups[i].refs){g=&com_groups[i];break;}
    }
    if(g)for(i=0;i<COM_LIMIT;i++)if(com_objects[i].group==g && com_objects[i].kind==kind){
        ++g->refs;*out=&com_objects[i];if(identity_owned)com_inner_release(identity);com_inner_release(inner);return S_OK;
    }
    for(i=0;i<COM_LIMIT;i++)if(!com_objects[i].inner && !com_objects[i].group){o=&com_objects[i];break;}
    if(!g)for(i=0;i<COM_LIMIT;i++)if(!com_groups[i].identity){g=&com_groups[i];break;}
    if(!g || !o){if(identity_owned)com_inner_release(identity);com_inner_release(inner);return E_OUTOFMEMORY;}
    o->inner=inner;o->kind=kind;o->group=g;o->vtable=com_vtable(kind);
    if(!g->identity){g->identity=identity;g->canonical=o;g->refs=0;}
    ++g->refs;*out=o;if(identity_owned)com_inner_release(identity);return S_OK;
}
static HRESULT com_wrap_owned(void *inner,unsigned kind,void **out)
{return com_wrap_owned_in_group(inner,kind,out,0);}
static ULONG WINAPI com_addref(void *self){COMOBJ *o=com_find(self);return o?++o->group->refs:0;}
static ULONG WINAPI com_release(void *self)
{
    COMOBJ *o=com_find(self);COMGROUP *g;unsigned i;void *inner[COM_LIMIT];unsigned count=0;
    if(!o)return 0;g=o->group;if(--g->refs)return g->refs;
    /* Make retiring objects unreachable, but reserve their slots and group
     * through backend callbacks. Re-entrant allocation must not reuse them. */
    for(i=0;i<COM_LIMIT;i++)if(com_objects[i].group==g){
        inner[count++]=com_objects[i].inner;com_objects[i].inner=0;com_objects[i].vtable=0;
    }
    for(i=0;i<count;i++)com_inner_release(inner[i]);
    for(i=0;i<COM_LIMIT;i++)if(com_objects[i].group==g){com_objects[i].group=0;com_objects[i].kind=0;}
    g->identity=0;g->canonical=0;return 0;
}
static unsigned com_kind(const GUID *iid)
{
    unsigned v=draw_version(iid);if(v && v<=3)return v;
    if(guid_eq(iid,&com_surface_iid))return COM_SURFACE;
    if(guid_eq(iid,&com_palette_iid))return COM_PALETTE;
    if(guid_eq(iid,&com_clipper_iid))return COM_CLIPPER;
    return 0;
}
static HRESULT WINAPI com_query(void *self,const GUID *iid,void **out)
{
    COMOBJ *o=com_find(self);void *inner=0;unsigned kind,i;HRESULT hr;
    if(!out)return E_POINTER;*out=0;if(!o || !iid)return E_INVALIDARG;
    if(guid_eq(iid,&com_iunknown)){++o->group->refs;*out=o->group->canonical;return S_OK;}
    kind=com_kind(iid);
    if(!kind || ((o->kind<=3)!=(kind<=3)) || (o->kind>3 && kind!=o->kind))return E_NOINTERFACE;
    for(i=0;i<COM_LIMIT;i++)if(com_objects[i].group==o->group && com_objects[i].kind==kind){++o->group->refs;*out=&com_objects[i];return S_OK;}
    backend_enter();hr=((PFN_QUERY)(*(void***)o->inner)[0])(o->inner,iid,&inner);backend_leave();
    return hr<0?hr:com_wrap_owned_in_group(inner,kind,out,o->group);
}
/* Strongly typed forwarding below; no code generation at runtime or patching
 * of private backend functions. Every interface argument is explicitly unwrapped. */
static HRESULT WINAPI com_compact(void *self)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *))(*(void***)o->inner)[3])(o->inner);backend_leave();return hr;
}
static HRESULT WINAPI com_enum_modes(void *self, DWORD flags, SDESC *desc, void *ctx, void *cb)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, SDESC *, void *, void *))(*(void***)o->inner)[8])(o->inner, flags, desc, ctx, cb);backend_leave();return hr;
}
static HRESULT WINAPI com_flip_gdi(void *self)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *))(*(void***)o->inner)[10])(o->inner);backend_leave();return hr;
}
static HRESULT WINAPI com_caps(void *self, void *driver, void *hel)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *, void *))(*(void***)o->inner)[11])(o->inner, driver, hel);backend_leave();return hr;
}
static HRESULT WINAPI com_mode(void *self, SDESC *desc)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, SDESC *))(*(void***)o->inner)[12])(o->inner, desc);backend_leave();
    if(hr>=0 && desc && desc->dwSize==108){desc->dwFlags|=0x40000;desc->mip=60;}
    return hr;
}
static HRESULT WINAPI com_fourcc(void *self, DWORD *count, DWORD *codes)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD *, DWORD *))(*(void***)o->inner)[13])(o->inner, count, codes);backend_leave();return hr;
}
#include "logical_display.h"
static HRESULT WINAPI com_frequency(void *self, DWORD *out)
{
    if(!com_find(self))return E_FAIL;if(!out)return E_INVALIDARG;*out=LOGICAL_HZ;return S_OK;
}
static HRESULT WINAPI com_scanline(void *self, DWORD *out)
{
    if(!com_find(self))return E_FAIL;if(!out)return E_INVALIDARG;
    DWORD phase=logical_phase();*out=0;
    if(phase>=LOGICAL_BLANK)return DDERR_VERTICALBLANKINPROGRESS;
    *out=phase*480/LOGICAL_BLANK;return S_OK;
}
static HRESULT WINAPI com_vblank(void *self, BOOL *out)
{
    if(!com_find(self))return E_FAIL;if(!out)return E_INVALIDARG;*out=logical_phase()>=LOGICAL_BLANK;return S_OK;
}
static HRESULT WINAPI com_init(void *self, GUID *guid)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, GUID *))(*(void***)o->inner)[18])(o->inner, guid);backend_leave();return hr;
}
static HRESULT WINAPI com_restore_mode(void *self)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *))(*(void***)o->inner)[19])(o->inner);backend_leave();return hr;
}
static HRESULT WINAPI com_coop(void *self, HWND window, DWORD flags)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, HWND, DWORD))(*(void***)o->inner)[20])(o->inner, window, flags);backend_leave();return hr;
}
static HRESULT WINAPI com_mode1(void *self, DWORD w, DWORD h, DWORD bpp)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, DWORD, DWORD))(*(void***)o->inner)[21])(o->inner, w, h, bpp);backend_leave();return hr;
}
static HRESULT WINAPI com_wait(void *self, DWORD flags, HANDLE event)
{
    (void)event;if(!com_find(self))return E_FAIL;
    if(flags==2)return E_NOTIMPL; /* BLOCKBEGINEVENT is unsupported by DirectDraw. */
    if(flags!=1 && flags!=4)return E_INVALIDARG;
    /* A delayed scheduler may wake past blank; never spin until a narrow phase. */
    Sleep(logical_wait_ms(logical_phase(),flags));return S_OK;
}
static HRESULT WINAPI com_mode2(void *self, DWORD w, DWORD h, DWORD bpp, DWORD hz, DWORD flags)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, DWORD, DWORD, DWORD, DWORD))(*(void***)o->inner)[21])(o->inner, w, h, bpp, hz, flags);backend_leave();return hr;
}
static HRESULT WINAPI com_memory(void *self, void *caps, DWORD *total, DWORD *freebytes)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *, DWORD *, DWORD *))(*(void***)o->inner)[23])(o->inner, caps, total, freebytes);backend_leave();return hr;
}
static HRESULT WINAPI com_attach(void *self, void *surface)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *))(*(void***)o->inner)[3])(o->inner, com_unwrap(surface));backend_leave();return hr;
}
static HRESULT WINAPI com_overlay_dirty(void *self, RECT *rect)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, RECT *))(*(void***)o->inner)[4])(o->inner, rect);backend_leave();return hr;
}
static HRESULT WINAPI com_blt_fast(void *self, DWORD x, DWORD y, void *src, RECT *rect, DWORD flags)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, DWORD, void *, RECT *, DWORD))(*(void***)o->inner)[7])(o->inner, x, y, com_unwrap(src), rect, flags);backend_leave();return hr;
}
static HRESULT WINAPI com_detach(void *self, DWORD flags, void *surface)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, void *))(*(void***)o->inner)[8])(o->inner, flags, com_unwrap(surface));backend_leave();return hr;
}
static HRESULT WINAPI com_flip(void *self, void *target, DWORD flags)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *, DWORD))(*(void***)o->inner)[11])(o->inner, com_unwrap(target), flags);backend_leave();return hr;
}
static HRESULT WINAPI com_blt_status(void *self, DWORD flags)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD))(*(void***)o->inner)[13])(o->inner, flags);backend_leave();return hr;
}
static HRESULT WINAPI com_surface_caps(void *self, void *caps)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *))(*(void***)o->inner)[14])(o->inner, caps);backend_leave();return hr;
}
static HRESULT WINAPI com_color_key(void *self, DWORD flags, void *key)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, void *))(*(void***)o->inner)[16])(o->inner, flags, key);backend_leave();return hr;
}
static HRESULT WINAPI com_get_dc(void *self, void **dc)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void **))(*(void***)o->inner)[17])(o->inner, dc);backend_leave();return hr;
}
static HRESULT WINAPI com_flip_status(void *self, DWORD flags)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD))(*(void***)o->inner)[18])(o->inner, flags);backend_leave();return hr;
}
static HRESULT WINAPI com_overlay_pos(void *self, long *x, long *y)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, long *, long *))(*(void***)o->inner)[19])(o->inner, x, y);backend_leave();return hr;
}
static HRESULT WINAPI com_pixel_format(void *self, void *format)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *))(*(void***)o->inner)[21])(o->inner, format);backend_leave();return hr;
}
static HRESULT WINAPI com_surface_desc(void *self, SDESC *desc)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, SDESC *))(*(void***)o->inner)[22])(o->inner, desc);backend_leave();return hr;
}
static HRESULT WINAPI com_surface_init(void *self, void *draw, SDESC *desc)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *, SDESC *))(*(void***)o->inner)[23])(o->inner, com_unwrap(draw), desc);backend_leave();return hr;
}
static HRESULT WINAPI com_lost(void *self)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *))(*(void***)o->inner)[24])(o->inner);backend_leave();return hr;
}
static HRESULT WINAPI com_lock(void *self, RECT *rect, SDESC *desc, DWORD flags, HANDLE event)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, RECT *, SDESC *, DWORD, HANDLE))(*(void***)o->inner)[25])(o->inner, rect, desc, flags, event);backend_leave();return hr;
}
static HRESULT WINAPI com_release_dc(void *self, void *dc)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *))(*(void***)o->inner)[26])(o->inner, dc);backend_leave();return hr;
}
static HRESULT WINAPI com_restore(void *self)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *))(*(void***)o->inner)[27])(o->inner);backend_leave();return hr;
}
static HRESULT WINAPI com_set_clipper(void *self, void *clipper)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *))(*(void***)o->inner)[28])(o->inner, com_unwrap(clipper));backend_leave();return hr;
}
static HRESULT WINAPI com_set_key(void *self, DWORD flags, void *key)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, void *))(*(void***)o->inner)[29])(o->inner, flags, key);backend_leave();return hr;
}
static HRESULT WINAPI com_set_overlay_pos(void *self, long x, long y)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, long, long))(*(void***)o->inner)[30])(o->inner, x, y);backend_leave();return hr;
}
static HRESULT WINAPI com_set_palette(void *self, void *palette)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *))(*(void***)o->inner)[31])(o->inner, com_unwrap(palette));backend_leave();return hr;
}
static HRESULT WINAPI com_unlock(void *self, void *pixels)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *))(*(void***)o->inner)[32])(o->inner, pixels);backend_leave();return hr;
}
static HRESULT WINAPI com_overlay_display(void *self, DWORD flags)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD))(*(void***)o->inner)[34])(o->inner, flags);backend_leave();return hr;
}
static HRESULT WINAPI com_overlay_order(void *self, DWORD flags, void *surface)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, void *))(*(void***)o->inner)[35])(o->inner, flags, com_unwrap(surface));backend_leave();return hr;
}
static HRESULT WINAPI com_palette_caps(void *self, DWORD *caps)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD *))(*(void***)o->inner)[3])(o->inner, caps);backend_leave();return hr;
}
static HRESULT WINAPI com_entries(void *self, DWORD flags, DWORD start, DWORD count, void *entries)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, DWORD, DWORD, void *))(*(void***)o->inner)[4])(o->inner, flags, start, count, entries);backend_leave();return hr;
}
static HRESULT WINAPI com_palette_init(void *self, void *draw, DWORD flags, void *entries)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *, DWORD, void *))(*(void***)o->inner)[5])(o->inner, com_unwrap(draw), flags, entries);backend_leave();return hr;
}
static HRESULT WINAPI com_set_entries(void *self, DWORD flags, DWORD start, DWORD count, void *entries)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, DWORD, DWORD, void *))(*(void***)o->inner)[6])(o->inner, flags, start, count, entries);backend_leave();return hr;
}
static HRESULT WINAPI com_clip_list(void *self, RECT *rect, void *data, DWORD *size)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, RECT *, void *, DWORD *))(*(void***)o->inner)[3])(o->inner, rect, data, size);backend_leave();return hr;
}
static HRESULT WINAPI com_clip_hwnd(void *self, HWND *window)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, HWND *))(*(void***)o->inner)[4])(o->inner, window);backend_leave();return hr;
}
static HRESULT WINAPI com_clip_init(void *self, void *draw, DWORD flags)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *, DWORD))(*(void***)o->inner)[5])(o->inner, com_unwrap(draw), flags);backend_leave();return hr;
}
static HRESULT WINAPI com_clip_changed(void *self, BOOL *changed)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, BOOL *))(*(void***)o->inner)[6])(o->inner, changed);backend_leave();return hr;
}
static HRESULT WINAPI com_set_clip(void *self, void *data, DWORD flags)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *, DWORD))(*(void***)o->inner)[7])(o->inner, data, flags);backend_leave();return hr;
}
static HRESULT WINAPI com_set_hwnd(void *self, DWORD flags, HWND window)
{
    COMOBJ *o=com_find(self);HRESULT hr;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, HWND))(*(void***)o->inner)[8])(o->inner, flags, window);backend_leave();return hr;
}
static HRESULT WINAPI com_create_clipper(void *self, DWORD flags, void **out, void *outer)
{
    COMOBJ *o=com_find(self);void *inner=0;HRESULT hr,wrapped;
    if(!out)return E_POINTER;*out=0;if(!o)return E_FAIL;
    if(outer)return CLASS_E_NOAGGREGATION;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, void **, void *))(*(void***)o->inner)[4])(o->inner, flags, &inner, outer);backend_leave();
    if(hr<0)return hr;wrapped=com_wrap_owned(inner,COM_CLIPPER,out);if(wrapped<0)return wrapped;
    return hr;
}
static HRESULT WINAPI com_create_palette(void *self, DWORD flags, void *entries, void **out, void *outer)
{
    COMOBJ *o=com_find(self);void *inner=0;HRESULT hr,wrapped;
    if(!out)return E_POINTER;*out=0;if(!o)return E_FAIL;
    if(outer)return CLASS_E_NOAGGREGATION;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, DWORD, void *, void **, void *))(*(void***)o->inner)[5])(o->inner, flags, entries, &inner, outer);backend_leave();
    if(hr<0)return hr;wrapped=com_wrap_owned(inner,COM_PALETTE,out);if(wrapped<0)return wrapped;
    return hr;
}
static HRESULT WINAPI com_create_surface(void *self, SDESC *desc, void **out, void *outer)
{
    COMOBJ *o=com_find(self);void *inner=0;HRESULT hr,wrapped;
    if(!out)return E_POINTER;*out=0;if(!o)return E_FAIL;
    if(outer)return CLASS_E_NOAGGREGATION;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, SDESC *, void **, void *))(*(void***)o->inner)[6])(o->inner, desc, &inner, outer);backend_leave();
    if(hr<0)return hr;wrapped=com_wrap_owned(inner,COM_SURFACE,out);if(wrapped<0)return wrapped;
    return hr;
}
static HRESULT WINAPI com_duplicate_surface(void *self, void *source, void **out)
{
    COMOBJ *o=com_find(self);void *inner=0;HRESULT hr,wrapped;
    if(!out)return E_POINTER;*out=0;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *, void **))(*(void***)o->inner)[7])(o->inner, com_unwrap(source), &inner);backend_leave();
    if(hr<0)return hr;wrapped=com_wrap_owned(inner,COM_SURFACE,out);if(wrapped<0)return wrapped;
    com_surface_ready(*out,0,self);
    return hr;
}
static HRESULT WINAPI com_gdi_surface(void *self, void **out)
{
    COMOBJ *o=com_find(self);void *inner=0;HRESULT hr,wrapped;
    if(!out)return E_POINTER;*out=0;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void **))(*(void***)o->inner)[14])(o->inner, &inner);backend_leave();
    if(hr<0)return hr;wrapped=com_wrap_owned(inner,COM_SURFACE,out);if(wrapped<0)return wrapped;
    com_surface_ready(*out,0,self);
    return hr;
}
static HRESULT WINAPI com_surface_dc(void *self, void *dc, void **out)
{
    COMOBJ *o=com_find(self);void *inner=0;HRESULT hr,wrapped;
    if(!out)return E_POINTER;*out=0;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *, void **))(*(void***)o->inner)[24])(o->inner, dc, &inner);backend_leave();
    if(hr<0)return hr;wrapped=com_wrap_owned(inner,COM_SURFACE,out);if(wrapped<0)return wrapped;
    com_surface_ready(*out,0,self);
    return hr;
}
static HRESULT WINAPI com_get_attached(void *self, void *caps, void **out)
{
    COMOBJ *o=com_find(self);void *inner=0;HRESULT hr,wrapped;
    if(!out)return E_POINTER;*out=0;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void *, void **))(*(void***)o->inner)[12])(o->inner, caps, &inner);backend_leave();
    if(hr<0)return hr;wrapped=com_wrap_owned(inner,COM_SURFACE,out);if(wrapped<0)return wrapped;
    return hr;
}
static HRESULT WINAPI com_get_clipper(void *self, void **out)
{
    COMOBJ *o=com_find(self);void *inner=0;HRESULT hr,wrapped;
    if(!out)return E_POINTER;*out=0;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void **))(*(void***)o->inner)[15])(o->inner, &inner);backend_leave();
    if(hr<0)return hr;wrapped=com_wrap_owned(inner,COM_CLIPPER,out);if(wrapped<0)return wrapped;
    return hr;
}
static HRESULT WINAPI com_get_palette(void *self, void **out)
{
    COMOBJ *o=com_find(self);void *inner=0;HRESULT hr,wrapped;
    if(!out)return E_POINTER;*out=0;if(!o)return E_FAIL;
    backend_enter();hr=((HRESULT(WINAPI*)(void *, void **))(*(void***)o->inner)[20])(o->inner, &inner);backend_leave();
    if(hr<0)return hr;wrapped=com_wrap_owned(inner,COM_PALETTE,out);if(wrapped<0)return wrapped;
    return hr;
}
/* Surface-valued unions must be copied and unwrapped; color values and the
 * caller's structures are left intact. Layout is the public 32-bit DX ABI. */
typedef struct {DWORD word[25];} COMBLTFX;
typedef struct {RECT *dst;void *src;RECT *rect;DWORD flags;COMBLTFX *fx;} COMBATCH;
_Static_assert(sizeof(COMBLTFX)==100 && sizeof(COMBATCH)==20,"DirectDraw x86 ABI");
static HRESULT com_copy_bltfx(COMBLTFX *dst,const COMBLTFX *src,DWORD flags)
{
    unsigned i;if(!src)return S_OK;if(src->word[0]!=100)return E_INVALIDARG;
    /* DDROPS is not exercised by this game; do not guess its union semantics. */
    if(flags&0x1000)return E_NOTIMPL;
    for(i=0;i<25;i++)dst->word[i]=src->word[i];
    if(flags&0x00200000)dst->word[10]=(DWORD)com_unwrap((void*)dst->word[10]);
    if(flags&0x00800000)dst->word[12]=(DWORD)com_unwrap((void*)dst->word[12]);
    if(flags&0x8)dst->word[17]=(DWORD)com_unwrap((void*)dst->word[17]);
    if(flags&0x100)dst->word[19]=(DWORD)com_unwrap((void*)dst->word[19]);
    if(flags&0x20000){DWORD rop=(dst->word[2]>>16)&255;if(((rop>>4)^rop)&15)dst->word[20]=(DWORD)com_unwrap((void*)dst->word[20]);}
    return S_OK;
}
static HRESULT WINAPI com_blt(void *self,RECT *dst,void *src,RECT *rect,DWORD flags,void *fx)
{
    COMOBJ *o=com_find(self);COMBLTFX copy;HRESULT hr;if(!o)return E_FAIL;
    hr=com_copy_bltfx(&copy,fx,flags);if(hr<0)return hr;
    backend_enter();hr=((PFN_SURF_BLT)(*(void***)o->inner)[5])(o->inner,dst,com_unwrap(src),rect,flags,fx?&copy:0);backend_leave();return hr;
}
static HRESULT WINAPI com_blt_batch(void *self,COMBATCH *batch,DWORD count,DWORD flags)
{
    COMOBJ *o=com_find(self);COMBATCH *copy;COMBLTFX *effects;DWORD i;HRESULT hr=S_OK;
    if(!o)return E_FAIL;if((count && !batch) || count>0xfffffffful/(sizeof(COMBATCH)+sizeof(COMBLTFX)))return E_INVALIDARG;
    copy=HeapAlloc(GetProcessHeap(),0,(count?count:1)*(sizeof(COMBATCH)+sizeof(COMBLTFX)));if(!copy)return E_OUTOFMEMORY;
    effects=(COMBLTFX*)(copy+count);
    for(i=0;i<count;i++){
        copy[i].dst=batch[i].dst;copy[i].src=com_unwrap(batch[i].src);copy[i].rect=batch[i].rect;copy[i].flags=batch[i].flags;
        copy[i].fx=batch[i].fx?&effects[i]:0;hr=com_copy_bltfx(&effects[i],batch[i].fx,batch[i].flags);if(hr<0)break;
    }
    if(hr>=0){backend_enter();hr=((HRESULT(WINAPI*)(void*,COMBATCH*,DWORD,DWORD))(*(void***)o->inner)[6])(o->inner,copy,count,flags);backend_leave();}
    HeapFree(GetProcessHeap(),0,copy);return hr;
}
static HRESULT WINAPI com_update_overlay(void *self,RECT *src,void *dest,RECT *dst,DWORD flags,void *fx)
{
    COMOBJ *o=com_find(self);DWORD copy[14];unsigned i;HRESULT hr;if(!o)return E_FAIL;
    if(fx){if(*(DWORD*)fx!=sizeof(copy))return E_INVALIDARG;for(i=0;i<14;i++)copy[i]=((DWORD*)fx)[i];
        if(flags&8)copy[5]=(DWORD)com_unwrap((void*)copy[5]);if(flags&0x100)copy[7]=(DWORD)com_unwrap((void*)copy[7]);}
    backend_enter();hr=((HRESULT(WINAPI*)(void*,RECT*,void*,RECT*,DWORD,void*))(*(void***)o->inner)[33])(o->inner,src,com_unwrap(dest),dst,flags,fx?copy:0);backend_leave();return hr;
}
typedef HRESULT(WINAPI *COMENUM)(void*,SDESC*,void*);
typedef struct {COMENUM callback;void *context;void *owner;HRESULT error;} COMENUMCTX;
/* EnumSurfaces(DOESEXIST) and EnumAttachedSurfaces hand one owned reference
 * to each callback. Transfer it to the outer object; the application releases
 * it. A failing adapter consumes it and cancels the enumeration. */
static HRESULT WINAPI com_enum_callback(void *inner,SDESC *desc,void *context)
{
    COMENUMCTX *ctx=context;void *surface=0;
    if(inner){ctx->error=com_wrap_owned(inner,COM_SURFACE,&surface);if(ctx->error<0)return 0;
        com_surface_ready(surface,desc,ctx->owner);}
    return ctx->callback(surface,desc,ctx->context);
}
static HRESULT WINAPI com_enum_surfaces(void *self,DWORD flags,SDESC *desc,void *context,COMENUM callback)
{
    COMOBJ *o=com_find(self);COMENUMCTX ctx={callback,context,self,S_OK};HRESULT hr;
    if(!o || !callback)return E_INVALIDARG;
    if(flags&1)return E_NOTIMPL; /* CANBECREATED temporary-surface lifetime not implemented. */
    backend_enter();hr=((HRESULT(WINAPI*)(void*,DWORD,SDESC*,void*,COMENUM))(*(void***)o->inner)[9])(o->inner,flags,desc,&ctx,com_enum_callback);backend_leave();
    return ctx.error<0?ctx.error:hr;
}
static HRESULT WINAPI com_enum_attached(void *self,void *context,COMENUM callback)
{
    COMOBJ *o=com_find(self);COMENUMCTX ctx={callback,context,com_surface_owner(self),S_OK};HRESULT hr;
    if(!o || !callback)return E_INVALIDARG;
    backend_enter();hr=((HRESULT(WINAPI*)(void*,void*,COMENUM))(*(void***)o->inner)[9])(o->inner,&ctx,com_enum_callback);backend_leave();
    return ctx.error<0?ctx.error:hr;
}
static HRESULT WINAPI com_enum_overlays(void *self,DWORD flags,void *context,COMENUM callback)
{
    /* Not used by Generations. Do not pass through a raw callback interface:
     * overlay reference/version semantics have not been qualified here. */
    (void)self;(void)flags;(void)context;(void)callback;return E_NOTIMPL;
}

static void **com_vtable(unsigned kind)
{
    static void *d1[]={(void*)com_query,(void*)com_addref,(void*)com_release,(void*)com_compact,(void*)com_create_clipper,(void*)com_create_palette,(void*)com_create_surface,(void*)com_duplicate_surface,(void*)com_enum_modes,(void*)com_enum_surfaces,(void*)com_flip_gdi,(void*)com_caps,(void*)com_mode,(void*)com_fourcc,(void*)com_gdi_surface,(void*)com_frequency,(void*)com_scanline,(void*)com_vblank,(void*)com_init,(void*)com_restore_mode,(void*)com_coop,(void*)com_mode1,(void*)com_wait};
    static void *d2[]={(void*)com_query,(void*)com_addref,(void*)com_release,(void*)com_compact,(void*)com_create_clipper,(void*)com_create_palette,(void*)com_create_surface,(void*)com_duplicate_surface,(void*)com_enum_modes,(void*)com_enum_surfaces,(void*)com_flip_gdi,(void*)com_caps,(void*)com_mode,(void*)com_fourcc,(void*)com_gdi_surface,(void*)com_frequency,(void*)com_scanline,(void*)com_vblank,(void*)com_init,(void*)com_restore_mode,(void*)com_coop,(void*)com_mode2,(void*)com_wait,(void*)com_memory};
    static void *d3[]={(void*)com_query,(void*)com_addref,(void*)com_release,(void*)com_compact,(void*)com_create_clipper,(void*)com_create_palette,(void*)com_create_surface,(void*)com_duplicate_surface,(void*)com_enum_modes,(void*)com_enum_surfaces,(void*)com_flip_gdi,(void*)com_caps,(void*)com_mode,(void*)com_fourcc,(void*)com_gdi_surface,(void*)com_frequency,(void*)com_scanline,(void*)com_vblank,(void*)com_init,(void*)com_restore_mode,(void*)com_coop,(void*)com_mode2,(void*)com_wait,(void*)com_memory,(void*)com_surface_dc};
    static void *s[]={(void*)com_query,(void*)com_addref,(void*)com_release,(void*)com_attach,(void*)com_overlay_dirty,(void*)com_blt,(void*)com_blt_batch,(void*)com_blt_fast,(void*)com_detach,(void*)com_enum_attached,(void*)com_enum_overlays,(void*)com_flip,(void*)com_get_attached,(void*)com_blt_status,(void*)com_surface_caps,(void*)com_get_clipper,(void*)com_color_key,(void*)com_get_dc,(void*)com_flip_status,(void*)com_overlay_pos,(void*)com_get_palette,(void*)com_pixel_format,(void*)com_surface_desc,(void*)com_surface_init,(void*)com_lost,(void*)com_lock,(void*)com_release_dc,(void*)com_restore,(void*)com_set_clipper,(void*)com_set_key,(void*)com_set_overlay_pos,(void*)com_set_palette,(void*)com_unlock,(void*)com_update_overlay,(void*)com_overlay_display,(void*)com_overlay_order};
    static void *p[]={(void*)com_query,(void*)com_addref,(void*)com_release,(void*)com_palette_caps,(void*)com_entries,(void*)com_palette_init,(void*)com_set_entries};
    static void *c[]={(void*)com_query,(void*)com_addref,(void*)com_release,(void*)com_clip_list,(void*)com_clip_hwnd,(void*)com_clip_init,(void*)com_clip_changed,(void*)com_set_clip,(void*)com_set_hwnd};
    switch(kind){case COM_DRAW1:return d1;case COM_DRAW2:return d2;case COM_DRAW3:return d3;case COM_SURFACE:return s;case COM_PALETTE:return p;case COM_CLIPPER:return c;default:return 0;}
}
