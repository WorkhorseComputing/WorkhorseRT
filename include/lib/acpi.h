/** MIT License
 *
 * Copyright (c) 2026 Humza Khan
 * <mohammed.khan.2024@uni.strath.ac.uk>
 * <https://github.com/humzak711>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef _ACPI_H_
#define _ACPI_H_

#include <compiler.h>

#define ACPI_SIG32(sig)             \
    ( ((uint32_t)(sig)[0])        | \
      ((uint32_t)(sig)[1] << 8)   | \
      ((uint32_t)(sig)[2] << 16)  | \
      ((uint32_t)(sig)[3] << 24) )

#define ACPI_RSDP_SIGNATURE      "RSD PTR "
#define ACPI_RSDT_SIGNATURE      "RSDT"
#define ACPI_XSDT_SIGNATURE      "XSDT"
#define ACPI_MADT_SIGNATURE      "APIC"
#define ACPI_FADT_SIGNATURE      "FACP"
#define ACPI_FACS_SIGNATURE      "FACS"
#define ACPI_MCFG_SIGNATURE      "MCFG"
#define ACPI_HPET_SIGNATURE      "HPET"
#define ACPI_SRAT_SIGNATURE      "SRAT"
#define ACPI_SLIT_SIGNATURE      "SLIT"
#define ACPI_DSDT_SIGNATURE      "DSDT"
#define ACPI_SSDT_SIGNATURE      "SSDT"
#define ACPI_PSDT_SIGNATURE      "PSDT"
#define ACPI_ECDT_SIGNATURE      "ECDT"
#define ACPI_RHCT_SIGNATURE      "RHCT"
#define ACPI_DMAR_SIGNATURE      "DMAR"

#define ACPI_RSDT_SIGNATURE_UINT32  0x54445352U
#define ACPI_XSDT_SIGNATURE_UINT32  0x54445358U 
#define ACPI_MADT_SIGNATURE_UINT32  0x43495041U
#define ACPI_FADT_SIGNATURE_UINT32  0x50434146U 
#define ACPI_FACS_SIGNATURE_UINT32  0x53434146U
#define ACPI_MCFG_SIGNATURE_UINT32  0x4746434DU
#define ACPI_HPET_SIGNATURE_UINT32  0x54455048U
#define ACPI_SRAT_SIGNATURE_UINT32  0x54415253U
#define ACPI_SLIT_SIGNATURE_UINT32  0x54494C53U
#define ACPI_DSDT_SIGNATURE_UINT32  0x54445344U 
#define ACPI_SSDT_SIGNATURE_UINT32  0x54445353U
#define ACPI_PSDT_SIGNATURE_UINT32  0x54445350U
#define ACPI_ECDT_SIGNATURE_UINT32  0x54444345U
#define ACPI_RHCT_SIGNATURE_UINT32  0x54434852U
#define ACPI_DMAR_SIGNATURE_UINT32  0x52414D44U


#define ACPI_AS_ID_SYS_MEM       0x00
#define ACPI_AS_ID_SYS_IO        0x01
#define ACPI_AS_ID_PCI_CFG_SPACE 0x02
#define ACPI_AS_ID_EC            0x03
#define ACPI_AS_ID_SMBUS         0x04
#define ACPI_AS_ID_SYS_CMOS      0x05
#define ACPI_AS_ID_PCI_BAR_TGT   0x06
#define ACPI_AS_ID_IPMI          0x07
#define ACPI_AS_ID_GP_IO         0x08
#define ACPI_AS_ID_GENERIC_SBUS  0x09
#define ACPI_AS_ID_PCC           0x0A
#define ACPI_AS_ID_FFH           0x7F
#define ACPI_AS_ID_OEM_BASE      0xC0
#define ACPI_AS_ID_OEM_END       0xFF

#define ACPI_ACCESS_UD    0
#define ACPI_ACCESS_BYTE  1
#define ACPI_ACCESS_WORD  2
#define ACPI_ACCESS_DWORD 3
#define ACPI_ACCESS_QWORD 4

typedef struct ATTR_PACKED acpiRsdp 
{
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdtAddr;

    // vvvv available if .revision >= 2.0 only
    uint32_t length;
    uint64_t xsdtAddr;
    uint8_t extendedChecksum;
    uint8_t rsvd[3];
} acpiRsdp_t;
SIZE_ASSERT(acpiRsdp_t, 36);

typedef struct ATTR_PACKED acpiSdtHdr
{
    char     signature[4];
    uint32_t length;
    uint8_t  revision;
    uint8_t  checksum;
    char     oemid[6];
    char     oemTableId[8];
    uint32_t oemRevision;
    uint32_t creatorId;
    uint32_t creatorRevision;
} acpiSdtHdr_t;
SIZE_ASSERT(acpiSdtHdr_t, 36);

typedef struct ATTR_PACKED acpiRsdt
{
    acpiSdtHdr_t hdr;
    uint32_t     entries[];
} acpiRsdt_t;

typedef struct ATTR_PACKED acpiEntryHdr
{
    uint8_t type;
    uint8_t length;
} acpiEntryHdr_t;

#define ACPI_MADT_ENTRY_TYPE_LAPIC                          0x00
#define ACPI_MADT_ENTRY_TYPE_IOAPIC                         0x01
#define ACPI_MADT_ENTRY_TYPE_INTERRUPT_SOURCE_OVERRIDE      0x02
#define ACPI_MADT_ENTRY_TYPE_NMI_SOURCE                     0x03
#define ACPI_MADT_ENTRY_TYPE_LAPIC_NMI                      0x04
#define ACPI_MADT_ENTRY_TYPE_LAPIC_ADDRESS_OVERRIDE         0x05
#define ACPI_MADT_ENTRY_TYPE_IOSAPIC                        0x06
#define ACPI_MADT_ENTRY_TYPE_LSAPIC                         0x07
#define ACPI_MADT_ENTRY_TYPE_PLATFORM_INTERRUPT_SOURCES     0x08
#define ACPI_MADT_ENTRY_TYPE_LOCAL_X2APIC                   0x09
#define ACPI_MADT_ENTRY_TYPE_LOCAL_X2APIC_NMI               0x0A
#define ACPI_MADT_ENTRY_TYPE_GICC                           0x0B
#define ACPI_MADT_ENTRY_TYPE_GICD                           0x0C
#define ACPI_MADT_ENTRY_TYPE_GIC_MSI_FRAME                  0x0D
#define ACPI_MADT_ENTRY_TYPE_GICR                           0x0E
#define ACPI_MADT_ENTRY_TYPE_GIC_ITS                        0x0F
#define ACPI_MADT_ENTRY_TYPE_MULTIPROCESSOR_WAKEUP          0x10
#define ACPI_MADT_ENTRY_TYPE_CORE_PIC                       0x11
#define ACPI_MADT_ENTRY_TYPE_LIO_PIC                        0x12
#define ACPI_MADT_ENTRY_TYPE_HT_PIC                         0x13
#define ACPI_MADT_ENTRY_TYPE_EIO_PIC                        0x14
#define ACPI_MADT_ENTRY_TYPE_MSI_PIC                        0x15
#define ACPI_MADT_ENTRY_TYPE_BIO_PIC                        0x16
#define ACPI_MADT_ENTRY_TYPE_LPC_PIC                        0x17
#define ACPI_MADT_ENTRY_TYPE_RINTC                          0x18
#define ACPI_MADT_ENTRY_TYPE_IMSIC                          0x19
#define ACPI_MADT_ENTRY_TYPE_APLIC                          0x1A
#define ACPI_MADT_ENTRY_TYPE_PLIC                           0x1B
#define ACPI_MADT_ENTRY_TYPE_RESERVED                       0x1C
#define ACPI_MADT_ENTRY_TYPE_OEM                            0x80

#define ACPI_MADT_LAPIC_FLAGS_ENABLED_MASK                  (1 << 0)
#define ACPI_MADT_LAPIC_FLAGS_ONLINE_CAP_MASK               (1 << 1)

#define ACPI_MADT_POLARITY_MASK                             0b11
#define ACPI_MADT_POLARITY_CONFORMING                       0b00
#define ACPI_MADT_POLARITY_ACTIVE_HIGH                      0b01
#define ACPI_MADT_POLARITY_ACTIVE_LOW                       0b11

#define ACPI_MADT_TRIGGERING_MASK                           0b1100
#define ACPI_MADT_TRIGGERING_CONFORMING                     0b0000
#define ACPI_MADT_TRIGGERING_EDGE                           0b0100
#define ACPI_MADT_TRIGGERING_LEVEL                          0b1100

#define ACPI_PM_TMR_HZ                                      3579545
#define ACPI_FADT_TMR_VAL_EXT_MASK                          (1 << 8)

typedef struct ATTR_PACKED acpiMadt
{
    acpiSdtHdr_t    hdr;
    uint32_t        localInterruptControllerAddress;
    uint32_t        flags;
    acpiEntryHdr_t  entries[];
} acpiMadt_t;
SIZE_ASSERT(acpiMadt_t, 44);

typedef struct ATTR_PACKED acpiMadtX2Apic
{
    acpiEntryHdr_t hdr;  
    uint16_t       rsvd;
    uint32_t       id;    
    uint32_t       flags; 
    uint32_t       uid; 
} acpiMadtX2Apic_t;
SIZE_ASSERT(acpiMadtX2Apic_t, 16);

typedef struct ATTR_PACKED acpiMadtLapic
{
    acpiEntryHdr_t hdr;
    uint8_t        uid;
    uint8_t        id;
    uint32_t       flags;
} acpiMadtLapic_t;
SIZE_ASSERT(acpiMadtLapic_t, 8);

typedef struct ATTR_PACKED acpiMadtIoApic
{
    acpiEntryHdr_t hdr;
    uint8_t        id;
    uint8_t        rsvd;
    uint32_t       address;
    uint32_t       gsiBase;
} acpiMadtIoApic_t;
SIZE_ASSERT(acpiMadtIoApic_t, 12);

typedef struct ATTR_PACKED acpiMadtLapicAddrOverride
{
    acpiEntryHdr_t hdr; 
    uint16_t       rsvd;
    uint64_t       address;
} acpiMadtLapicAddrOverride_t;
SIZE_ASSERT(acpiMadtLapicAddrOverride_t, 12);

typedef struct ATTR_PACKED acpiMadtX2ApicNmi
{
    acpiEntryHdr_t hdr;
    uint16_t       flags; 
    uint32_t       uid; 
    uint8_t        lint;
    uint8_t        reserved[3];
} acpiMadtX2ApicNmi_t;
SIZE_ASSERT(acpiMadtX2ApicNmi_t, 12);

typedef struct ATTR_PACKED acpiMadtLapicNmi
{
    acpiEntryHdr_t hdr; 
    uint8_t        uid; 
    uint16_t       flags;
    uint8_t        lint;
} acpiMadtLapicNmi_t;
SIZE_ASSERT(acpiMadtLapicNmi_t, 6);

typedef struct ATTR_PACKED acpiMadtNmiSource
{
    acpiEntryHdr_t hdr;
    uint16_t       flags;
    uint32_t       gsi;
} acpiMadtNmiSource_t;
SIZE_ASSERT(acpiMadtNmiSource_t, 8);

typedef struct ATTR_PACKED acpiGas
{
    uint8_t  addressSpaceId;
    uint8_t  registerBitWidth;
    uint8_t  registerBitOffset;
    uint8_t  accessSize; 
    uint64_t address; 
} acpiGas_t;

SIZE_ASSERT(acpiGas_t, 12);

typedef struct ATTR_PACKED acpiHpet
{
    acpiSdtHdr_t hdr;     
    uint32_t     blockId;  
    acpiGas_t    address;
    uint8_t      number;
    uint16_t     minClockTick;
    uint8_t      flags;
} acpiHpet_t;

SIZE_ASSERT(acpiHpet_t, 56);

typedef struct ATTR_PACKED acpiFadt 
{
    acpiSdtHdr_t hdr;
    uint32_t     firmwareCtrl;
    uint32_t     dsdt;
    uint8_t      reserved0; 
    uint8_t      preferredPmProfile;
    uint16_t     sciInt;
    uint32_t     smiCmd;
    uint8_t      acpiEnable;
    uint8_t      acpiDisable;
    uint8_t      s4BiosReq;
    uint8_t      pstateCnt;
    uint32_t     pm1aEvtBlk;
    uint32_t     pm1bEvtBlk;
    uint32_t     pm1aCntBlk;
    uint32_t     pm1bCntBlk;
    uint32_t     pm2CntBlk;
    uint32_t     pmTmrBlk;
    uint32_t     gpe0Blk;
    uint32_t     gpe1Blk;
    uint8_t      pm1EvtLen;
    uint8_t      pm1CntLen;
    uint8_t      pm2CntLen;
    uint8_t      pmTmrLen;
    uint8_t      gpe0BlkLen;
    uint8_t      gpe1BlkLen;
    uint8_t      gpe1Base;
    uint8_t      cstCnt;
    uint16_t     pLvl2Lat;
    uint16_t     pLvl3Lat;
    uint16_t     flushSize;
    uint16_t     flushStride;
    uint8_t      dutyOffset;
    uint8_t      dutyWidth;
    uint8_t      dayAlrm;
    uint8_t      monAlrm;
    uint8_t      century;
    uint16_t     iapcBootArch;
    uint8_t      reserved1;
    uint32_t     flags;
    acpiGas_t    resetReg;
    uint8_t      resetValue;
    uint16_t     armBootArch;
    uint8_t      fadtMinorVersion;
    uint64_t     xFirmwareCtrl;
    uint64_t     xDsdt;
    acpiGas_t    xPm1aEvtBlk;
    acpiGas_t    xPm1bEvtBlk;
    acpiGas_t    xPm1aCntBlk;
    acpiGas_t    xPm1bCntBlk;
    acpiGas_t    xPm2CntBlk;
    acpiGas_t    xPmTmrBlk;
    acpiGas_t    xGpe0Blk;
    acpiGas_t    xGpe1Blk;
    acpiGas_t    sleepControlReg;
    acpiGas_t    sleepStatusReg;
    uint64_t     hypervisorVendorId;
} acpiFadt_t;
SIZE_ASSERT(acpiFadt_t, 276);

#endif