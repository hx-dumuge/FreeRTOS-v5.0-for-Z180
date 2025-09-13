;;; 编写者：dumuge
;;; 编写时间：2024-06-20
;;; 版本：1.00
;;; 功能：Z180的boot

	.include "z180.inc"

	;; module crt0
	.module crt0

	;; ENABLE Z180 ASSEMBLER
	.z180
	
	;; Public variables in this module
	.globl	_main
	.globl	_int1_isr
	.globl	_int2_isr
	.globl	_prt0_isr
	.globl	_prt1_isr
	.globl	_dma0_isr
	.globl	_dma1_isr
	.globl	_csio_isr
	.globl	_asci0_isr
	.globl	_asci1_isr

	;; variables params
	.equ	STACK_TOP,	0xFFFF
	.equ	IVT_BASE,	0x0100

	;; absolute external ram data
	.area	_HEADER (ABS)

	;; Reset vector
	.org 	0x0000
	di
	jp	init

	;; int enter
	.org	0x08
	; ei
	reti
	.org	0x10
	; ei
	reti
	.org	0x18
	; ei
	reti
	.org	0x20
	; ei
	reti
	.org	0x28
	; ei
	reti
	.org	0x30
	; ei
	reti

	;; int0 posion
	.org	0x38
	; ei
	reti

	;; interrupt vector
	;; 中断向量表放在0x100
	.area	_IVT (ABS)
	.org	IVT_BASE
	.dw	_int1_isr
	.dw	_int2_isr
	.dw	_prt0_isr
	.dw	_prt1_isr
	.dw	_dma0_isr
	.dw	_dma1_isr
	.dw	_csio_isr
	.dw	_asci0_isr
	.dw	_asci1_isr
	.rept	7
		.dw	dummy_isr
	.endm
	

	;; BOOT INIT
	.area	_BOOT (ABS)
	.org	0x0200
init:
	
;;; default power on
;;; CBAR = $F0
;;; CBR  = $00
;;; BBR  = $00
;;; $F000-$FFFF = ( $F000 - $FFFF )  4KB - Common Area 1
;;; $0000-$EFFF = ( $0000 - $EFFF ) 60KB - Bank Area
;;; $0000-$0000 = ( $0000 - $0000 )  0KB - Common Area 0
;;; CBAR = $F0
;;; CBR  = $10
;;; BBR  = $00
;;; $F000-$FFFF = ( $1F000 - $1FFFF )  4KB - Common Area 1
;;; $0000-$EFFF = ( $00000 - $0EFFF ) 60KB - Bank Area
;;; $0000-$0000 = ( $00000 - $00000 ) 0KB - Common Area 0
	; ld      a,#0xF0
	; out0    (CBAR), a
	; ld      a, #0x10
	; out0    (CBR), a
	;; 这里是内存块切换用的，和硬件挂钩
	ld      a,#0x40
	out0    (CBAR), a
	ld      a, #0x80
	out0    (CBR), a

	;; init DMA/WAIT Control Register (DCNTL: 32H)
	ld      a, #0x40
	out0    (DCNTL), a

	;; init Refresh Control Register (RCR: 36H)
	ld      a, #0x7F
	out0    (RCR), a

	;; init sp
	ld      sp, #0x0FFFF

	;; init ASCI Control Register A
	;; 18.4320Mhz 9600bps
	in0       a, (CNTLA0)
	; ld       a, #0x6c
	ld        a, #0x64
	OUT0      (CNTLA0), a
	; ld        a, #0x0f
	ld        a, #0x21
	OUT0      (CNTLB0), a

	;; INT 入口 0100
	;; INT 方式2
	ld		hl, #IVT_BASE
	ld		a, h
	ld		i, a
	ld		a, l
	out0    (IL), a
	im      2


	;; print Test CPU OK !
	ld hl, #testCPUok
pLoopCPUok:
    ld a, (hl)
    or a
    jr z, pCPUokend
    ld e, (hl)
    inc hl
pWait_CPUok:
	in0 a, (STAT0)
	and #0x02
	jr z, pWait_CPUok
	ld  a, e
	OUT0 (TDR0), a
    jr pLoopCPUok
pCPUokend:
	nop

	;jp pLoopRAMend


	;; Test RAM !
	;; RAM ADDR START 0x4000-0x6000
	; reg bc: number of bytes to test
	; reg hl: start address of test
	ld bc, #0xFFFF-0x4000
	ld hl, #0x4000
ramtstloop:
ramtst00:
    ld e,#0x0
    ld (hl),e
    ld a,(hl)
    cp e
    jp z,ramtstff
    jp pLoopRAMerr
ramtstff:
    ld e,#0xFF
    ld (hl),e
    ld a,(hl)
    cp e
    jp z,ramtstnext
    jp pLoopRAMerr
ramtstnext:
    inc hl
    dec bc
    ld a,b
    or c
    jr nz, ramtstloop
	jp ramtestend
pRAMErr:
	;; print Test RAM FAIL !
	ld hl, #testRAMerr
pLoopRAMerr:
    ld a, (hl)
    or a
    jr z, pRAMerrend
    ld e, (hl)
    inc hl
pWait_RAMerr:
	in0 a, (STAT0)
	and #0x02
	jr z, pWait_RAMerr
	ld  a, e
	OUT0 (TDR0), a
    jr pLoopRAMerr
pRAMerrend:
	jp	endhalt
ramtestend:
	;; print Test RAM ok !
	ld hl, #testRAMok
pLoopRAMok:
    ld a, (hl)
    or a
    jr z, pRAMokend
    ld e, (hl)
    inc hl
pWait_RAMok:
	in0 a, (STAT0)
	and #0x02
	jr z, pWait_RAMok
	ld  a, e
	OUT0 (TDR0), a
    jr pLoopRAMok
pRAMokend:
	nop

	;; 初始化全局变量后进main函数
	or	a, a
	call	gsinit
	call	_main
	jp	exit


	;; 其他函数
exit::
	ld	a,#0
	rst	0x08

endhalt::
	halt
	jp	endhalt

dummy_isr::
	; ei
	reti

	;; func putASCIS
	;; 之所有没用这个打印，因为在没验证RAM区域前不能用栈
putASCIA::
	in0 a, (STAT0)
	and #0x02
	jr z, putASCIA
	ld a, e
	OUT0 (TDR0), a
	ret


	;; 排列一下ROM区域的顺序
	.area	_HOME
	.area	_CODE
	.area	_TEXT
	.area	_INITIALIZER
	.area   _GSINIT
	.area   _GSFINAL
	;; 下面的在RAM里
	.area	_DATA
	.area	_INITIALIZED
	.area	_BSEG
	.area   _BSS
	.area   _HEAP


	;; 文本区域，放些字符串
	.area	_TEXT
	.STR "dumuge"
testCPUok:
    .STR "Test CPU OK."
	.DB 0x0A
    .DB 0
testRAMok:
    .STR "Test RAM OK."
	.DB 0x0A
    .DB 0
testRAMerr:
    .STR "Test RAM FAIL !"
	.DB 0x0A
    .DB 0


	;; 全局变量到内存的拷贝
	.area   _GSINIT
gsinit::
	ld	bc, #l__INITIALIZER
	ld	a, b
	or	a, c
	jr	Z, gsinit_next
	ld	de, #s__INITIALIZED
	ld	hl, #s__INITIALIZER
	ldir
gsinit_next:

	.area   _GSFINAL
	ret
