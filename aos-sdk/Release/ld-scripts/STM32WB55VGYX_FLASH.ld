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
 The AN 5289 (chapter 4.2) indicates what to do. Unfortunately it is said that
 the size is variable for FUSE and BLE FW. Fortunately, the space is at the end
 but before the secure part pointed by the famous SFSA. The AN also indicates:
 The user application must take into account that the available memory may vary
 between different versions of the RF stack. The available space for the user
 application can be obtained from the release notes for STM32WB coprocessor 
 wireless binaries. The install address for the RF stack is also the boundary
 address for the user Flash memory area. 0x080EC000 FUS 1.2.0 0x080C7000 BLE stack full
 v1.13.0 It is recommended to let space for future update.
 
 Since we need to support OTA firmware updates, we need to set aside half of the
 application flash space for uploading new code. Furthermore, we need a bootloader
 to move that second bank into the first one after due validation, and apparently
 the LoRaWAN FUOTA 'smart delta' feature needs a swap area to reconstruct chunks
 of firmware. That area has been sized at 46kB in Lionel's flash partitioning doc,
 to keep flash page alignment we increase it to 48kB (maybe this part could be in
 ram ?).

 We also need a certain amount of flash for EEPROM emulation. For a device life of
 10 years, about 40kB are needed given the planned update frequency (20mn minimum).
 These pages need to be at a fixed address to preserve device and LoRaWAN MAC network
 parameters across firmware updates.
 
 NOTE: If the eeprom emulation flash area is changed, any devices already in the field
       will need to be re-provisioned on the network or incur many failed join attempts
       due to the lost DevNonce parameter.
 
 Any flash areas that may be re-programmed (EEPROM, SWAP and app/dfu banks) need to be
 page aligned. A flash page is 4kB.
 
 Given all these constraints, the remaining application flash size is 316kB.
 
 Summarizing all the above, this is the planned flash layout as of Apr 15 2022:
 
	 08000000 08010000  64k ST SBSFU bootloader
	 08010000 0805f000 316k (*)Application flash
	 0805f000 080ae000 316k (*)FUOTA Bank to reconstruct new app image
	 080ae000 080ba000  48k (*)FUOTA SWAP area to assemble deltas
	 080ba000 080c4000  40k (*)EEPROM emulation storage for 508 32-bit variables
	 080c4000 080c7000  12k Reserved in case the BLE code size increases
	 080c7000 080ec000 148k BLE full stack (fixed address for BLE V1.13.0)
	 080ec000 08100000  80k BLE FUS (fixed address for FUS V1.2.0)

 (*) This may change slightly if for example we reduce eeprom update interval,
     or move the fuota swap area to ram.
*/

_eeprom_size = 40k; /* Must be sized to fit PAGES_NUMBER * FLASH_PAGE_SIZE in eeprom_emul.h ! */

/* Specify the memory areas */
MEMORY
{
FLASH (rx)                 : ORIGIN = 0x08000000, LENGTH = 316k+316k		/* XXX needs to be split into APP and DFU areas */
APP_BANK                   : org = ORIGIN(FLASH), len = 316k 				/* Application bank so the IDE displays how much is used */
DFU_BANK                   : org = ORIGIN(APP_BANK)+LENGTH(APP_BANK), len = LENGTH(APP_BANK)	/* DFU bank, same size as application bank */
SBSFU (rx)                 : ORIGIN = ORIGIN(FLASH)+LENGTH(FLASH), LENGTH = 64k /* XXX needs to be moved to address 0x08000000 */
DFU_SWAP                   : ORIGIN = ORIGIN(SBSFU)+LENGTH(SBSFU), LENGTH = 48k /* XXX FUOTA SWAP space - check this is needed */
EEPROM (r)                 : ORIGIN = ORIGIN(DFU_SWAP)+LENGTH(DFU_SWAP), LENGTH = _eeprom_size 
BLE_RFU                    : ORIGIN = ORIGIN(EEPROM)+LENGTH(EEPROM), LENGTH = 0x080c7000-ORIGIN(BLE_RFU) 	/*  12k Possible ST BLE Stack size increase */
BLE_STK                    : ORIGIN = 0x080c7000, LENGTH = 0x080ec000-ORIGIN(BLE_STK)	/* 148k BLE full stack, Version V1.13.0 @0x080C7000 */
BLE_FUS                    : ORIGIN = 0x080ec000, LENGTH = 0x08100000-ORIGIN(BLE_FUS)	/*  80k FUS, Version 1.2.0 @0x080EC000 */
RAM1 (xrw)                 : ORIGIN = 0x20000004, LENGTH = 0x2FFFC
RAM_SHARED (xrw)           : ORIGIN = 0x20030000, LENGTH = 10K
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
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


 .commands : {
    . = ALIGN(4);
    __cli_command_table = .;
    KEEP(*(SORT_BY_NAME(.commands.*)));
    LONG (0)	/* end of table */
  } > FLASH

    _etext = .;        /* define a global symbols at end of code */

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  /* EEPROM Emulation goes to a special memory region in flash */
  .eeprom (NOLOAD) : {
  	_eeprom_pages = .;
  	. = . + _eeprom_size;
  	_eeprom_pages_end = .;
  } > EEPROM
  
  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
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

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM1

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
