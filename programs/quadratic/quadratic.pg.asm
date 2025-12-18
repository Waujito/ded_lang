call ._start
dump
halt
._start:
input r0
push r0
pop r0
ldc r1 $0
stm r1 r0
input r0
push r0
pop r0
ldc r1 $1
stm r1 r0
input r0
push r0
pop r0
ldc r1 $2
stm r1 r0
ldc r0 $1
ldm r0 r0
push r0
ldc r0 $1
ldm r0 r0
push r0
pop r1
pop r0
mul r0 r0 r1
push r0
ldc r0 $4
push r0
ldc r0 $0
ldm r0 r0
push r0
pop r1
pop r0
mul r0 r0 r1
push r0
ldc r0 $2
ldm r0 r0
push r0
pop r1
pop r0
mul r0 r0 r1
push r0
pop r1
pop r0
sub r0 r0 r1
push r0
pop r0
ldc r1 $3
stm r1 r0
ldc r0 $3
ldm r0 r0
push r0
pop r0
print r0
push r0
pop r0
ldc r0 $3
ldm r0 r0
push r0
ldc r0 $0
push r0
pop r1
pop r0
cmp r0 r1
ldc r0 $0
jmp.leq ._jmp_tps__0
ldc r0 $1
._jmp_tps__0:
push r0
pop r0
ldc r1 $0
cmp r0 r1
jmp.eq ._jmp_tps__2
ldc r0 $0
push r0
ldc r0 $1
ldm r0 r0
push r0
pop r1
pop r0
sub r0 r0 r1
push r0
ldc r0 $3
ldm r0 r0
push r0
pop r0
sqrt r0 r0
push r0
pop r1
pop r0
add r0 r0 r1
push r0
ldc r0 $2
push r0
ldc r0 $0
ldm r0 r0
push r0
pop r1
pop r0
mul r0 r0 r1
push r0
pop r1
pop r0
div r0 r0 r1
push r0
pop r0
ldc r1 $4
stm r1 r0
ldc r0 $0
push r0
ldc r0 $1
ldm r0 r0
push r0
pop r1
pop r0
sub r0 r0 r1
push r0
ldc r0 $3
ldm r0 r0
push r0
pop r0
sqrt r0 r0
push r0
pop r1
pop r0
sub r0 r0 r1
push r0
ldc r0 $2
push r0
ldc r0 $0
ldm r0 r0
push r0
pop r1
pop r0
mul r0 r0 r1
push r0
pop r1
pop r0
div r0 r0 r1
push r0
pop r0
ldc r1 $5
stm r1 r0
ldc r0 $2
push r0
pop r0
print r0
push r0
pop r0
ldc r0 $4
ldm r0 r0
push r0
pop r0
print r0
push r0
pop r0
ldc r0 $5
ldm r0 r0
push r0
pop r0
print r0
push r0
pop r0
jmp ._jmp_tps__1
._jmp_tps__2:
ldc r0 $3
ldm r0 r0
push r0
ldc r0 $0
push r0
pop r1
pop r0
cmp r0 r1
ldc r0 $0
jmp.neq ._jmp_tps__3
ldc r0 $1
._jmp_tps__3:
push r0
pop r0
ldc r1 $0
cmp r0 r1
jmp.eq ._jmp_tps__5
ldc r0 $0
push r0
ldc r0 $1
ldm r0 r0
push r0
pop r1
pop r0
sub r0 r0 r1
push r0
ldc r0 $2
push r0
ldc r0 $0
ldm r0 r0
push r0
pop r1
pop r0
mul r0 r0 r1
push r0
pop r1
pop r0
div r0 r0 r1
push r0
pop r0
ldc r1 $6
stm r1 r0
ldc r0 $1
push r0
pop r0
print r0
push r0
pop r0
ldc r0 $6
ldm r0 r0
push r0
pop r0
print r0
push r0
pop r0
jmp ._jmp_tps__4
._jmp_tps__5:
ldc r0 $0
push r0
pop r0
print r0
push r0
pop r0
._jmp_tps__4:
._jmp_tps__1:
ret
