BOOTLOADER_SIZE=0x1000
  
  .globl __start__stack

  .area	SSEG	(DATA)

__start__stack:
	.ds	1

 	.area VECTOR    (CODE)
	.globl __interrupt_vect
__interrupt_vect:
    ljmp __sdcc_gsinit_startup

    ljmp #(BOOTLOADER_SIZE + 0x0003)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x000B)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x0013)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x001B)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x0023)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x002B)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x0033)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x003B)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x0043)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x004B)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x0053)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x005B)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x0063)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x006B)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x0073)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x007B)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x0083)
    .ds     5

    ljmp #(BOOTLOADER_SIZE + 0x008B)

    .globl __start__stack
    .area GSINIT0 (CODE)

__sdcc_gsinit_startup:
    mov     sp,#__start__stack - 1

    .area GSFINAL (CODE)
    ljmp    _bootloader_main