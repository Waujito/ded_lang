call ._start
dump
halt
.func_clearArea:
pop r0
ldc r1 $0
stm r1 r0
scrhw r0 r1
push r0
pop r0
ldc r1 $1
stm r1 r0
scrhw r0 r1
push r1
pop r0
ldc r1 $2
stm r1 r0
ldc r0 $1
ldm r0 r0
push r0
ldc r0 $2
ldm r0 r0
push r0
pop r1
pop r0
mul r0 r0 r1
push r0
ldc r0 $8
push r0
pop r1
pop r0
add r0 r0 r1
push r0
pop r0
ldc r1 $3
stm r1 r0
ldc r0 $0
ldm r0 r0
push r0
ldc r0 $3
ldm r0 r0
push r0
pop r1
pop r0
add r0 r0 r1
push r0
pop r0
ldc r1 $4
stm r1 r0
._jmp_tps__0:
ldc r0 $0
ldm r0 r0
push r0
ldc r0 $4
ldm r0 r0
push r0
pop r1
pop r0
cmp r0 r1
ldc r0 $0
jmp.geq ._jmp_tps__2
ldc r0 $1
._jmp_tps__2:
push r0
pop r0
ldc r1 $0
cmp r0 r1
jmp.eq ._jmp_tps__1
ldc r0 $0
ldm r0 r0
push r0
ldc r0 $0
push r0
pop r1
pop r0
stm r0 r1
push r1
pop r0
ldc r0 $0
ldm r0 r0
push r0
ldc r0 $1
push r0
pop r1
pop r0
add r0 r0 r1
push r0
pop r0
ldc r1 $0
stm r1 r0
jmp ._jmp_tps__0
._jmp_tps__1:
ldc r0 $0
push r0
pop r0
ret
pop r0
ret
.func_WriteCoords:
pop r0
ldc r1 $5
stm r1 r0
pop r0
ldc r1 $6
stm r1 r0
pop r0
ldc r1 $7
stm r1 r0
pop r0
ldc r1 $8
stm r1 r0
scrhw r0 r1
push r0
pop r0
ldc r1 $9
stm r1 r0
scrhw r0 r1
push r1
pop r0
ldc r1 $10
stm r1 r0
ldc r0 $7
ldm r0 r0
push r0
ldc r0 $10
ldm r0 r0
push r0
pop r1
pop r0
mul r0 r0 r1
push r0
ldc r0 $6
ldm r0 r0
push r0
pop r1
pop r0
add r0 r0 r1
push r0
pop r0
ldc r1 $11
stm r1 r0
ldc r0 $11
ldm r0 r0
push r0
ldc r0 $3
push r0
pop r1
pop r0
shr r0 r0 r1
push r0
pop r0
ldc r1 $12
stm r1 r0
ldc r0 $5
ldm r0 r0
push r0
ldc r0 $12
ldm r0 r0
push r0
pop r1
pop r0
add r0 r0 r1
push r0
pop r0
ldc r1 $13
stm r1 r0
ldc r0 $11
ldm r0 r0
push r0
ldc r0 $7
push r0
pop r1
pop r0
and r0 r0 r1
push r0
pop r0
ldc r1 $14
stm r1 r0
ldc r0 $13
ldm r0 r0
push r0
pop r0
ldm r0 r1
push r1
pop r0
ldc r1 $15
stm r1 r0
ldc r0 $15
ldm r0 r0
push r0
ldc r0 $8
ldm r0 r0
push r0
ldc r0 $8
push r0
ldc r0 $14
ldm r0 r0
push r0
pop r1
pop r0
mul r0 r0 r1
push r0
pop r1
pop r0
shl r0 r0 r1
push r0
pop r1
pop r0
or r0 r0 r1
push r0
pop r0
ldc r1 $15
stm r1 r0
ldc r0 $12
ldm r0 r0
push r0
pop r0
print r0
push r0
pop r0
ldc r0 $14
ldm r0 r0
push r0
pop r0
print r0
push r0
pop r0
ldc r0 $13
ldm r0 r0
push r0
pop r0
print r0
push r0
pop r0
ldc r0 $13
ldm r0 r0
push r0
ldc r0 $15
ldm r0 r0
push r0
pop r1
pop r0
stm r0 r1
push r1
pop r0
ldc r0 $15
ldm r0 r0
push r0
pop r0
print r0
push r0
pop r0
ret
._start:
scrhw r0 r1
push r0
pop r0
ldc r1 $16
stm r1 r0
scrhw r0 r1
push r1
pop r0
ldc r1 $17
stm r1 r0
ldc r0 $50
push r0
pop r0
ldc r1 $18
stm r1 r0
ldc r0 $18
ldm r0 r0
push r0
call .func_clearArea
push r0
pop r0
ldc r0 $12
push r0
pop r0
ldc r1 $19
stm r1 r0
ldc r0 $7
push r0
pop r0
ldc r1 $20
stm r1 r0
ldc r0 $7
push r0
pop r0
ldc r1 $21
stm r1 r0
ldc r0 $0
push r0
pop r0
ldc r1 $22
stm r1 r0
._jmp_tps__3:
ldc r0 $22
ldm r0 r0
push r0
ldc r0 $16
ldm r0 r0
push r0
pop r1
pop r0
cmp r0 r1
ldc r0 $0
jmp.geq ._jmp_tps__5
ldc r0 $1
._jmp_tps__5:
push r0
pop r0
ldc r1 $0
cmp r0 r1
jmp.eq ._jmp_tps__4
ldc r0 $0
push r0
pop r0
ldc r1 $23
stm r1 r0
._jmp_tps__6:
ldc r0 $23
ldm r0 r0
push r0
ldc r0 $17
ldm r0 r0
push r0
pop r1
pop r0
cmp r0 r1
ldc r0 $0
jmp.geq ._jmp_tps__8
ldc r0 $1
._jmp_tps__8:
push r0
pop r0
ldc r1 $0
cmp r0 r1
jmp.eq ._jmp_tps__7
ldc r0 $23
ldm r0 r0
push r0
ldc r0 $20
ldm r0 r0
push r0
pop r1
pop r0
sub r0 r0 r1
push r0
ldc r0 $23
ldm r0 r0
push r0
ldc r0 $20
ldm r0 r0
push r0
pop r1
pop r0
sub r0 r0 r1
push r0
pop r1
pop r0
mul r0 r0 r1
push r0
ldc r0 $22
ldm r0 r0
push r0
ldc r0 $21
ldm r0 r0
push r0
pop r1
pop r0
sub r0 r0 r1
push r0
ldc r0 $22
ldm r0 r0
push r0
ldc r0 $21
ldm r0 r0
push r0
pop r1
pop r0
sub r0 r0 r1
push r0
pop r1
pop r0
mul r0 r0 r1
push r0
pop r1
pop r0
add r0 r0 r1
push r0
ldc r0 $19
ldm r0 r0
push r0
pop r1
pop r0
cmp r0 r1
ldc r0 $0
jmp.gt ._jmp_tps__9
ldc r0 $1
._jmp_tps__9:
push r0
pop r0
ldc r1 $0
cmp r0 r1
jmp.eq ._jmp_tps__10
ldc r0 $23
ldm r0 r0
push r0
pop r0
print r0
push r0
pop r0
ldc r0 $22
ldm r0 r0
push r0
pop r0
print r0
push r0
pop r0
ldc r0 $1
push r0
ldc r0 $22
ldm r0 r0
push r0
ldc r0 $23
ldm r0 r0
push r0
ldc r0 $18
ldm r0 r0
push r0
call .func_WriteCoords
push r0
pop r0
._jmp_tps__10:
ldc r0 $23
ldm r0 r0
push r0
ldc r0 $1
push r0
pop r1
pop r0
add r0 r0 r1
push r0
pop r0
ldc r1 $23
stm r1 r0
jmp ._jmp_tps__6
._jmp_tps__7:
ldc r0 $22
ldm r0 r0
push r0
ldc r0 $1
push r0
pop r1
pop r0
add r0 r0 r1
push r0
pop r0
ldc r1 $22
stm r1 r0
jmp ._jmp_tps__3
._jmp_tps__4:
ldc r0 $18
ldm r0 r0
push r0
pop r0
draw r0
push r0
pop r0
ret
