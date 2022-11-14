	.file	1 "test.c"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	fp=xx
	.module	nooddspreg
	.abicalls
	.text
	.globl	d
	.data
	.align	2
	.type	d, @object
	.size	d, 4
d:
	.word	1
	.globl	c
	.rdata
	.align	2
	.type	c, @object
	.size	c, 4
c:
	.word	2
	.globl	arr
	.data
	.align	2
	.type	arr, @object
	.size	arr, 8
arr:
	.word	3
	.word	4
	.globl	carr
	.rdata
	.align	2
	.type	carr, @object
	.size	carr, 8
carr:
	.word	5
	.word	6
	.text
	.align	2
	.globl	add
	.set	nomips16
	.set	nomicromips
	.ent	add
	.type	add, @function
add:
	.frame	$fp,8,$31		# vars= 0, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-8
	sw	$fp,4($sp)
	move	$fp,$sp
	sw	$4,8($fp)
	sw	$5,12($fp)
	lw	$3,8($fp)
	lw	$2,12($fp)
	addu	$2,$3,$2
	move	$sp,$fp
	lw	$fp,4($sp)
	addiu	$sp,$sp,8
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	add
	.size	add, .-add
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
	lui	$2,%hi(arr)
	lw	$2,%lo(arr)($2)
	sw	$2,28($fp)
	li	$2,6			# 0x6
	sw	$2,32($fp)
	lw	$3,28($fp)
	li	$2,1			# 0x1
	bne	$3,$2,$L4
	nop

	lw	$2,28($fp)
	addiu	$2,$2,1
	move	$5,$2
	lw	$4,28($fp)
	.option	pic0
	jal	add
	nop

	.option	pic2
	sw	$2,36($fp)
$L4:
	move	$2,$0
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
