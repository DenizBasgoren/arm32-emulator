/* memmap */
MEMORY
{
    rom(RX)   : ORIGIN = 0x00000000, LENGTH = 0x00100000
    ram(WAIL) : ORIGIN = 0x20000000, LENGTH = 0x00100000
}

SECTIONS
{
    .text : { *(.text*) *(.rodata*)} > rom
    .data : { *(.data*) *(.bss*) } > ram
}

