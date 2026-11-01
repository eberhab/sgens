/* Loaded PE32 import adapter, MIT license.
 * Match resolved IAT addresses rather than assuming OriginalFirstThunk still
 * contains name RVAs. A loader/packer may omit that table or alias it to IAT.
 * No backend version, private offset, name-table dereference or code detour.
 * Only static imports in this module are covered, after LoadLibrary returns. */
typedef struct {void *base,*allocation;DWORD allocation_protect,size,state,protect,type;} IMPORT_MEMORY;
DLLIMPORT DWORD WINAPI VirtualQuery(const void*,IMPORT_MEMORY*,DWORD);
typedef struct {void *original,*replacement;} IMPORT_PATCH;
static int import_readable(BYTE *image,DWORD limit,DWORD offset,DWORD bytes)
{
    if(offset>limit || bytes>limit-offset || (DWORD)image>0xfffffffful-offset ||
       bytes>0xfffffffful-((DWORD)image+offset))return 0;
    DWORD pos=(DWORD)image+offset,end=pos+bytes;
    while(pos<end){
        IMPORT_MEMORY m;
        if(VirtualQuery((void*)pos,&m,sizeof(m))!=sizeof(m) || m.allocation!=image ||
           m.state!=0x1000 || (m.protect&0x101) || !(m.protect&0xee))return 0;
        DWORD start=(DWORD)m.base;
        if(!m.size || start>pos || m.size>0xfffffffful-start || start+m.size<=pos)return 0;
        pos=start+m.size;
    }
    return 1;
}
/* Preflight the whole descriptor/IAT walk before changing any slot. An absent
 * import directory is valid. Malformed or inaccessible tables fail explicitly.
 * Lookup/name tables are deliberately never read, even for normal PE images. */
static int patch_import_addresses(HMODULE module,const IMPORT_PATCH *patches,unsigned count,unsigned *matched)
{
    BYTE *image=module;DWORD bound=0xfffffffful-(DWORD)image,nt,optional,size,rva,bytes;
    *matched=0;
    if(!image || !import_readable(image,bound,0,64) || *(unsigned short*)image!=0x5a4d)return 0;
    nt=*(DWORD*)(image+0x3c);
    if(!import_readable(image,bound,nt,24) || *(DWORD*)(image+nt)!=0x4550 ||
       *(unsigned short*)(image+nt+4)!=0x14c)return 0;
    optional=*(unsigned short*)(image+nt+20);
    if(optional<112 || !import_readable(image,bound,nt+24,optional) ||
       *(unsigned short*)(image+nt+24)!=0x10b)return 0;
    size=*(DWORD*)(image+nt+24+56);
    if(!size || size>bound || nt>size || 24+optional>size-nt)return 0;
    if(*(DWORD*)(image+nt+24+92)<2)return 1;
    rva=*(DWORD*)(image+nt+24+104);bytes=*(DWORD*)(image+nt+24+108);
    if(!rva && !bytes)return 1;
    if(!rva || bytes<20 || !import_readable(image,size,rva,bytes))return 0;
    for(unsigned pass=0;pass<2;pass++){
        int terminated=0;
        for(DWORD at=0;at<=bytes-20;at+=20){
            DWORD *d=(DWORD*)(image+rva+at);
            if(!(d[0]|d[1]|d[2]|d[3]|d[4])){terminated=1;break;}
            DWORD thunk=d[4];
            if(!thunk || (thunk&3))return 0;
            for(;;){
                if(!import_readable(image,size,thunk,4))return 0;
                DWORD *slot=(DWORD*)(image+thunk),value=*slot;
                if(!value)break;
                if(pass)for(unsigned i=0;i<count;i++){
                    if(!patches[i].original || !patches[i].replacement ||
                       value!=(DWORD)patches[i].original)continue;
                    DWORD old,ignored;
                    if(!VirtualProtect(slot,4,0x04,&old))return 0;
                    *slot=(DWORD)patches[i].replacement;
                    if(!VirtualProtect(slot,4,old,&ignored))return 0;
                    ++*matched;break;
                }
                if(thunk>size-4)return 0;
                thunk+=4;
            }
        }
        if(!terminated)return 0;
    }
    return 1;
}
