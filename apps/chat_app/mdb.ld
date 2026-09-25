ENTRY(_start)

SECTIONS
{
    . = 0x0;
    
    /* 1. Header Metadata Section */
    .mdb_header : {
        LONG(_dram_size);       
        LONG(_iram_size);       
        LONG(_start);           
        LONG(_got_file_offset); 
    }

    /* ZONE 1: CODE + INLINE LITERALS (Mapped strictly to Executable IRAM) */
    _iram_start = .;
    .text : ALIGN(4) { 
        KEEP(*(.text._start))
        
        /* 🛠️ THE PERMANENT FIX: Interleave literal pools inside the text section.
           This ensures L32R instructions can safely look backwards over tiny distances. */
        *(.literal .text .literal.* .text.*)
    }
    . = ALIGN(4);
    _iram_end = .;
    _iram_size = _iram_end - _iram_start;

    /* ZONE 2: VARIABLES & DATA STRINGS ONLY (Mapped strictly to Data DRAM) */
    _dram_start = .;
    
    /* Removed individual .literal block from here */
    
    .rodata : ALIGN(4) { 
        KEEP(*(.rodata .rodata.*)) 
        . = ALIGN(4); 
    }
    .data : ALIGN(4) { 
        KEEP(*(.data .data.*)) 
        . = ALIGN(4); 
    }
    
    _got_file_offset = .; 
    .got : ALIGN(4) {
        _got_start = .;
        KEEP(*(.got .got.plt))
        _got_end = .;
    }
    . = ALIGN(4);
    _dram_end = .;
    _dram_size = _dram_end - _dram_start;
}
