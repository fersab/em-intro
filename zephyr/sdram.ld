/*
 * Place .sdram_bss section into SDRAM2 (NOLOAD).
 *
 * NOLOAD because the MEMC/FMC driver initializes SDRAM at POST_KERNEL,
 * which is after the C runtime zeroes BSS. All arrays placed here are
 * explicitly initialized by app code before use (framebuffers, particles).
 */
SECTION_DATA_PROLOGUE(.sdram_bss, (NOLOAD),)
{
    *(.sdram_bss)
    *(.sdram_bss.*)
} GROUP_DATA_LINK_IN(SDRAM2, SDRAM2)
