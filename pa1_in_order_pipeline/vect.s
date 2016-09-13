	addi	R1	R0	#12	R1 <- R0 + 3		n<-3
loop	beqz	R1	done		Have we run n times
	subi	R1	R1	#1	R1  <- R1 - 1		R1--
	l.s	F0	R2	a	FO  <- R2(a)		F0  <- a[n]
	l.s	F8	R2	b	F8  <- R2(b)		F8  <- b[n]
	l.s	F16	R2	c	F16 < R2(c)		F16 <- c[n]
	mult.s	F0	F0	F8	F0  <- F0 * F8		F0  <- a[n]   * b[n]
	add.s	F0	F0	F16	F0  <- F0 + F16		F0  <- a[n]   + b[n] 
	s.s	F0	R2	answer	answer+R2 = [F0]
	addui	R2	R2	#4	R2  <- R2 + 4
	j	loop
done	halt
a	.df	0.01
	.df	0.02
	.df	0.03
	.df	0.04
	.df	0.05
	.df	0.06
	.df	0.07
	.df	0.08
	.df	0.09
	.df	0.10
	.df	0.11
	.df	0.12
b	.df	1
	.df	2
	.df	3
	.df	4
	.df	5
	.df	6
	.df	7
	.df	8
	.df	9
	.df	10
	.df	11
	.df	12
c	.df	100
	.df	101
	.df	102
	.df	103
	.df	104
	.df	105
	.df	106
	.df	107
	.df	108
	.df	109
	.df	110
	.df	111
answer	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
