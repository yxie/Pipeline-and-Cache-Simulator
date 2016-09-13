	addi	R1	R0	#3	R1 <- R0 + 3		n<-3
	addui	R3	R0	#4	R3 <- R0 + 4		R3<-4
	addui	R4	R0	#8	R4 <- R0 + 8		R3<-8
	addui	R5	R0	#12	R5 <- R0 + 12		R3<-12
loop	beqz	R1	done		Have we run n times
	subi	R1	R1	#1	R1  <- R1 - 1		R1--
	l.s	F0	R2	a	FO  <- R2(a)		F0  <- a[n]
	l.s	F8	R2	b	F8  <- R2(b)		F8  <- b[n]
	l.s	F16	R2	c	F16 < R2(c)		F16 <- c[n]
	l.s	F2	R3	a	F2  <- R3(a)		F2  <- a[n+1]
	l.s	F10	R3	b	F8  <- R3(b)		F10 <- b[n+1]
	l.s	F18	R3	c	F18 <- R3(c)		F18 <- c[n+1]
	l.s	F4	R4	a	F4  <- R4(a)		F4  <- a[n+2]
	l.s	F12	R4	b	F8  <- R4(b)		F12 <- b[n+2]
	l.s	F20	R4	c	F20 <- R4(c)		F20 <- c[n+2]
	l.s	F6	R5	a	F6  <- R5(a)		F6  <- a[n+3]
	l.s	F14	R5	b	F8  <- R5(b)		F14 <- b[n+3]
	l.s	F22	R5	c	F22 <- R5(c)		F22 <- c[n+3]
	mult.s	F0	F0	F8	F0  <- F0 * F8		F0  <- a[n]   * b[n]
	mult.s	F2	F2	F10	F2  <- F2 * F10		F2  <- a[n+1] * b[n+1] 
	mult.s	F4	F4	F12	F4  <- F4 * F12		F4  <- a[n+1] * b[n+1] 
	mult.s	F6	F6	F14	F6  <- F6 * F14		F6  <- a[n+1] * b[n+1] 
	add.s	F0	F0	F16	F0  <- F0 + F16		F0  <- a[n]   + b[n] 
	add.s	F2	F2	F18	F2  <- F2 + F18		F2  <- a[n+1] + b[n+1]
	add.s	F4	F4	F20	F4  <- F4 + F20		F4  <- a[n+2] + b[n+2]
	add.s	F6	F6	F22	F6  <- F6 + F22		F6  <- a[n+3] + b[n+3]
	s.s	F0	R2	answer	answer+R2 = [F0]
	s.s	F2	R3	answer	answer+R3 = [F2]
	s.s	F4	R4	answer	answer+R4 = [F4] 
	s.s	F6	R5	answer	answer+R5 = [F6] 
	addui	R2	R2	#16	R2  <- R2 + 4
	addui	R3	R3	#16	R3  <- R3 + 4
	addui	R4	R4	#16	R4  <- R4 + 4
	addui	R5	R5	#16	R5  <- R5 + 4
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
