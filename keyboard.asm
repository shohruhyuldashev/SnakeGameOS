[BITS 64]
global read_scancode

read_scancode:
    in al, 0x60
    ret
