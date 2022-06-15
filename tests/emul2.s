.extern lab1
.global lab2
.section myCode
mesto:
.word 2
	ldr r0, $8
	ldr r1, $2
	ldr r4, $1
	ldr r3, mesto
.section myData
lab2:
.word 1
.word 47
	ldr r4, $10
	push r1
	pop r4
	halt
.end