 	l.s	F0	R0	x	flt[0] <- x			x
	mult.s	F0	F0	F0	flt[0] <- flt[0] * flt[0]	x^2
	lw	R1	R0	n	reg[1] <- n			counter
	l.s	F2	R0	one	flt[2] <- one			coeff
	l.s	F4	R0	zero	flt[4] <- zero			k
	l.s	F6	R0	one	flt[6] <- one			sum
	l.s	F8	R0	neg	flt[8] <- neg			-1
	l.s	F10	R0	one	flt[10] <- one			1
loop	beqz	R1	done						Have we run n times
	add.s	F4	F4	F10	flt[4] <- flt[4] + flt[10]	k+1
	div.s	F2	F2	F4	flt[2] <- flt[4] / flt[2]	coeff/k-1
	add.s	F4	F4	F10	flt[4] <- flt[4] + flt[10]	k+1
	div.s	F2	F2	F4	flt[2] <- flt[4] / flt[2]	coeff/k
	mult.s	F2	F2	F0	flt[2] <- flt[2] * flt[0]	coeff*x^2
	mult.s	F2	F2	F8	flt[2] <- flt[2] * flt[8]	coeff*-1
	add.s	F6	F6	F2	flt[6] <- flt[6] * flt[2]	sum+coeff
	subi	R1	R1	#1	reg[1] <- reg[1] - 1		n-1
	j	loop							loop
done	s.s	F6	R0	answer	reg[5] -> answer
	halt
answer	.df	0
n	.dw	10
x	.df	1.0
zero	.df	0.0
neg	.df	-1.0
one	.df	1.0
