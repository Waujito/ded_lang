ldc r0 $1
push r0
pop r0
ldc r1 $0
stm r1 r0
ldc r0 $2
push r0
pop r0
ldc r1 $0
stm r1 r0
ldc r0 $3
push r0
pop r0
ldc r1 $0
stm r1 r0
ldc r0 $4
push r0
pop r0
ldc r1 $0
stm r1 r0
ldc r0 $0
ldm r0 r0
push r0
ldc r0 $2
push r0
pop r1
pop r0
cmp r0 r1
ldc r0 $0
jmp.geq ._jmp_tps__0
ldc r0 $1
._jmp_tps__0:
push r0
pop r0
ldc r1 $0
cmp r0 r1
jmp.eq ._jmp_tps__2
ldc r0 $5
push r0
pop r0
ldc r1 $0
stm r1 r0
jmp ._jmp_tps__1
._jmp_tps__2:
ldc r0 $6
push r0
pop r0
ldc r1 $0
stm r1 r0
._jmp_tps__1:
ldc r0 $0
ldm r0 r0
push r0
pop r0
print r0
push r0
pop r0
dump
