.global lab1
.extern lab2
.section myCode
lab1:
	mul r4, r3
	cmp r0, r4
	jgt lab1
	sub r4, r4
	jeq *[r1 + lab2]
	ldr r4, $36
.end