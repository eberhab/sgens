/* Optional final-image filtering. Copyright (c) 2026. MIT license.
 * Owns only the visible presenter's OpenGL context; no backend APIs/hooks.
 * Runs on the game/UI thread, after readback and logical GDI composition.
 * GLSL 1.20 bicubic interpolation uses the Catmull-Rom kernel (a = -0.5).
 * No external shader files, OpenGL import library, or C runtime is required. */
#ifndef SGENS_PRESENTATION_FILTER_H
#define SGENS_PRESENTATION_FILTER_H

typedef struct {
    unsigned short size,version;
    DWORD flags;
    BYTE type,color,red,red_shift,green,green_shift,blue,blue_shift,alpha,alpha_shift;
    BYTE accum,accum_red,accum_green,accum_blue,accum_alpha,depth,stencil,aux,layer,reserved;
    DWORD layer_mask,visible_mask,damage_mask;
} FILTER_PFD;
_Static_assert(sizeof(FILTER_PFD)==40,"PIXELFORMATDESCRIPTOR must be 40 bytes");
DLLIMPORT int WINAPI ChoosePixelFormat(void*,const FILTER_PFD*);
DLLIMPORT int WINAPI GetPixelFormat(void*);
DLLIMPORT BOOL WINAPI SetPixelFormat(void*,int,const FILTER_PFD*);
DLLIMPORT BOOL WINAPI SwapBuffers(void*);
DLLIMPORT BOOL WINAPI DestroyWindow(HWND);
DLLIMPORT DWORD WINAPI GetModuleFileNameA(HMODULE,char*,DWORD);
int _fltused; /* MSVC floating-point marker; no CRT code is needed. */

#define FILTER_FN(ret,name,args) static ret (WINAPI *f_##name) args
FILTER_FN(void*,wglCreateContext,(void*));
FILTER_FN(BOOL,wglDeleteContext,(void*));
FILTER_FN(BOOL,wglMakeCurrent,(void*,void*));
FILTER_FN(void*,wglGetCurrentContext,(void));
FILTER_FN(void*,wglGetCurrentDC,(void));
FILTER_FN(void*,wglGetProcAddress,(const char*));
FILTER_FN(unsigned,glCreateShader,(unsigned));
FILTER_FN(void,glShaderSource,(unsigned,int,const char* const*,const int*));
FILTER_FN(void,glCompileShader,(unsigned));
FILTER_FN(void,glGetShaderiv,(unsigned,unsigned,int*));
FILTER_FN(void,glGetShaderInfoLog,(unsigned,int,int*,char*));
FILTER_FN(void,glDeleteShader,(unsigned));
FILTER_FN(unsigned,glCreateProgram,(void));
FILTER_FN(void,glAttachShader,(unsigned,unsigned));
FILTER_FN(void,glLinkProgram,(unsigned));
FILTER_FN(void,glGetProgramiv,(unsigned,unsigned,int*));
FILTER_FN(void,glGetProgramInfoLog,(unsigned,int,int*,char*));
FILTER_FN(void,glDeleteProgram,(unsigned));
FILTER_FN(void,glUseProgram,(unsigned));
FILTER_FN(int,glGetUniformLocation,(unsigned,const char*));
FILTER_FN(void,glUniform1i,(int,int));
FILTER_FN(void,glUniform2f,(int,float,float));
FILTER_FN(void,glGenTextures,(int,unsigned*));
FILTER_FN(void,glDeleteTextures,(int,const unsigned*));
FILTER_FN(void,glBindTexture,(unsigned,unsigned));
FILTER_FN(void,glTexParameteri,(unsigned,unsigned,int));
FILTER_FN(void,glTexImage2D,(unsigned,int,int,int,int,int,unsigned,unsigned,const void*));
FILTER_FN(void,glTexSubImage2D,(unsigned,int,int,int,int,int,unsigned,unsigned,const void*));
FILTER_FN(void,glViewport,(int,int,int,int));
FILTER_FN(void,glDisable,(unsigned));
FILTER_FN(void,glBegin,(unsigned));
FILTER_FN(void,glEnd,(void));
FILTER_FN(void,glTexCoord2f,(float,float));
FILTER_FN(void,glVertex2f,(float,float));
FILTER_FN(unsigned,glGetError,(void));
#undef FILTER_FN

static HMODULE g_filter_module;
static HWND g_filter_hwnd;
static void *g_filter_dc,*g_filter_context;
static int g_filter_choice=-1,g_filter_failed,g_filter_busy,g_filter_reset;
static int g_filter_w,g_filter_h,g_filter_size_uniform;
static unsigned g_filter_program,g_filter_texture;
#define FILTER_RESET_MESSAGE (0x8000u+0x534u)

static int filter_equal(const char *a,const char *b)
{
    while(*a && *b){unsigned c=(BYTE)*a++;if(c>='A' && c<='Z')c+=32;
        if(c!=(BYTE)*b++)return 0;}
    return !*a && !*b;
}
static int filter_choice(void)
{
    if(g_filter_choice<0){
        /* Generations temporarily changes CWD to WORLD during screen creation. */
        char path[260]={0},value[32]={0};HMODULE module=0;unsigned cut=0;
        g_filter_choice=0;
        if(!GetModuleHandleExA(6,(LPCSTR)(void*)filter_choice,&module))return 0;
        DWORD len=GetModuleFileNameA(module,path,sizeof(path));
        if(!len || len>=sizeof(path))return 0;
        for(unsigned i=0;i<len;i++)if(path[i]=='\\' || path[i]=='/')cut=i+1;
        if(!cut || cut+10>sizeof(path))return 0;
        for(unsigned i=0;i<10;i++)path[cut+i]="ddraw.ini"[i];
        GetPrivateProfileStringA("sgens-shim","filter","none",value,sizeof(value),path);
        g_filter_choice=filter_equal(value,"bilinear")?1:filter_equal(value,"bicubic")?2:0;
        {char line[128];unsigned at=0;concat(line,&at,"Presentation filter configured: ");concat(line,&at,value);append_log(line);}
        if(!g_filter_choice && !filter_equal(value,"none"))
            append_log("Unknown [sgens-shim] filter; using unfiltered GDI");
    }
    return g_filter_choice;
}
static void *filter_proc(const char *name)
{
    void *p=GetProcAddress(g_filter_module,name);
    if(!p && f_wglGetProcAddress)p=f_wglGetProcAddress(name);
    /* WGL can return these sentinel values for unsupported entry points. */
    if((DWORD)p<=3 || (DWORD)p==0xfffffffful)return 0;
    return p;
}
#define FILTER_LOAD(name) do {f_##name=(void*)filter_proc(#name);if(!f_##name)return 0;} while(0)
static int filter_load_wgl(void)
{
    if(!g_filter_module)g_filter_module=LoadLibraryA("opengl32.dll");
    if(!g_filter_module)return 0;
    FILTER_LOAD(wglCreateContext);FILTER_LOAD(wglDeleteContext);
    FILTER_LOAD(wglMakeCurrent);FILTER_LOAD(wglGetCurrentContext);
    FILTER_LOAD(wglGetCurrentDC);FILTER_LOAD(wglGetProcAddress);
    return 1;
}
static int filter_load_gl(void)
{
    FILTER_LOAD(glCreateShader);FILTER_LOAD(glShaderSource);FILTER_LOAD(glCompileShader);
    FILTER_LOAD(glGetShaderiv);FILTER_LOAD(glGetShaderInfoLog);FILTER_LOAD(glDeleteShader);
    FILTER_LOAD(glCreateProgram);FILTER_LOAD(glAttachShader);FILTER_LOAD(glLinkProgram);
    FILTER_LOAD(glGetProgramiv);FILTER_LOAD(glGetProgramInfoLog);FILTER_LOAD(glDeleteProgram);
    FILTER_LOAD(glUseProgram);FILTER_LOAD(glGetUniformLocation);FILTER_LOAD(glUniform1i);
    FILTER_LOAD(glUniform2f);FILTER_LOAD(glGenTextures);FILTER_LOAD(glDeleteTextures);
    FILTER_LOAD(glBindTexture);FILTER_LOAD(glTexParameteri);FILTER_LOAD(glTexImage2D);
    FILTER_LOAD(glTexSubImage2D);FILTER_LOAD(glViewport);FILTER_LOAD(glDisable);
    FILTER_LOAD(glBegin);FILTER_LOAD(glEnd);FILTER_LOAD(glTexCoord2f);
    FILTER_LOAD(glVertex2f);FILTER_LOAD(glGetError);
    return 1;
}
#undef FILTER_LOAD

static const char filter_vertex[]=
    "#version 120\n"
    "varying vec2 uv; void main(){gl_Position=gl_Vertex;uv=gl_MultiTexCoord0.xy;}\n";
static const char filter_linear[]=
    "#version 120\n"
    "uniform sampler2D image; varying vec2 uv;"
    "void main(){gl_FragColor=vec4(texture2D(image,uv).rgb,1.0);}\n";
static const char filter_cubic[]=
    "#version 120\n"
    "uniform sampler2D image; uniform vec2 size; varying vec2 uv;"
    "float weight(float x){x=abs(x);"
    " if(x<=1.0)return (1.5*x-2.5)*x*x+1.0;"
    " if(x<2.0)return ((-0.5*x+2.5)*x-4.0)*x+2.0; return 0.0;}"
    "void main(){vec2 p=uv*size-0.5;vec2 b=floor(p);vec2 f=p-b;vec3 c=vec3(0.0);"
    " for(int y=-1;y<=2;y++)for(int x=-1;x<=2;x++){vec2 d=vec2(float(x),float(y));"
    " vec2 q=(clamp(b+d,vec2(0.0),size-1.0)+0.5)/size;"
    " c+=texture2D(image,q).rgb*weight(d.x-f.x)*weight(d.y-f.y);}"
    " gl_FragColor=vec4(clamp(c,0.0,1.0),1.0);}\n";

static unsigned filter_compile(unsigned type,const char *source)
{
    unsigned shader=f_glCreateShader(type);int ok=0;
    if(!shader)return 0;
    f_glShaderSource(shader,1,&source,0);f_glCompileShader(shader);
    f_glGetShaderiv(shader,0x8b81,&ok); /* GL_COMPILE_STATUS */
    if(!ok){char log[256]={0};f_glGetShaderInfoLog(shader,255,0,log);append_log(log);
        f_glDeleteShader(shader);return 0;}
    return shader;
}
static int filter_build(void)
{
    unsigned v=filter_compile(0x8b31,filter_vertex);
    unsigned f=filter_compile(0x8b30,filter_choice()==2?filter_cubic:filter_linear);
    if(!v || !f){if(v)f_glDeleteShader(v);if(f)f_glDeleteShader(f);return 0;}
    g_filter_program=f_glCreateProgram();int ok=0;
    if(g_filter_program){f_glAttachShader(g_filter_program,v);f_glAttachShader(g_filter_program,f);
        f_glLinkProgram(g_filter_program);f_glGetProgramiv(g_filter_program,0x8b82,&ok);}
    f_glDeleteShader(v);f_glDeleteShader(f);
    if(!ok){char log[256]={0};if(g_filter_program)f_glGetProgramInfoLog(g_filter_program,255,0,log);
        append_log(log);return 0;}
    f_glUseProgram(g_filter_program);
    f_glUniform1i(f_glGetUniformLocation(g_filter_program,"image"),0);
    g_filter_size_uniform=f_glGetUniformLocation(g_filter_program,"size");
    f_glGenTextures(1,&g_filter_texture);f_glBindTexture(0x0de1,g_filter_texture);
    int interpolation=filter_choice()==1?0x2601:0x2600; /* LINEAR / NEAREST */
    f_glTexParameteri(0x0de1,0x2801,interpolation);f_glTexParameteri(0x0de1,0x2800,interpolation);
    f_glTexParameteri(0x0de1,0x2802,0x812f);f_glTexParameteri(0x0de1,0x2803,0x812f); /* CLAMP_TO_EDGE */
    f_glDisable(0x0bd0); /* GL_DITHER: preserve the composed frame's colours. */
    return g_filter_texture && f_glGetError()==0;
}
static void filter_destroy(HWND hwnd)
{
    if(hwnd!=g_filter_hwnd)return;
    if(g_filter_context){
        void *old=f_wglGetCurrentContext(),*dc=f_wglGetCurrentDC();
        if(f_wglMakeCurrent(g_filter_dc,g_filter_context)){
            if(g_filter_texture && f_glDeleteTextures)f_glDeleteTextures(1,&g_filter_texture);
            if(g_filter_program && f_glDeleteProgram)f_glDeleteProgram(g_filter_program);
        }
        f_wglMakeCurrent(old==g_filter_context?0:dc,old==g_filter_context?0:old);
        f_wglDeleteContext(g_filter_context);
    }
    if(g_filter_dc)ReleaseDC(hwnd,g_filter_dc);
    g_filter_context=g_filter_dc=0;g_filter_hwnd=0;
    g_filter_program=g_filter_texture=0;g_filter_w=g_filter_h=0;g_filter_reset=0;
}
/* 1: presented, 0: use GDI, -1: recreate the child before using GDI.
 * Pixel formats are immutable. A failed GL child is replaced asynchronously
 * with a fresh GDI child, rather than mixing GDI with a double-buffered DC. */
static int filter_present(HWND hwnd,const BYTE *pixels,int w,int h,int outw,int outh)
{
    if(!filter_choice() || g_filter_failed)return 0;
    if(g_filter_busy)return 1; /* Defend against WM_PAINT reentry from a driver. */
    g_filter_busy=1;
    void *old=0,*olddc=0;int changed=0,ok=0;
    if(!filter_load_wgl())goto failed;
    old=f_wglGetCurrentContext();olddc=f_wglGetCurrentDC();
    if(!g_filter_context){
        g_filter_hwnd=hwnd;g_filter_dc=GetDCEx(hwnd,0,0x10); /* CS_OWNDC, CLIPSIBLINGS */
        if(!g_filter_dc)goto failed;
        FILTER_PFD p={0};p.size=sizeof(p);p.version=1;p.flags=0x25;p.color=24;
        int format=ChoosePixelFormat(g_filter_dc,&p);
        if(!format || GetPixelFormat(g_filter_dc) || !SetPixelFormat(g_filter_dc,format,&p))goto failed;
        g_filter_reset=1;
        g_filter_context=f_wglCreateContext(g_filter_dc);
        if(!g_filter_context || !f_wglMakeCurrent(g_filter_dc,g_filter_context))goto failed;
        changed=1;
        if(!filter_load_gl() || !filter_build())goto failed;
        append_log(filter_choice()==2?"Presentation filter active: bicubic (OpenGL)":"Presentation filter active: bilinear (OpenGL)");
    }else{
        if(!f_wglMakeCurrent(g_filter_dc,g_filter_context))goto failed;
        changed=1;
    }
    f_glViewport(0,0,outw,outh);
    if(w!=g_filter_w || h!=g_filter_h){
        f_glTexImage2D(0x0de1,0,0x8051,w,h,0,0x80e1,0x1401,pixels); /* RGB8 / BGRA / UBYTE */
        g_filter_w=w;g_filter_h=h;
        if(g_filter_size_uniform>=0)f_glUniform2f(g_filter_size_uniform,(float)w,(float)h);
    }else f_glTexSubImage2D(0x0de1,0,0,0,w,h,0x80e1,0x1401,pixels);
    f_glBegin(7); /* GL_QUADS; top-down readback, same stretch as the GDI path. */
    f_glTexCoord2f(0,0);f_glVertex2f(-1,1);f_glTexCoord2f(1,0);f_glVertex2f(1,1);
    f_glTexCoord2f(1,1);f_glVertex2f(1,-1);f_glTexCoord2f(0,1);f_glVertex2f(-1,-1);
    f_glEnd();
    ok=f_glGetError()==0 && SwapBuffers(g_filter_dc);
    if(!ok)goto failed;
    if(!f_wglMakeCurrent(olddc,old))goto failed;
    g_filter_busy=0;return 1;
failed:
    if(changed)f_wglMakeCurrent(olddc,old);
    g_filter_failed=1;g_filter_busy=0;
    append_log("Presentation filter unavailable; falling back to unfiltered GDI");
    return g_filter_reset?-1:0;
}
#endif
