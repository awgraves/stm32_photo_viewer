ENTRY(Reset_Handler)

/* RM0390 pgs. 60 & 65   */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
  SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K 
}

_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{ 
  .text : {
    . = ALIGN(4);
    *(.vtable)
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH
  
  _la_data = LOADADDR(.data);
  .data : {
    . = ALIGN(4);
    _sdata = .;
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > SRAM AT > FLASH

  .bss : {
    . = ALIGN(4);
    _sbss = .;
    *(.bss*)
    . = ALIGN(4);
    _ebss = .;
  } >SRAM
}

