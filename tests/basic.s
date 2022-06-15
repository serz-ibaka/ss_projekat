.global lab1
.section myCode
mesto:
.word 2
	ldr r0, $8
	ldr r1, $2
	ldr r4, $1
	ldr r3, mesto
lab1:
	mul r4, r3
	cmp r0, r4
	jgt lab1
	sub r4, r4
	jeq *[r1 + lab2]
	ldr r4, $36
lab2:
.word 1
.word 47
	ldr r4, $10
	halt
.end