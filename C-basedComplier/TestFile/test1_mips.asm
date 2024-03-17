lui $sp,0x1001
j main
program:
sw $ra 4($sp)
addi $s7 $zero 0
lw $s6 12($sp)
lw $s5 16($sp)
add $s4 $s6 $s5
sw $s7 20($sp)
lw $s3 8($sp)
ble $s3 $s4 B1
B0:
lw $s7 12($sp)
lw $s6 16($sp)
mul $s7 $s7 $s6
addi $s6 $zero 1
add $s7 $s7 $s6
lw $s5 8($sp)
add $s4 $s5 $s7
sw $s4 24($sp)
j B2
B1:
lw $s7 8($sp)
sw $s7 24($sp)
B2:
lw $s7 8($sp)
addi $s6 $zero 10
add $s7 $s7 $s6
B3:
lw $s7 20($sp)
addi $s6 $zero 100
bgt $s7 $s6 B5
B4:
lw $s7 24($sp)
addi $s6 $zero 2
mul $s7 $s7 $s6
sw $s7 20($sp)
sw $s7 24($sp)
j B3
B5:
lw $v0 20($sp)
lw $ra 4($sp)
jr $ra
demo:
sw $ra 4($sp)
lw $s7 8($sp)
addi $s6 $zero 2
add $s7 $s7 $s6
addi $s5 $zero 2
mul $s4 $s7 $s5
add $v0 $zero $s4
lw $ra 4($sp)
jr $ra
main:
addi $s7 $zero 3
addi $s6 $zero 43
addi $s5 $zero 22
add $s4 $s6 $s7
sw $s7 8($sp)
sw $s6 12($sp)
sw $s4 24($sp)
sw $sp 16($sp)
addi $sp $sp 16
jal demo
lw $sp 0($sp)
B6:
lw $s7 8($sp)
sw $s7 24($sp)
lw $s7 12($sp)
sw $s7 28($sp)
sw $v0 32($sp)
sw $sp 16($sp)
addi $sp $sp 16
jal program
lw $sp 0($sp)
B7:
add $v0 $zero $v0
j end
end:
