	lw	R1	R0	op1	reg[1] <- op1
	lw	R2	R0	op2	reg[2] <- op2
	lw	R3	R0	op3	reg[3] <- op3
	add	R4	R1	R2	reg[4] <- reg[1] + reg[2]
	sub	R5	R4	R3	reg[5] <- reg[4] - reg[3]
	sw	R5	R0	answer	reg[5] -> answer
done	halt
op1	.dw	50			
op2	.dw	30
op3	.dw	20
answer	.dw	0
