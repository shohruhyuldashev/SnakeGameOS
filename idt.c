#include "idt.h"
#include "io.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void timer_isr();

void idt_set_gate(int n, unsigned long handler){
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].selector = 0x08;
    idt[n].ist = 0;
    idt[n].type_attr = 0x8E;
    idt[n].offset_mid = (handler >> 16) & 0xFFFF;
    idt[n].offset_high = (handler >> 32);
    idt[n].zero = 0;
}

static void pic_remap(){
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask all IRQs except timer (IRQ0)
    outb(0x21, 0xFE); 
    outb(0xA1, 0xFF);
}

void idt_init(){
    idtp.limit = sizeof(idt)-1;
    idtp.base = (unsigned long)&idt;

    pic_remap();

    idt_set_gate(32, (unsigned long)timer_isr);

    __asm__ volatile("lidt %0" : : "m"(idtp));
    __asm__ volatile("sti");
}
