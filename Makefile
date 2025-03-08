# =====================================================
#
#		Makefile
#		Uinxed编译脚本
#
#		2024/6/23 By Rainy101112
#		基于 GPL-3.0 开源协议
#		Copyright © 2020 ViudiraTech，基于GPLv3协议。
#
# =====================================================

ifeq ($(VERBOSE), 1)
  V=
  Q=
else
  V=@printf "\033[1;32m[Build]\033[0m $@ ...\n";
  Q=@
endif

C_SOURCES	:= $(shell find * -name "*.c")
S_SOURCES	:= $(shell find * -name "*.s")
OBJS		:= $(C_SOURCES:%.c=%.o) $(S_SOURCES:%.s=%.o)
DEPS		:= $(OBJS:%.o=%.d)
LIBS		:= $(wildcard libs/lib*.a)

QEMU		:= qemu-system-x86_64
QEMU_FLAGS	:= -machine q35 -drive if=pflash,format=raw,file=assets/ovmf-code.fd

C_FLAGS		:= -Wall -Wextra -g3 -O0 -m64 -nostdinc -fno-builtin -fno-pie -fno-stack-protector -fno-sanitize=undefined \
               -mcmodel=kernel -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -msoft-float -I include -MMD
LD_FLAGS	:= -static -nostdlib -m elf_x86_64 -T assets/linker.ld
AS_FLAGS	:= -g --64

all: info Uinxed-x64.iso

%.o: %.c
	$(V)$(CC) $(C_FLAGS) -c -o $@ $<

%.o: %.s
	$(V)$(AS) $(AS_FLAGS) -o $@ $<

.PHONY: info
info:
	@printf "Uinxed-Kernel Compile Script.\n"
	@printf "Copyright 2020 ViudiraTech. Based on the GPLv3 license.\n"
	@printf "Based on the GPL-3.0 open source license.\n"
	@echo

UxImage: $(OBJS) $(LIBS)
	$(V)$(LD) $(LD_FLAGS) -o $@ $^

Uinxed-x64.iso: UxImage
	@echo
	@printf "\033[1;32m[ISO]\033[0m Packing ISO file...\n"
	@cp -a assets/Limine iso
	@cp $< iso/efi/boot

	@xorriso -as mkisofs -R -r -J -iso-level 3 --efi-boot /efi/boot/limine-uefi-cd.bin -o Uinxed-x64.iso iso

	@rm -rf iso
	@printf "\033[1;32m[Done]\033[0m Compilation complete.\n"

.PHONY: clean
clean:
	$(Q)$(RM) $(OBJS) $(DEPS) UxImage Uinxed-x64.iso

.PHONY: run
run: Uinxed-x64.iso
	$(QEMU) $(QEMU_FLAGS) -cdrom $<

-include $(DEPS)
