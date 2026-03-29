OUTPUT_FORMAT(elf32-littlearc)

MEMORY {
    RAM (rwx)  : ORIGIN = 0x3C800000, LENGTH = 7M /* SD */
    /* TODO: Check actual size of static area */
    /* RAM (rwx) : ORIGIN = 0x3C000300, LENGTH = 3K */
}

/* ENTRY(entry) */

SECTIONS {
    .text (ORIGIN(RAM) + 16K) : {
        *(.text)
        *(.text.*)

        TEXT_END = .;
    }

    .rodata (TEXT_END) : {
        *(.rodata)
        *(.rodata.*)

        RODATA_END = .;
    }

    .data (RODATA_END) : {
        *(.data)
        *(.data.*)

        DATA_END = .;
    }

    .bss (DATA_END) : {
        __start_BSS = .;

        *(.bss)
        *(.bss.*)

        __stop_BSS = .;
    }
}
