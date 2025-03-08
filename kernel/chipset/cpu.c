/*
 *
 *		cpu.c
 *		cpu相关操作
 *
 *		2024/8/21 By MicroFish
 *		基于 GPL-3.0 开源协议
 *		Copyright © 2020 ViudiraTech，基于GPLv3协议。
 *
 */

#include "cpu.h"
#include "printk.h"
#include "alloc.h"

/* 获取CPUID */
void cpuid(uint32_t code, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d)
{
	__asm__ __volatile__("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(code) : "memory");
}

/* 获取CPU厂商名称 */
char *get_vendor_name(void)
{
	int cpuid_level;
	static char x86_vendor_id[16] = {0};
	cpuid(0x00000000, (unsigned int *)&cpuid_level,
                      (unsigned int *)&x86_vendor_id[0],
                      (unsigned int *)&x86_vendor_id[8],
                      (unsigned int *)&x86_vendor_id[4]);
	return x86_vendor_id;
}

/* 获取CPU型号名称 */
char *get_model_name(void)
{
	static char model_name[64];
	uint32_t *p = (uint32_t *)model_name;

	cpuid(0x80000002, &p[0], &p[1], &p[2], &p[3]);
	cpuid(0x80000003, &p[4], &p[5], &p[6], &p[7]);
	cpuid(0x80000004, &p[8], &p[9], &p[10], &p[11]);
	model_name[48] = 0;
	return model_name;
}

/* 获取CPU物理地址大小 */
unsigned int get_cpu_phys_bits(void)
{
	unsigned int eax, ebx, ecx, edx;
	cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
	return eax & 0xff;
}

/* 获取CPU虚拟地址大小 */
unsigned int get_cpu_virt_bits(void)
{
	unsigned int eax, ebx, ecx, edx;
	cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
	return (eax >> 8) & 0xff;
}
