#ifndef ACPI_H
#define ACPI_H
#include <stdint.h>

struct RSDP {
    char signature[8];      // "RSD PTR "
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdt_addr;
} __attribute__((packed));

struct SDTHeader {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemid[6];
    char oemtableid[8];
    uint32_t oemrevision;
    uint32_t creatorid;
    uint32_t creatorrev;
} __attribute__((packed));



struct FADT {
    struct SDTHeader h;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t  reserved;
    uint8_t  preferred_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t  acpi_enable;
    uint8_t  acpi_disable;
    uint32_t pm1a_cnt_blk;   // PM1a control port
    uint32_t pm1b_cnt_blk;   // PM1b control port (optional)
} __attribute__((packed));




extern struct FADT* fadt;
void acpi_init();



#endif
