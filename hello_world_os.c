// Copyright (C) 2008 Michal Drozd
// All Rights Reserved

// Very simple "operating system" in C which after booting show "Hello World" on display, with its own printf implementation!
// KEEP IN MIND, you can NOT just compile it in standard way as standard program as there is no OS when this program is started, so "PE header" won't be recognised of course, etc.
// Raw compiled code must be placed to boot sector to run it.

// How to build this:
// nasm -f bin -o boot.bin boot.asm
// gcc -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c hello_world_os.c
// ld -m elf_i386 --oformat binary -Ttext 0x7c00 --o hello_world_os.bin hello_world_os.o
// cat boot.bin c_code.bin > os.img
// qemu-system-i386 os.img

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>


// This function depends on architecture a lot of course!
void bios_video_print_char(char c) {
	auto *video_memory = (uint8_t *) 0xB8000;
	static uint8_t x = 0;
	static uint8_t y = 0;
	switch (c) {
	case '\n': {
		x = 0;
		y++;
		break;
	}
	default: {
		video_memory[2 * (80 * y + x)] = c;
		video_memory[2 * (80 * y + x) + 1] = 0x07; // white on black text
		x++;
		if (x >= 80) {
			x = 0;
			y++;
		}
		break;
	}
	}
	if (y >= 25) {
		for (y = 0; y < 24; y++) {
			for (x = 0; x < 80; x++) {
				video_memory[2 * (80 * y + x)] = video_memory[2 * ((80 * y + 1) + x)];
				video_memory[2 * ((80 * y + 1) + x)] = 0x07; // white on black text
			}
		}
		for (x = 0; x < 80; x++) {
			video_memory[2 * ((80 * 24) + x)] = ' ';
			video_memory[2 * ((80 * 24) + x) + 1] = 0x07; // white on black text
		}
		y = 24;
	}
}

void print(const char* str)
{

	while (*str)
	{
		bios_video_print_char(*str);
		str++;
	}
}

void print_hex(uint32_t num)
{
	char hex[] = "0123456789ABCDEF";
	for (int i = 28; i >= 0; i -= 4)
	{
		bios_video_print_char(hex[(num >> i) & 0xF]);
	}
}
void print_dec(uint32_t num)
{
	if (num < 10) {
		bios_video_print_char(num + '0');
	} else {
		print_dec(num / 10);
		bios_video_print_char(num % 10 + '0');
	}
}

void printf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	while (*format != '\0')
	{
		if (*format == '%')
		{
			format++;
			switch (*format)
			{
			case 'c':
			{
				char c = va_arg(args, int);
				bios_video_print_char(c);
				break;
			}
			case 's':
			{
				char* str = va_arg(args, char *);
				print(str);
				break;
			}
			case 'd':
			{
				int32_t num = va_arg(args, int32_t);
				if (num < 0)
				{
					bios_video_print_char('-');
					num = -num;
				}
				print_dec(num);
				break;
			}
			case 'x':
			{
				uint32_t num = va_arg(args, uint32_t);
				print("0x");
				print_hex(num);
				break;
			}
			}
		}
		else
		{
			bios_video_print_char(*format);
		}
		format++;
	}

	va_end(args);

}

// This is very HW specific, need to adjust for your HW!
void bios_video_set_mode(uint8_t mode) {
	asm volatile("int $0x10" : : "a"(0x00), "b"(mode));
}

// Boot entry point!
void main()
{
	bios_video_set_mode(0x03);
	printf("Hello world!\n");
	printf("%d\n", -123);
	printf("%x\n", 0x12345678);
	for (;;)
	{}
}