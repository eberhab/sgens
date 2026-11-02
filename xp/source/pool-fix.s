# SPDX-License-Identifier: MIT
.intel_syntax noprefix
.text
.global pool_fix
pool_fix:
# cdecl char *append(pool*, const char*); preserve all callee-saved registers.
push ebx
push esi
push edi
push ebp
mov ebx,[esp+20]
mov esi,[esp+24]
mov edi,esi
xor eax,eax
mov ecx,-1
repne scasb
not ecx
mov ebp,ecx                     # strlen + NUL
mov eax,[ebx+0x0c]
mov edx,[ebx+0x10]
sub edx,eax                    # conservative remaining capacity
cmp ebp,edx
jbe copy_string
mov edi,406
cmp edi,ebp
jae allocate
mov edi,ebp
allocate:
push edi
call alloc_target
add esp,4
test eax,eax
jz done
mov [ebx+0x0c],eax
add edi,eax
mov [ebx+0x10],edi              # exclusive end of actual new allocation
push eax
push dword ptr [ebx+0x14]
call list_target
add esp,8
mov [ebx+0x14],eax
copy_string:
mov edi,[ebx+0x0c]
mov eax,edi
mov ecx,ebp
rep movsb
mov [ebx+0x0c],edi
# Existing nodes own all string blocks; original destructor still frees them.
done:
pop ebp
pop edi
pop esi
pop ebx
ret
