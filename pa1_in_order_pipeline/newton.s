start	l.s	F0	R0	a	flt[0] <- a
	l.s	F2	R0	b	flt[2] <- b
	l.s	F4	R0	c	flt[4] <- c
	lw	R1	R0	n	reg[1] <- n
	l.s	F6	R0	neg	flt[6] <- neg
	l.s	F10	R0	two	flt[10] <- two
	mult.s	F8	F6	F2	flt[8] <- flt[6] * flt[2]	-b
	div.s	F8	F8	F0	flt[8] <- flt[8] * flt[0]	-b/a
	add.s	F8	F8	F6	flt[8] <- flt[8] * flt[6]	-b/a-1
loop	beqz	R1	done						Have we run n times
	subi	R1	R1	#1	reg[1] <- reg[1] - 1		c--
	mult.s	F12	F8	F8	flt[12] <- flt[8] * flt[8]	x^2
	mult.s	F12	F12	F0	flt[12] <- flt[12] * flt[0]	ax^2
	mult.s	F14	F8	F2	flt[14] <- flt[8] * flt[2]	bx
	add.s	F12	F12	F4	flt[12] <- flt[12] + flt[4]	ax^2+c
	add.s	F12	F12	F14	flt[12] <- flt[12] + flt[14]	ax^2+bx+c
	mult.s	F14	F0	F8	flt[14] <- flt[0] * flt[8]	ax
	mult.s	F14	F10	F14	flt[14] <- flt[10] * flt[14]	2ax
	add.s	F14	F14	F2	flt[14] <- flt[14] + flt[2]	2ax + b
	div.s	F12	F12	F14	flt[12] <- flt[12] / flt[14]	f(x)/f'(x)
	sub.s	F8	F8	F12	flt[8] <- flt[8] - flt[12]	G- f(x)/f'(x)
	j	loop
done	s.s	F8	R0	answer	flt[8] -> answer
	halt
a	.df	1.0
b	.df	2.0
c	.df	0
neg	.df	-1.0
n	.dw	3
two	.df	2.0
answer	.df	0
