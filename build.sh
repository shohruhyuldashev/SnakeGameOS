#!/bin/bash
set -e

ISO_NAME="CyberBro_Snake64OS_v4.iso"

echo "[*] Assembling bootloader (64-bit)..."
nasm -f elf64 boot.asm -o boot.o

echo "[*] Assembling keyboard I/O..."
nasm -f elf64 keyboard.asm -o keyboard.o

echo "[*] Assembling ISR (timer interrupt)..."
nasm -f elf64 isr.asm -o isr.o

echo "[*] Compiling IDT..."
x86_64-linux-gnu-gcc -c idt.c \
    -ffreestanding \
    -mno-red-zone \
    -m64 \
    -nostdlib \
    -o idt.o

echo "[*] Compiling ACPI..."
x86_64-linux-gnu-gcc -c acpi.c \
    -ffreestanding \
    -mno-red-zone \
    -m64 \
    -nostdlib \
    -o acpi.o

echo "[*] Compiling kernel..."
x86_64-linux-gnu-gcc -c kernel.c \
    -ffreestanding \
    -mno-red-zone \
    -m64 \
    -nostdlib \
    -o kernel.o

echo "[*] Linking kernel..."
x86_64-linux-gnu-ld -T linker.ld \
    boot.o kernel.o keyboard.o idt.o isr.o acpi.o \
    -o kernel.bin

echo "[*] Preparing ISO directory..."
rm -rf iso
mkdir -p iso/boot/grub

cp kernel.bin iso/boot/kernel.bin
cp grub.cfg iso/boot/grub/grub.cfg

echo "[*] Building Universal BIOS + UEFI ISO..."
grub-mkrescue \
    -o "$ISO_NAME" \
    --modules="multiboot2 normal part_msdos part_gpt fat ext2 iso9660" \
    iso >/dev/null 2>&1

echo "[+] DONE! Universal ISO created -> $ISO_NAME"
