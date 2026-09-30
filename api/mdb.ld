ENTRY(_start)

SECTIONS
{
    . = 0x0;
    .mdb_header : {
        LONG(_dram_size);
        LONG(_iram_size);
        LONG(_start);
        LONG(_got_file_offset);
    }
    _iram_start = .;
    .text : ALIGN(4) SUBALIGN(4) {
        KEEP(*(.text._start))

        _literal_start = .;
        *(.literal .literal.*)
        _literal_end = .;
        . = ALIGN(4);

        *(.text .text.*)
        . = ALIGN(4);
    }
    . = ALIGN(4);
    _iram_end = .;
    _iram_size = _iram_end - _iram_start;

    _dram_start = .;
    .rodata : ALIGN(4) SUBALIGN(4) {
        KEEP(*(.rodata .rodata.*))
        . = ALIGN(4);
    }
    .data : ALIGN(4) SUBALIGN(4) {
        KEEP(*(.data .data.*))
        . = ALIGN(4);
    }
    . = ALIGN(4);
    _got_file_offset = .;
    .got : ALIGN(4) SUBALIGN(4) {
        _got_start = .;
        KEEP(*(.got .got.plt))
        . = ALIGN(4);
        _got_end = .;
    }
    . = ALIGN(4);
    _dram_end = .;
    _dram_size = _dram_end - _dram_start;
}
