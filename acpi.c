#include "acpi.h"
#include "io.h"

struct FADT* fadt = 0;

// RSDP qidirish BIOS memoryda

static struct RSDP* find_rsdp(){
    for(uint64_t addr = 0x000E0000; addr < 0x00100000; addr += 16){
        char *sig = (char*)(uintptr_t)addr;
        if(sig[0]=='R' && sig[1]=='S' && sig[2]=='D' && sig[3]==' ' &&
           sig[4]=='P' && sig[5]=='T' && sig[6]=='R' && sig[7]==' '){
            return (struct RSDP*)(uintptr_t)addr;
        }
    }
    return 0;
}




void acpi_init(){
    struct RSDP* rsdp = find_rsdp();
    if(!rsdp) return;

    struct SDTHeader* rsdt = (struct SDTHeader*)(uint64_t)rsdp->rsdt_addr;
    uint32_t entries = (rsdt->length - sizeof(struct SDTHeader)) / 4;
    uint32_t* table_ptrs = (uint32_t*)((uint64_t)rsdt + sizeof(struct SDTHeader));

    for(uint32_t i=0;i<entries;i++){
        struct SDTHeader* h = (struct SDTHeader*)(uint64_t)table_ptrs[i];
        if(h->signature[0]=='F' && h->signature[1]=='A' &&
           h->signature[2]=='C' && h->signature[3]=='P'){
            fadt = (struct FADT*)h;
            return;
        }
    }
}
