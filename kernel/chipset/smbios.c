/*
 *
 *		smbios.c
 *		系统管理BIOS
 *
 *		2025/3/8 By MicroFish
 *		基于 GPL-3.0 开源协议
 *		Copyright © 2020 ViudiraTech，基于GPLv3协议。
 *
 */

#include "smbios.h"
#include "limine.h"
#include "stddef.h"
#include "string.h"

__attribute__((used, section(".limine_requests"))) static __volatile__ struct limine_smbios_request smbios_request = {
	.id = LIMINE_SMBIOS_REQUEST};

/* 获取SMBIOS入口点 */
void *smbios_entry(void)
{
	if (smbios_request.response == 0)
		return 0;
	if (smbios_request.response->entry_64)
		return (void *)smbios_request.response->entry_64;
	else if (smbios_request.response->entry_32)
		return (void *)smbios_request.response->entry_32;
	return 0;
}

/* 获取SMBIOS主版本 */
int smbios_major_version(void)
{
	if (smbios_request.response->entry_64)
		return ((struct EntryPoint64 *)smbios_entry())->MajorVersion;
	else
		return ((struct EntryPoint32 *)smbios_entry())->MajorVersion;
}

/* 获取SMBIOS次版本 */
int smbios_minor_version(void)
{
	if (smbios_request.response->entry_64)
		return ((struct EntryPoint64 *)smbios_entry())->MinorVersion;
	else
		return ((struct EntryPoint32 *)smbios_entry())->MajorVersion;
}

/* 获取SMBIOS表长度 */
size_t smbios_struct_len(struct Header *hd)
{
	size_t i;
	const char *strtab = (char *)hd + hd->length;
	/* 一直扫描到找到两个0x0为止 */
	for (i = 1; strtab[i - 1] != '\0' || strtab[i] != '\0'; i++)
		;
	return hd->length + i + 1;
}
