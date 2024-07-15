/*
******************************************************************************
**
**  File        : LinkerScript.ld
**
**  Author      : STM32CubeIDE
**
**  Abstract    : Linker script for STM32WB55xG Device
**                      1024Kbytes FLASH
**                      256Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed as is without any warranty
**                of any kind.
**
*****************************************************************************
** @attention
**
** Copyright (c) 2021 STMicroelectronics.
** All rights reserved.
**
** This software is licensed under terms that can be found in the LICENSE file
** in the root directory of this software component.
** If no LICENSE file comes with this software, it is provided AS-IS.
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM1) + LENGTH(RAM1);    /* end of RAM1 */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x2000 ;  /* required amount of heap  */
_Min_Stack_Size = 0x4000 ; /* required amount of stack */


/* 
 Any flash areas that may be re-programmed (SWAP, config and app/dfu banks) need to be
 page aligned. A flash page is 4kB.
 
 Given all these constraints, the remaining application flash size is 336kB.
 
 The layout is:
 	 Base address
	 08000000  		20k   Abeeway bootloader
	 08005000 	 	4k    Application parameter
	 08006000  		696k  Application flash
	 080B4000   	12k Reserved in case the BLE code size increases
	 080c7000  		148k BLE full stack (fixed address for BLE V1.13.0)
	 080ec000   	80k BLE FUS (fixed address for FUS V1.2.0)

 (*) This may change slightly if for example we reduce eeprom update interval,
     or move the fuota swap area to ram.
*/


/* Specify the memory areas */
MEMORY
{
BOOTLOADER (r)             : ORIGIN = 0x08000000, LENGTH = 20k	/*Abeeway bootloader (BL): 20k BL in release, 40k BL in debug */
PARAM (rx)                 : ORIGIN = ORIGIN(BOOTLOADER)+LENGTH(BOOTLOADER), LENGTH = 4k	/* Parameter */
FLASH (rx)                 : ORIGIN = ORIGIN(PARAM)+LENGTH(PARAM), LENGTH = 696k		/* Application flash */
BLE_RFU                    : ORIGIN = ORIGIN(FLASH)+LENGTH(FLASH), LENGTH = 12k 	/*  12k Possible ST BLE Stack size increase */
BLE_STK                    : ORIGIN = 0x080c7000, LENGTH = 0x080ec000-ORIGIN(BLE_STK)	/* 148k BLE full stack, Version V1.13.0 @0x080C7000 */
BLE_FUS                    : ORIGIN = 0x080ec000, LENGTH = 0x08100000-ORIGIN(BLE_FUS)	/*  80k FUS, Version 1.2.0 @0x080EC000 */
RAM1 (xrw)                 : ORIGIN = 0x20000004, LENGTH = 0x2FFFC
RAM_SHARED (xrw)           : ORIGIN = 0x20030000, LENGTH = 10K
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector (READONLY) :
  {
    . = ALIGN(4);
  
     PROVIDE ( __isr_vector__ = . );
    
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text (READONLY) :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
  } >FLASH


 .commands (READONLY) : {
    . = ALIGN(4);
    __cli_command_table = .;
    KEEP(*(SORT_BY_NAME(.commands.*)));
    LONG (0)	/* end of table */
  } > FLASH

    _etext = .;        /* define a global symbols at end of code */

  /* Constant data goes into FLASH */
  .rodata (READONLY) :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH
  
  .ARM.extab (NOLOAD) : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM (READONLY) : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array   (READONLY) :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array (READONLY) :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array (READONLY) :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data (READONLY) :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM1 AT> FLASH

 .config_file (READONLY) :
 {
   PROVIDE(__configuration_file_start = .);
 } > FLASH

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss (NOLOAD):
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    *(BLE_DRIVER_CONTEXT)
    *(SYSTEM_DRIVER_CONTEXT)

    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM1

  
.noinit (NOLOAD):
  {
   . = ALIGN(4);
    PROVIDE(__noinit_start__ = .);
   	*(.noinit)
    PROVIDE(__noinit_end__ = .);	
  } > RAM1

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM1

  /* Application param */
  .user_param (READONLY) :
  {
  	PROVIDE(__user_param_flash_base_addr = .);
  	PROVIDE(__user_param_flash_size = LENGTH(PARAM));
  } >PARAM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0       : { *(.ARM.attributes) }
   MAPPING_TABLE (NOLOAD) : { *(MAPPING_TABLE) } >RAM_SHARED
   MB_MEM1 (NOLOAD)       : { *(MB_MEM1) } >RAM_SHARED
   MB_MEM2 (NOLOAD)       : { _sMB_MEM2 = . ; *(MB_MEM2) ; _eMB_MEM2 = . ; } >RAM_SHARED
}
