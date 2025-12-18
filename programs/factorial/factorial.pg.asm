call ._start
dump
halt
.func_factorial:
pop r0
ldc r1 $0
stm r1 r0
ldc r0 $0
ldm r0 r0
push r0
ldc r0 $1
push r0
pop r1
pop r0
cmp r0 r1
ldc r0 $0
jmp.gt ._jmp_tps__0
ldc r0 $1
._jmp_tps__0:
push r0
pop r0
ldc r1 $0
cmp r0 r1
jmp.eq ._jmp_tps__1
ldc r0 $1
push r0
pop r0
ret
pop r0
._jmp_tps__1:
ldc r0 $0
ldm r0 r0
push r0
ldc r0 $0
ldm r0 r0
push r0
ldc r0 $1
push r0
pop r1
pop r0
sub r0 r0 r1
push r0
call .func_factorial
push r0
pop r1
pop r0
mul r0 r0 r1
push r0
pop r0
ldc r1 $0
stm r1 r0
ret
._start:
ldc r0 $5
push r0
call .func_factorial
push r0
pop r0
print r0
push r0
pop r0
ret
