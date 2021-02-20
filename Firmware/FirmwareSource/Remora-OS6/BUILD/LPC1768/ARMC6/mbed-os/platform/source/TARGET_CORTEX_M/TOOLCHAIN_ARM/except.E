# 1 ".\\mbed-os\\platform\\source\\TARGET_CORTEX_M\\TOOLCHAIN_ARM\\except.S"
# 1 "<built-in>" 1
# 380 "<built-in>"
# 1 "./BUILD/LPC1768/ARMC6\\mbed_config.h" 1
# 381 "<built-in>" 2
# 1 ".\\mbed-os\\platform\\source\\TARGET_CORTEX_M\\TOOLCHAIN_ARM\\except.S" 2
;
# 30 ".\\mbed-os\\platform\\source\\TARGET_CORTEX_M\\TOOLCHAIN_ARM\\except.S"
FAULT_TYPE_HARD_FAULT EQU 0x10
FAULT_TYPE_MEMMANAGE_FAULT EQU 0x20
FAULT_TYPE_BUS_FAULT EQU 0x30
FAULT_TYPE_USAGE_FAULT EQU 0x40

                PRESERVE8
                THUMB

                AREA |.text|, CODE, READONLY

HardFault_Handler PROC

                EXPORT HardFault_Handler
                LDR R3,=FAULT_TYPE_HARD_FAULT
                B Fault_Handler
                ENDP

MemManage_Handler PROC

                EXPORT MemManage_Handler
                LDR R3,=FAULT_TYPE_MEMMANAGE_FAULT
                B Fault_Handler
                ENDP

BusFault_Handler PROC

                EXPORT BusFault_Handler
                LDR R3,=FAULT_TYPE_BUS_FAULT
                B Fault_Handler
                ENDP

UsageFault_Handler PROC

                EXPORT UsageFault_Handler
                LDR R3,=FAULT_TYPE_USAGE_FAULT
                B Fault_Handler
                ENDP

Fault_Handler PROC
                EXPORT Fault_Handler

                IMPORT mbed_fault_handler
                IMPORT mbed_fault_context

                MRS R0,MSP
                LDR R1,=0x4
                MOV R2,LR
                TST R2,R1 ; Check EXC_RETURN for bit 2
                BEQ Fault_Handler_Continue
                MRS R0,PSP

Fault_Handler_Continue
                MOV R12,R3
                LDR R3,=mbed_fault_context
                LDR R1,[R3]
                LDR R2,[R0] ; Capture R0
                STR R2,[R1]
                ADDS R1,#4
                LDR R2,[R0,#4] ; Capture R1
                STR R2,[R1]
                ADDS R1,#4
                LDR R2,[R0,#8] ; Capture R2
                STR R2,[R1]
                ADDS R1,#4
                LDR R2,[R0,#12] ; Capture R3
                STR R2,[R1]
                ADDS R1,#4
                STMIA R1!,{R4-R7} ; Capture R4..R7
                MOV R7,R8 ; Capture R8
                STR R7,[R1]
                ADDS R1,#4
                MOV R7,R9 ; Capture R9
                STR R7,[R1]
                ADDS R1,#4
                MOV R7,R10 ; Capture R10
                STR R7,[R1]
                ADDS R1,#4
                MOV R7,R11 ; Capture R11
                STR R7,[R1]
                ADDS R1,#4
                LDR R2,[R0,#16] ; Capture R12
                STR R2,[R1]
                ADDS R1,#8 ; Add 8 here to capture LR next, we will capture SP later
                LDR R2,[R0,#20] ; Capture LR
                STR R2,[R1]
                ADDS R1,#4
                LDR R2,[R0,#24] ; Capture PC
                STR R2,[R1]
                ADDS R1,#4
                LDR R2,[R0,#28] ; Capture xPSR
                STR R2,[R1]
                ADDS R1,#4
                ; Adjust stack pointer to its original value and capture it
                MOV R3,R0
                ADDS R3,#0x20 ; Add 0x20 to get the SP value prior to exception
                LDR R6,=0x200
                TST R2,R6 ; Check for if STK was aligned by checking bit-9 in xPSR value
                BEQ Fault_Handler_Continue1
                ADDS R3,#0x4

Fault_Handler_Continue1
                MOV R5,LR
                LDR R6,=0x10 ; Check for bit-4 to see if FP context was saved
                TST R5,R6
                BNE Fault_Handler_Continue2
                ADDS R3,#0x48 ; 16 FP regs + FPCSR + 1 Reserved

Fault_Handler_Continue2
                MOV R4,R1
                SUBS R4,#0x10 ; Set the location of SP in ctx
                STR R3,[R4] ; Capture the adjusted SP
                MRS R2,PSP ; Get PSP
                STR R2,[R1]
                ADDS R1,#4
                MRS R2,MSP ; Get MSP
                STR R2,[R1]
                ADDS R1,#4
                MOV R2,LR ; Get current LR(EXC_RETURN)
                STR R2,[R1]
                ADDS R1,#4
                MRS R2,CONTROL ; Get CONTROL Reg
                STR R2,[R1]
                MOV R0,R12
                LDR R3,=mbed_fault_context
                LDR R1,[R3]
                BL mbed_fault_handler

                B . ; Just in case we come back here
                ENDP



                END
