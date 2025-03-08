/*
 *
 *		power.c
 *		电源管理
 *
 *		2025/2/16 By MicroFish
 *		基于 GPL-3.0 开源协议
 *		Copyright © 2020 ViudiraTech，基于GPLv3协议。
 *
 */

#include "acpi.h"
#include "timer.h"
#include "common.h"
#include "hhdm.h"
#include "printk.h"
#include "string.h"

uint16_t SLP_TYPa;
uint16_t SLP_TYPb;
uint32_t SMI_CMD;
uint16_t SLP_EN;
uint16_t SCI_EN;
acpi_facp_t *facp;

/* 初始化facp */
void facp_init(acpi_facp_t *facp0)
{
	int i;
	uint8_t *S5Addr;
	uint32_t dsdtlen;
	facp = facp0;

	dsdt_table_t *dsdtTable = (dsdt_table_t*)(uint64_t)facp->dsdt;
	if (dsdtTable == 0) {
		plogk("FACP: DSDT table not found.\n");
		return;
	} else {
		dsdtTable = phys_to_virt((uint64_t)dsdtTable);
		plogk("FACP: DSDT table found at 0x%016x\n", (unsigned long long)dsdtTable);
	}
	if (!memcmp(dsdtTable->signature, "DSDT", 4)) {
		S5Addr = &(dsdtTable->definition_block);
		dsdtlen = dsdtTable->length - 36;
		while (dsdtlen--) {
			if (!memcmp(S5Addr, "_S5_", 4)) {
				break;
			}
			S5Addr++;
		}
		SLP_EN = 1 << 13;
		SCI_EN = 1;

		if (dsdtlen) {
			if (*(S5Addr - 1) == 0x08 || (*(S5Addr - 2) == 0x08 && *(S5Addr - 1) == '\\')) {
				S5Addr += 5;
				S5Addr += ((*S5Addr & 0xC0) >> 6) + 2;
				if (*S5Addr == 0x0A) {
					S5Addr++;
				}
				SLP_TYPa = *(S5Addr) << 10;
				S5Addr++;
				if (*S5Addr == 0x0A) {
					S5Addr++;
				}
				SLP_TYPb = *(S5Addr) << 10;
				S5Addr++;
				plogk("FACP: SLP_TYPa = 0x%04x, SLP_TYPb = 0x%04x\n", SLP_TYPa, SLP_TYPb);
			}
		} else {
			plogk("FACP: _S5_ not found in DSDT.\n");
		}
	} else {
		plogk("FACP: Invalid DSDT signature.\n");
	}
	if (inw(facp->pm1a_cnt_blk) & SCI_EN) {
		plogk("FACP: SCI already enabled.\n");
		return;
	}
	if (SMI_CMD && facp->acpi_enable) {
		plogk("FACP: Enabling ACPI via SMI command.\n");
		outb(SMI_CMD, facp->acpi_enable);
		for (i = 0; i < 300; i++){
			if (inw(facp->pm1a_cnt_blk) & SCI_EN) {
				plogk("FACP: ACPI enabled successfully.\n");
				break;
			}
			usleep(5);
		}
		if (facp->pm1b_cnt_blk) {
			for (int i = 0; i < 300; i++) {
				if (inw(facp->pm1b_cnt_blk) & SCI_EN) {
					plogk("FACP: ACPI enabled successfully.\n");
					break;
				}
				usleep(5);
			}
		}
		if (i < 300) {
			plogk("FACP: ACPI enable failed.\n");
			return;
		}
	}
}

/* 重启电源 */
void power_reset(void)
{
	if (!SCI_EN)
		return;
	while (1) {
		outb(0x92, 0x01);
		outb((uint32_t)facp->reset_reg.address, facp->reset_value);
	}
}

/* 关闭电源 */
void power_off(void)
{
	if (!SCI_EN) return;
	while (1) {
		outw((uint32_t)facp->pm1a_cnt_blk, SLP_TYPa | SLP_EN);
		if (!facp->pm1b_cnt_blk) {
			outw((uint32_t)facp->pm1b_cnt_blk, SLP_TYPb | SLP_EN);
		}
	}
}
