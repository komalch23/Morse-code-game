#include "hardware/regs/addressmap.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/timer.h"
#include "hardware/regs/m0plus.h"

.syntax unified
.cpu    cortex-m0plus
.thumb
.global main_asm
.align 4

.equ    ALRM_ISR_OFFSET, 0x40
.equ    GPIO_ISR_OFFSET, 0x74
.equ    GPIO_BUTTON_PIN, 21
.equ    INITIAL_STATE, 0
.equ    GPIO_BTN_DN_MSK, 0x00400000
.equ    GPIO_BTN_UP_MSK, 0x00800000
.equ    INPUT_END_TIME, 2500000

main_asm:
    push     {lr}
    bl       init_pin
    bl       install_alrm_isr
    bl       install_gpio_isr
    ldr      r0, =wlcm_msg
    bl       printf
    ldr      r0, =inpt_msg
    bl       printf
    wfi

loop:
    bl      set_alarm
    wfi
    ldr     r2, =atimer
    ldr     r1, [r2]
    movs    r0, #1
    cmp     r0, r1
    bne     loop
    movs    r0, #0
    str     r0, [r2]
    movs    r0, #3
    movs    r1, #0
    bl      add_input
    pop     {pc}

init_pin:
    push    {lr}
    movs    r0, #GPIO_BUTTON_PIN
    bl      asm_gpio_init
    movs    r0, #GPIO_BUTTON_PIN
    bl      asm_gpio_set_irq
    pop     {pc}

set_alarm:
    ldr     r2,=TIMER_BASE
    movs    r1, #1
    str     r1, [r2, #TIMER_INTE_OFFSET]
    ldr     r1, [r2, #TIMER_TIMELR_OFFSET]
    ldr     r3, =ltimer
    ldr     r0, [r3]
    add     r1, r0
    str     r1, [r2, #TIMER_ALARM0_OFFSET]
    bx      lr

install_alrm_isr:
    ldr     r2, =(PPB_BASE + M0PLUS_VTOR_OFFSET)
    ldr     r1, [r2]
    movs    r2, #ALRM_ISR_OFFSET
    add     r2, r1
    ldr     r0, =alrm_isr
    str     r0, [r2]
    movs    r0, #1
    ldr     r1, =(PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)
    str     r0, [r1]
    ldr     r1, =(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)
    str     r0, [r1]
    bx      lr

.thumb_func
alrm_isr:
    push    {lr}
    ldr     r2, =TIMER_BASE
    movs    r1, #1
    str     r1, [r2, #TIMER_INTR_OFFSET]
    ldr     r2, =atimer
    movs    r0, #1
    str     r0, [r2]
    pop     {pc}

install_gpio_isr:
ldr     r2, =(PPB_BASE + M0PLUS_VTOR_OFFSET)
ldr     r1, [r2]
movs    r2, #GPIO_ISR_OFFSET
add     r2, r1
ldr     r0, =gpio_isr
str     r0, [r2]
ldr     r0, =8192
ldr     r2, =(PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)
str     r0, [r2]
ldr     r2, =(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)
str     r0, [r2]
bx      lr

.thumb_func
gpio_isr:
    push    {lr}
    bl      watchdog_update
    ldr     r2, =(IO_BANK0_BASE + IO_BANK0_PROC0_INTS2_OFFSET)
    ldr     r1, [r2]
    ldr     r0, =GPIO_BTN_DN_MSK
    cmp     r0, r1
    beq     fall
    ldr     r0, =GPIO_BTN_UP_MSK
    cmp     r0, r1
    beq     rise

gpio_isr_done:
    bl      start_timer
    pop     {pc}

fall:
    ldr     r2, =(IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET)
    ldr     r1, =GPIO_BTN_DN_MSK
    str     r1, [r2]
    bl      end_timer
    movs    r1, #1
    cmp     r0, r1
    bgt     space
    b       dot

rise:
    ldr     r2, =(IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET)
    ldr     r1, =GPIO_BTN_UP_MSK
    str     r1, [r2]
    bl      end_timer
    movs    r1, #1
    cmp     r0, r1
    bgt     dash
    b       gpio_isr_done

space:
    movs    r0, #2
    movs    r1, #0
    bl      add_input
    b       dot

dot:
    movs    r0, #0
    movs    r1, #0
    bl      add_input
    b       gpio_isr_done

dash:
    movs    r0, #1
    movs    r1, #1
    bl      add_input
    b       gpio_isr_done

.align 4
wlcm_msg:    .asciz "\n\t Press the GPIO 21 button to input your code\n"
inpt_msg:    .asciz "\n\t Input: "

.data
    .align 4
    ltimer:         .word   INPUT_END_TIME
    atimer:         .word   INITIAL_STATE
