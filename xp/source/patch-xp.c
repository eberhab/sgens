/* Star Trek Generations: native-XP or full-shim executable patch.
 * SPDX-License-Identifier: MIT
 * No C runtime or SDK headers; Windows XP x86 APIs only.
 * Input is read-only. Only the two complete, known original files are accepted.
 */
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short wchar;
typedef void *HANDLE;
#define API __declspec(dllimport) __stdcall
#define INVALID_HANDLE ((HANDLE)-1)
HANDLE API CreateFileW(const wchar *, u32, u32, void *, u32, u32, HANDLE);
u32 API GetFileSize(HANDLE, u32 *);
u32 API GetLastError(void);
int API ReadFile(HANDLE, void *, u32, u32 *, void *);
int API WriteFile(HANDLE, const void *, u32, u32 *, void *);
u32 API SetFilePointer(HANDLE, int, int *, u32);
int API FlushFileBuffers(HANDLE);
int API CloseHandle(HANDLE);
int API DeleteFileW(const wchar *);
HANDLE API GetStdHandle(u32);
__declspec(dllimport) wchar *__stdcall GetCommandLineW(void);
__declspec(dllimport) wchar **__stdcall CommandLineToArgvW(const wchar *, int *);
HANDLE API LocalFree(HANDLE);
void API ExitProcess(u32);

static u8 file[1571328];

/* SHA-256 is embedded so hashing needs no certutil, CryptoAPI provider,
 * .NET, PowerShell, Python, or downloaded runtime on XP. */
static const u32 constants[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};
static u32 rotate(u32 x, u32 n) { return (x >> n) | (x << (32 - n)); }
static void block(u32 h[8], const u8 *p) {
    u32 w[64], a=h[0], b=h[1], c=h[2], d=h[3], e=h[4], f=h[5], g=h[6], z=h[7];
    u32 i, t1, t2, s0, s1;
    for (i=0; i<16; ++i)
        w[i]=((u32)p[i*4]<<24)|((u32)p[i*4+1]<<16)|((u32)p[i*4+2]<<8)|p[i*4+3];
    for (i=16; i<64; ++i) {
        s0=rotate(w[i-15],7)^rotate(w[i-15],18)^(w[i-15]>>3);
        s1=rotate(w[i-2],17)^rotate(w[i-2],19)^(w[i-2]>>10);
        w[i]=w[i-16]+s0+w[i-7]+s1;
    }
    for (i=0; i<64; ++i) {
        s1=rotate(e,6)^rotate(e,11)^rotate(e,25);
        t1=z+s1+((e&f)^((~e)&g))+constants[i]+w[i];
        s0=rotate(a,2)^rotate(a,13)^rotate(a,22);
        t2=s0+((a&b)^(a&c)^(b&c));
        z=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=z;
}
static void sha256(const u8 *p, u32 length, char hex[65]) {
    u32 h[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    u8 tail[128];
    u32 full=length&~63u, left=length-full, pad=(left<56 ? 64 : 128), i, v;
    static const char digits[]="0123456789abcdef";
    for (i=0; i<full; i+=64) block(h,p+i);
    for (i=0; i<pad; ++i) tail[i]=0;
    for (i=0; i<left; ++i) tail[i]=p[full+i];
    tail[left]=0x80;
    /* All accepted inputs are below 2 MiB: the high length word is zero. */
    for (i=0; i<4; ++i) tail[pad-1-i]=(u8)((length*8)>>(i*8));
    block(h,tail);
    if (pad==128) block(h,tail+64);
    for (i=0; i<32; ++i) {
        v=(h[i/4]>>((3-i%4)*8))&255;
        hex[i*2]=digits[v>>4]; hex[i*2+1]=digits[v&15];
    }
    hex[64]=0;
}
static int equal(const char *a, const char *b) {
    while (*a && *a==*b) { ++a; ++b; }
    return *a==*b;
}
static int option(const wchar *wide, const char *ascii) {
    while (*wide && *wide==(u8)*ascii) { ++wide; ++ascii; }
    return *wide==0 && *ascii==0;
}
static void say(const char *s) {
    u32 length=0, written;
    while (s[length]) ++length;
    WriteFile(GetStdHandle((u32)-11),s,length,&written,0);
}

/* Inspect a DLL as data, never LoadLibrary: another vendor's ddraw.dll must
 * not enable the Generations-shim choice. The three null-terminated project
 * markers identify the shim independently of its build hash/version. This is
 * a format/identity check, not cryptographic authentication of a DLL. */
static int contains(u32 size, const char *text) {
    u32 n=0,i,j;
    while (text[n]) ++n;
    ++n; /* Include the terminating zero, not just a substring. */
    if (size<n) return 0;
    for (i=0; i<=size-n; ++i) {
        for (j=0; j<n && file[i+j]==(u8)text[j]; ++j) {}
        if (j==n) return 1;
    }
    return 0;
}
static u32 le16(u32 offset) { return file[offset]|((u32)file[offset+1]<<8); }
static u32 le32(u32 offset) { return le16(offset)|(le16(offset+2)<<16); }
static int check_shim(const wchar *path) {
    HANDLE in=CreateFileW(path,0x80000000,1,0,3,0x80,0);
    u32 size,high=0,count,pe,error;
    if (in==INVALID_HANDLE) {
        error=GetLastError();
        if (error==2 || error==3) return 1;
        say("Cannot read the candidate shim DLL.\r\n"); return 4;
    }
    size=GetFileSize(in,&high);
    if (size==0xffffffff) { CloseHandle(in); return 4; }
    if (high || size<0x40 || size>sizeof(file)) { CloseHandle(in); return 1; }
    if (!ReadFile(in,file,size,&count,0) || count!=size) { CloseHandle(in); return 4; }
    CloseHandle(in);
    if (file[0]!='M' || file[1]!='Z') return 1;
    pe=le32(0x3c);
    if (pe>size-26 || le32(pe)!=0x4550 || le16(pe+4)!=0x14c
        || !(le16(pe+22)&0x2000) || le16(pe+20)<2 || le16(pe+24)!=0x10b) return 1;
    return contains(size,"sgens-shim") && contains(size,"sgens-ddraw-shim.log")
        && contains(size,"ddraw-backend.dll") ? 0 : 1;
}

typedef struct {
    u32 size, offset, child_style, main_style, registry_root;
    const char *name, *original, *native_hash, *shim_hash;
} Edition;
static const Edition editions[] = {
    {1471488,0xCDBB0,0x4ACAE,0x50C66,0x87ADF,"German",
     "3e4329143247177c4415c06137bd5751e6b9de506405dfab4c15e3245a0e536a",
     "3fcda8a52d9158536fccba70ce654b1fb081ffdd93d31bcfebd0aa36c3cb5d96",
     "06dbbd6e2efd20d9ba0f4b41367e948c5d21224b3fc40e3edd8835ee36259849"},
    {1571328,0xDE940,0x5F19E,0x8B0FB,0x867CF,"English",
     "e102fcb25ade3255cb0262578431c942c0f59de8cb8b8c16a8a8ceddb4618504",
     "9f56f2e43630683d04a40f8fc6d01495bc4ff144b8bb77684ec29dccf866b4f7",
     "982838a50c97f403916e7ce43fb29ec3741620ce38dfa8b2e39d5aeba5d9e40e"}
};
static int patch(const wchar *input, const wchar *output, int shim) {
    HANDLE in, out;
    u32 high=0, size, count, i;
    char hash[65];
    const Edition *edition=0;
    const char *expected;
    in=CreateFileW(input,0x80000000,1,0,3,0x80,0); /* READ, share READ, OPEN_EXISTING */
    if (in==INVALID_HANDLE) { say("Cannot open input for reading.\r\n"); return 4; }
    size=GetFileSize(in,&high);
    if (high || (size!=editions[0].size && size!=editions[1].size)) {
        CloseHandle(in); say("Unsupported file size. Supply an original DE or EN sgens.exe.\r\n"); return 3;
    }
    if (!ReadFile(in,file,size,&count,0) || count!=size) {
        CloseHandle(in); say("Cannot read the complete input.\r\n"); return 4;
    }
    CloseHandle(in);
    sha256(file,size,hash);
    say("Input SHA-256:  "); say(hash); say("\r\n");
    for (i=0; i<2; ++i) {
        if (size==editions[i].size && equal(hash,editions[i].original)) edition=&editions[i];
    }
    if (!edition) {
        say("Unsupported checksum: unknown, modified, or already patched EXE.\r\n"); return 3;
    }
    /* XP/NT forbids the original user-mode OUT/IN at VGA ports 03B4/03B5.
     * Replace only the first three bytes of the obsolete detection routine:
     * BA B4 03 ... (MOV EDX,03B4) -> 31 C0 C3 (XOR EAX,EAX; RET).
     * Returning false bypasses the probe, preventing exception C0000096.
     * Native XP preserves the original window styles; both modes also use HKCU. */
    i=edition->offset;
    if (file[i]!=0xBA || file[i+1]!=0xB4 || file[i+2]!=0x03) {
        say("Internal patch-site verification failed.\r\n"); return 4;
    }
    file[i]=0x31; file[i+1]=0xC0; file[i+2]=0xC3;
    if (shim) {
        if (file[edition->child_style]!=0x82 || file[edition->main_style]!=0x93) {
            say("Internal window-style verification failed.\r\n"); return 4;
        }
        /* Full shim only: internal popup screens compete with the presenter.
         * 82000000 -> 46000000: WS_POPUP becomes WS_CHILD|WS_CLIPSIBLINGS;
         * WS_CLIPCHILDREN remains. The shim supplies presentation and input. */
        file[edition->child_style]=0x46;
        /* Full shim only: hide the initial desktop-sized black main window
         * until the shim sizes it. 93000000 -> 83000000 removes WS_VISIBLE. */
        file[edition->main_style]=0x83;
    }
    /* Original RegOpenKeyExA root: PUSH 80000002 (HKLM). Changing its low
     * byte to 01 selects HKCU, avoiding administrator-only installation paths.
     * Both graphics variants keep the original subkey and Path/CDPath values. */
    if (file[edition->registry_root]!=0x02) {
        say("Internal registry-root verification failed.\r\n"); return 4;
    }
    file[edition->registry_root]=0x01;
    expected=shim ? edition->shim_hash : edition->native_hash;
    sha256(file,size,hash);
    if (!equal(hash,expected)) { say("Internal output checksum mismatch.\r\n"); return 4; }
    /* CREATE_NEW: even an existing output or an input/output alias is never
     * overwritten. No changes at all are made before full input validation. */
    out=CreateFileW(output,0xC0000000,0,0,1,0x80,0);
    if (out==INVALID_HANDLE) {
        say("Cannot create output: it exists, its folder is missing, or access was denied.\r\n"); return 4;
    }
    if (!WriteFile(out,file,size,&count,0) || count!=size || !FlushFileBuffers(out)
        || SetFilePointer(out,0,0,0)!=0 || !ReadFile(out,file,size,&count,0) || count!=size
        || GetFileSize(out,&high)!=size || high) goto failed_output;
    sha256(file,size,hash);
    if (!equal(hash,expected)) goto failed_output;
    CloseHandle(out);
    say(edition->name); say(shim ? " full shim patch created." : " native XP patch created.");
    say(" Original unchanged.\r\nOutput SHA-256: ");
    say(hash); say("\r\n");
    return 0;
failed_output:
    CloseHandle(out);
    if (DeleteFileW(output)) say("Output write/verification failed; incomplete output removed.\r\n");
    else say("Output write/verification failed; could not remove incomplete output. Delete it manually.\r\n");
    return 4;
}
void __stdcall entry(void) {
    int argc=0, result=2;
    wchar **argv=CommandLineToArgvW(GetCommandLineW(),&argc);
    if (argv && argc==3 && option(argv[1],"--check-shim")) result=check_shim(argv[2]);
    else if (argv && argc==4 && (option(argv[1],"--native") || option(argv[1],"--shim")))
        result=patch(argv[2],argv[3],option(argv[1],"--shim"));
    else say("Usage: patch-xp.exe --native|--shim INPUT.exe OUTPUT.exe\r\n"
             "       patch-xp.exe --check-shim FILE.dll\r\n"
             "Native mode requires removing game-local DirectDraw wrapper DLLs.\r\n"
             "Output must not exist. Input is never modified.\r\n");
    if (argv) LocalFree(argv);
    ExitProcess((u32)result);
}
