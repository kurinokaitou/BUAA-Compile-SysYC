	.file	1 "testmips.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	fp=xx
	.module	nooddspreg
	.abicalls
	.text
	.globl	a
	.data
	.align	2
	.type	a, @object
	.size	a, 4
a:
	.word	1
	.globl	b
	.align	2
	.type	b, @object
	.size	b, 16
b:
	.word	1
	.word	2
	.word	3
	.word	4
	.text
	.align	2
	.globl	main
	.set	nomips16
	.set	nomicromips
	.ent	main
	.type	main, @function
main:
	.frame	$fp,48,$31		# vars= 16, regs= 2/0, args= 16, gp= 8
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$fp,40($sp)
	move	$fp,$sp
	lui	$28,%hi(__gnu_local_gp)
	addiu	$28,$28,%lo(__gnu_local_gp)
	.cprestore	16
	lw	$2,%got(__stack_chk_guard)($28)
	lw	$2,0($2)
	sw	$2,36($fp)
	li	$2,1			# 0x1
	sw	$2,24($fp)
	li	$2,2			# 0x2
	sw	$2,28($fp)
	li	$2,3			# 0x3
	sw	$2,32($fp)
	lui	$2,%hi(b)
	addiu	$2,$2,%lo(b)
	sw	$0,12($2)
	move	$2,$0
	move	$4,$2
	lw	$2,%got(__stack_chk_guard)($28)
	lw	$3,36($fp)
	lw	$2,0($2)
	beq	$3,$2,$L3
	nop

	lw	$2,%call16(__stack_chk_fail)($28)
	move	$25,$2
	.reloc	1f,R_MIPS_JALR,__stack_chk_fail
1:	jalr	$25
	nop

$L3:
	move	$2,$4
	move	$sp,$fp
	lw	$31,44($sp)
	lw	$fp,40($sp)
	addiu	$sp,$sp,48
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	main
	.size	main, .-main
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04) 9.4.0"
