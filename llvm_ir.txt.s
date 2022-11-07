	.text
	.file	"llvm_ir.txt"
	.globl	getint                  # -- Begin function getint
	.p2align	4, 0x90
	.type	getint,@function
getint:                                 # @getint
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	leaq	4(%rsp), %rsi
	movl	$.L.str, %edi
	xorl	%eax, %eax
	callq	__isoc99_scanf
	movl	4(%rsp), %eax
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	getint, .Lfunc_end0-getint
	.cfi_endproc
                                        # -- End function
	.globl	putint                  # -- Begin function putint
	.p2align	4, 0x90
	.type	putint,@function
putint:                                 # @putint
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	%edi, %esi
	movl	%edi, 4(%rsp)
	movl	$.L.str, %edi
	xorl	%eax, %eax
	callq	printf
	popq	%rax
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end1:
	.size	putint, .Lfunc_end1-putint
	.cfi_endproc
                                        # -- End function
	.globl	putstr                  # -- Begin function putstr
	.p2align	4, 0x90
	.type	putstr,@function
putstr:                                 # @putstr
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	movq	%rdi, %rsi
	movq	%rdi, (%rsp)
	movl	$.L.str.1, %edi
	xorl	%eax, %eax
	callq	printf
	popq	%rax
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end2:
	.size	putstr, .Lfunc_end2-putstr
	.cfi_endproc
                                        # -- End function
	.globl	CalcMod                 # -- Begin function CalcMod
	.p2align	4, 0x90
	.type	CalcMod,@function
CalcMod:                                # @CalcMod
	.cfi_startproc
# %bb.0:                                # %_b0
	movl	%edi, -4(%rsp)
	movslq	%edi, %rax
	imulq	$1757988013, %rax, %rcx # imm = 0x68C8C4AD
	movq	%rcx, %rdx
	shrq	$63, %rdx
	sarq	$44, %rcx
	addl	%edx, %ecx
	imull	$10007, %ecx, %ecx      # imm = 0x2717
	subl	%ecx, %eax
                                        # kill: def $eax killed $eax killed $rax
	retq
.Lfunc_end3:
	.size	CalcMod, .Lfunc_end3-CalcMod
	.cfi_endproc
                                        # -- End function
	.globl	Check                   # -- Begin function Check
	.p2align	4, 0x90
	.type	Check,@function
Check:                                  # @Check
	.cfi_startproc
# %bb.0:                                # %_b0
	movl	%edi, -4(%rsp)
	testl	%edi, %edi
	js	.LBB4_1
# %bb.2:                                # %_b1
	xorl	%edi, %edi
	cmpl	$5, -4(%rsp)
	setl	%dil
	testl	%edi, %edi
	je	.LBB4_5
.LBB4_4:                                # %_b3
	movl	$1, %eax
	retq
.LBB4_1:
	notl	%edi
	shrl	$31, %edi
	testl	%edi, %edi
	jne	.LBB4_4
.LBB4_5:                                # %_b4
	xorl	%eax, %eax
	retq
.Lfunc_end4:
	.size	Check, .Lfunc_end4-Check
	.cfi_endproc
                                        # -- End function
	.globl	check                   # -- Begin function check
	.p2align	4, 0x90
	.type	check,@function
check:                                  # @check
	.cfi_startproc
# %bb.0:                                # %_b0
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	%edi, 4(%rsp)
	movl	%esi, (%rsp)
	testl	%edi, %edi
	js	.LBB5_1
# %bb.2:                                # %_b1
	xorl	%edi, %edi
	cmpl	$4, 4(%rsp)
	setg	%dil
	testl	%edi, %edi
	jne	.LBB5_5
.LBB5_4:                                # %_b3
	movl	(%rsp), %edi
	callq	Check
	xorl	%edi, %edi
	testl	%eax, %eax
	sete	%dil
.LBB5_5:                                # %_b4
	testl	%edi, %edi
	je	.LBB5_6
.LBB5_7:                                # %_b5
	xorl	%eax, %eax
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.LBB5_1:
	.cfi_def_cfa_offset 16
	shrl	$31, %edi
	testl	%edi, %edi
	je	.LBB5_4
	jmp	.LBB5_5
.LBB5_6:                                # %_b6
	movl	$Map, %eax
	movl	4(%rsp), %ecx
	movslq	(%rsp), %rdx
	leal	(%rcx,%rcx,4), %ecx
	movslq	%ecx, %rcx
	leaq	(%rax,%rcx,4), %rax
	cmpl	$0, (%rax,%rdx,4)
	je	.LBB5_7
# %bb.8:                                # %_b8
	movl	$1, %eax
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end5:
	.size	check, .Lfunc_end5-check
	.cfi_endproc
                                        # -- End function
	.globl	dfs                     # -- Begin function dfs
	.p2align	4, 0x90
	.type	dfs,@function
dfs:                                    # @dfs
	.cfi_startproc
# %bb.0:                                # %_b0
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r12
	pushq	%rbx
	subq	$16, %rsp
	.cfi_offset %rbx, -48
	.cfi_offset %r12, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	movq	%rcx, %r15
                                        # kill: def $edi killed $edi def $rdi
	movl	%edi, -40(%rbp)
	movl	%esi, -36(%rbp)
	movl	%edx, -44(%rbp)
	movslq	%esi, %rax
	leal	(%rdi,%rdi,4), %ecx
	movslq	%ecx, %rcx
	shlq	$2, %rax
	cmpl	$0, val(%rax,%rcx,4)
	jne	.LBB6_12
# %bb.1:                                # %_b2
	movl	$val, %r14d
	movl	-40(%rbp), %eax
	movslq	-36(%rbp), %rbx
	leal	(%rax,%rax,4), %eax
	cltq
	leaq	(%r14,%rax,4), %r12
	movl	-44(%rbp), %ecx
	leaq	(%r15,%rax,4), %rax
	imull	(%rax,%rbx,4), %ecx
	movslq	%ecx, %rax
	imulq	$1757988013, %rax, %rcx # imm = 0x68C8C4AD
	movq	%rcx, %rdx
	shrq	$63, %rdx
	sarq	$44, %rcx
	addl	%edx, %ecx
	imull	$10007, %ecx, %ecx      # imm = 0x2717
	negl	%ecx
	leal	10007(%rax,%rcx), %edi
	callq	CalcMod
	movl	%eax, (%r12,%rbx,4)
	movl	-40(%rbp), %eax
	movslq	-36(%rbp), %rcx
	leal	(%rax,%rax,4), %eax
	cltq
	leaq	(%r14,%rax,4), %rax
	cmpl	$0, (%rax,%rcx,4)
	jne	.LBB6_3
# %bb.2:                                # %_b4
	movl	-40(%rbp), %eax
	movslq	-36(%rbp), %rcx
	leal	(%rax,%rax,4), %eax
	cltq
	leaq	(%r14,%rax,4), %rax
	movl	$1, (%rax,%rcx,4)
.LBB6_3:                                # %_b6
	movq	%rsp, %rax
	leaq	-16(%rax), %rbx
	movq	%rbx, %rsp
	movl	$0, -16(%rax)
	movq	%rsp, %rax
	leaq	-16(%rax), %r12
	movq	%r12, %rsp
	movl	$0, -16(%rax)
	movl	-40(%rbp), %edi
	incl	%edi
	movl	-36(%rbp), %esi
	callq	check
	testl	%eax, %eax
	je	.LBB6_5
# %bb.4:                                # %_b7
	movl	-40(%rbp), %eax
	movslq	-36(%rbp), %rsi
	leal	(%rax,%rax,4), %ecx
	movl	%eax, %edi
	incl	%edi
	movslq	%ecx, %rax
	leaq	(%r14,%rax,4), %rax
	movl	(%rax,%rsi,4), %edx
                                        # kill: def $esi killed $esi killed $rsi
	movq	%r15, %rcx
	callq	dfs
	movl	$1, (%rbx)
.LBB6_5:                                # %_b9
	movl	-40(%rbp), %edi
	movl	-36(%rbp), %esi
	incl	%esi
	callq	check
	testl	%eax, %eax
	je	.LBB6_7
# %bb.6:                                # %_b10
	movl	-40(%rbp), %edi
	movslq	-36(%rbp), %rax
	leal	1(%rax), %esi
	leal	(%rdi,%rdi,4), %ecx
	movslq	%ecx, %rcx
	leaq	(%r14,%rcx,4), %rcx
	movl	(%rcx,%rax,4), %edx
                                        # kill: def $edi killed $edi killed $rdi
	movq	%r15, %rcx
	callq	dfs
	movl	$1, (%r12)
.LBB6_7:                                # %_b12
	xorl	%eax, %eax
	cmpl	$0, (%rbx)
	setne	%cl
	jne	.LBB6_8
# %bb.9:                                # %_b13
	xorl	%eax, %eax
	cmpl	$0, (%r12)
	setne	%al
	testl	%eax, %eax
	jne	.LBB6_12
.LBB6_11:                               # %_b16
	movl	-40(%rbp), %eax
	movslq	-36(%rbp), %rcx
	leal	(%rax,%rax,4), %eax
	cltq
	leaq	(%r14,%rax,4), %rax
	movl	$-1, (%rax,%rcx,4)
.LBB6_12:                               # %_b1
	leaq	-32(%rbp), %rsp
	popq	%rbx
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.LBB6_8:
	.cfi_def_cfa %rbp, 16
	movb	%cl, %al
	testl	%eax, %eax
	jne	.LBB6_12
	jmp	.LBB6_11
.Lfunc_end6:
	.size	dfs, .Lfunc_end6-dfs
	.cfi_endproc
                                        # -- End function
	.globl	main                    # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %_b0
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$120, %rsp
	.cfi_def_cfa_offset 176
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movl	$.str1, %edi
	callq	putstr
	movl	$0, 12(%rsp)
	xorl	%eax, %eax
	testb	%al, %al
	jne	.LBB7_5
# %bb.1:                                # %_b2
	movl	$0, 16(%rsp)
	jmp	.LBB7_2
	.p2align	4, 0x90
.LBB7_3:                                # %_b4
                                        #   in Loop: Header=BB7_2 Depth=1
	movl	12(%rsp), %eax
	movslq	16(%rsp), %rbx
	leal	(%rax,%rax,4), %eax
	cltq
	leaq	20(%rsp,%rax,4), %rbp
	callq	getint
	movl	%eax, (%rbp,%rbx,4)
	incl	16(%rsp)
.LBB7_2:                                # %_b3
                                        # =>This Loop Header: Depth=1
                                        #     Child Loop BB7_4 Depth 2
	cmpl	$5, 16(%rsp)
	jl	.LBB7_3
	.p2align	4, 0x90
.LBB7_4:                                # %_b5
                                        #   Parent Loop BB7_2 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	movl	12(%rsp), %eax
	incl	%eax
	movl	%eax, 12(%rsp)
	cmpl	$5, %eax
	je	.LBB7_4
	jmp	.LBB7_2
.LBB7_5:                                # %_b9
	movl	$strP, %ebx
	movl	$val, %r13d
	movl	$0, 12(%rsp)
	cmpl	$1, 12(%rsp)
	jg	.LBB7_8
	.p2align	4, 0x90
.LBB7_7:                                # %_b11
                                        # =>This Inner Loop Header: Depth=1
	movslq	12(%rsp), %rbp
	callq	getint
	movl	%eax, (%rbx,%rbp,4)
	incl	12(%rsp)
	cmpl	$1, 12(%rsp)
	jle	.LBB7_7
.LBB7_8:                                # %_b12
	callq	getint
	movl	%eax, base(%rip)
	movl	(%rbx), %edi
	movl	strP+4(%rip), %esi
	leaq	20(%rsp), %rcx
	movl	%eax, %edx
	callq	dfs
	movl	40(%r13), %ebx
	movl	val+44(%rip), %ebp
	movl	val+48(%rip), %r12d
	movl	val+52(%rip), %r15d
	movl	val+56(%rip), %r14d
	movl	$.str2, %edi
	callq	putstr
	movl	%ebx, %edi
	callq	putint
	movl	$.str3, %edi
	callq	putstr
	movl	%ebp, %edi
	callq	putint
	movl	$.str4, %edi
	callq	putstr
	movl	%r12d, %edi
	callq	putint
	movl	$.str5, %edi
	callq	putstr
	movl	%r15d, %edi
	callq	putint
	movl	$.str6, %edi
	callq	putstr
	movl	%r14d, %edi
	callq	putint
	movl	$.str7, %edi
	callq	putstr
	movl	60(%r13), %ebx
	movl	val+68(%rip), %ebp
	movl	val+76(%rip), %r14d
	movl	$.str8, %edi
	callq	putstr
	movl	%ebx, %edi
	callq	putint
	movl	$.str9, %edi
	callq	putstr
	movl	%ebp, %edi
	callq	putint
	movl	$.str10, %edi
	callq	putstr
	movl	%r14d, %edi
	callq	putint
	movl	$.str11, %edi
	callq	putstr
	movl	80(%r13), %ebp
	movl	val+88(%rip), %ebx
	movl	val+92(%rip), %r15d
	movl	val+96(%rip), %r14d
	movl	$.str12, %edi
	callq	putstr
	movl	%ebp, %edi
	callq	putint
	movl	$.str13, %edi
	callq	putstr
	movl	%ebx, %edi
	callq	putint
	movl	$.str14, %edi
	callq	putstr
	movl	%r15d, %edi
	callq	putint
	movl	$.str15, %edi
	callq	putstr
	movl	%r14d, %edi
	callq	putint
	movl	$.str16, %edi
	callq	putstr
	xorl	%eax, %eax
	addq	$120, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end7:
	.size	main, .Lfunc_end7-main
	.cfi_endproc
                                        # -- End function
	.type	.L.str,@object          # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"%d"
	.size	.L.str, 3

	.type	.L.str.1,@object        # @.str.1
.L.str.1:
	.asciz	"%s"
	.size	.L.str.1, 3

	.type	Mod,@object             # @Mod
	.section	.rodata,"a",@progbits
	.globl	Mod
	.p2align	2
Mod:
	.long	10007                   # 0x2717
	.size	Mod, 4

	.type	dx,@object              # @dx
	.globl	dx
	.p2align	2
dx:
	.long	1                       # 0x1
	.long	0                       # 0x0
	.size	dx, 8

	.type	dy,@object              # @dy
	.globl	dy
	.p2align	2
dy:
	.long	0                       # 0x0
	.long	1                       # 0x1
	.size	dy, 8

	.type	Map,@object             # @Map
	.globl	Map
	.p2align	4
Map:
	.long	0                       # 0x0
	.long	1                       # 0x1
	.long	0                       # 0x0
	.long	1                       # 0x1
	.long	0                       # 0x0
	.long	0                       # 0x0
	.long	1                       # 0x1
	.long	1                       # 0x1
	.long	1                       # 0x1
	.long	1                       # 0x1
	.long	1                       # 0x1
	.long	1                       # 0x1
	.long	1                       # 0x1
	.long	1                       # 0x1
	.long	1                       # 0x1
	.long	1                       # 0x1
	.long	0                       # 0x0
	.long	1                       # 0x1
	.long	0                       # 0x0
	.long	1                       # 0x1
	.long	1                       # 0x1
	.long	1                       # 0x1
	.long	1                       # 0x1
	.long	0                       # 0x0
	.long	1                       # 0x1
	.size	Map, 100

	.type	strP,@object            # @strP
	.bss
	.globl	strP
	.p2align	2
strP:
	.zero	8
	.size	strP, 8

	.type	ans,@object             # @ans
	.globl	ans
	.p2align	2
ans:
	.long	0                       # 0x0
	.size	ans, 4

	.type	base,@object            # @base
	.globl	base
	.p2align	2
base:
	.long	0                       # 0x0
	.size	base, 4

	.type	val,@object             # @val
	.globl	val
	.p2align	4
val:
	.zero	100
	.size	val, 100

	.type	.str1,@object           # @.str1
	.section	.rodata,"a",@progbits
	.globl	.str1
.str1:
	.asciz	"19373459\n"
	.size	.str1, 10

	.type	.str2,@object           # @.str2
	.globl	.str2
.str2:
	.asciz	"val20: "
	.size	.str2, 8

	.type	.str3,@object           # @.str3
	.globl	.str3
.str3:
	.asciz	", val21:"
	.size	.str3, 9

	.type	.str4,@object           # @.str4
	.globl	.str4
.str4:
	.asciz	", val22:"
	.size	.str4, 9

	.type	.str5,@object           # @.str5
	.globl	.str5
.str5:
	.asciz	", val23:"
	.size	.str5, 9

	.type	.str6,@object           # @.str6
	.globl	.str6
.str6:
	.asciz	", val24:"
	.size	.str6, 9

	.type	.str7,@object           # @.str7
	.globl	.str7
.str7:
	.asciz	"\n"
	.size	.str7, 2

	.type	.str8,@object           # @.str8
	.globl	.str8
.str8:
	.asciz	"val30: "
	.size	.str8, 8

	.type	.str9,@object           # @.str9
	.globl	.str9
.str9:
	.asciz	", val32: "
	.size	.str9, 10

	.type	.str10,@object          # @.str10
	.globl	.str10
.str10:
	.asciz	", val34: "
	.size	.str10, 10

	.type	.str11,@object          # @.str11
	.globl	.str11
.str11:
	.asciz	"\n"
	.size	.str11, 2

	.type	.str12,@object          # @.str12
	.globl	.str12
.str12:
	.asciz	"val40: "
	.size	.str12, 8

	.type	.str13,@object          # @.str13
	.globl	.str13
.str13:
	.asciz	", val42:"
	.size	.str13, 9

	.type	.str14,@object          # @.str14
	.globl	.str14
.str14:
	.asciz	", val43:"
	.size	.str14, 9

	.type	.str15,@object          # @.str15
	.globl	.str15
.str15:
	.asciz	", val44:"
	.size	.str15, 9

	.type	.str16,@object          # @.str16
	.globl	.str16
.str16:
	.asciz	"\n"
	.size	.str16, 2

	.section	".note.GNU-stack","",@progbits
