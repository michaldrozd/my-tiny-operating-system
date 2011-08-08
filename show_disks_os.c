// Copyright (C) 2008 Michal Drozd
// All Rights Reserved

// More advanced "operating system" in C which after booting show all available disks
// KEEP IN MIND, you can NOT just compile it in standard way as standard program as there is no OS when this program is started, so "PE header" won't be recognised of course, etc.
// Raw compiled code must be placed to boot sector to run it.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <bios.h>

// BIOS interrupt 13H (disk services) function 8H (get drive parameters)
// http://www.ctyme.com/intr/rb-1570.htm
// http://www.ctyme.com/intr/rb-1571.htm
// http://www.ctyme.com/intr/rb-1572.htm
// http://www.ctyme.com/intr/rb-1573.htm
// http://www.ctyme.com/intr/rb-1574.htm
// http://www.ctyme.com/intr/rb-1575.htm

// struct for drive parameters (see above)

struct drive_parameters {

	unsigned char size; // size of this structure in bytes (0x1E)

	unsigned char flags; // bit 0: 1=drive exists, 0=drive does not exist

	unsigned short cylinders; // number of cylinders on the disk

	unsigned char heads; // number of heads on the disk

	unsigned char sectors_per_track; // number of sectors per track on the disk

	unsigned short bytes_per_sector; // number of bytes per sector on the disk

	unsigned short total_sectors; // total number of sectors on the disk

	unsigned short sectors_per_fat; // number of sectors per FAT on the disk

	unsigned short drive_descriptor; // drive descriptor byte

	unsigned short version; // version number of the BPB (BIOS Parameter Block)

	unsigned short root_entries; // maximum number of root directory entries on the disk

	unsigned short total_sectors_small; // total number of sectors on the disk if < 32MB

	unsigned char media_descriptor; // media descriptor byte

	unsigned short sectors_per_fat_small; // number of sectors per FAT on the disk if < 32MB

	unsigned short sectors_per_track_small; // number of sectors per track on the disk if < 32MB

	unsigned short heads_small; // number of heads on the disk if < 32MB

	unsigned long hidden_sectors; // number of hidden sectors preceding this partition (if partitioned)

	unsigned long total_sectors_large; // total number of sectors on the disk if >= 32MB
};

// BIOS interrupt 13H (disk services) function 8H (get drive parameters)
// http://www.ctyme.com/intr/rb-1570.htm
// http://www.ctyme.com/intr/rb-1571.htm
// http://www.ctyme.com/intr/rb-1572.htm
// http://www.ctyme.com/intr/rb-1573.htm
// http://www.ctyme.com/intr/rb-1574.htm
// http://www.ctyme.com/intr/rb-1575.htm
int getdriveparameters(unsigned char drive, struct drive_parameters *dp) {

	union REGS regs;

	struct SREGS sregs;

	memset(&regs, 0, sizeof(regs));
	memset(&sregs, 0, sizeof(sregs));

	regs.h.ah = 0x08;
	regs.h.dl = drive + 1;
	sregs.es = FP_SEG(dp);
	regs.x.di = FP_OFF(dp);
	int86x(0x13, &regs, &regs, &sregs);

	return regs.x.cflag ? -1 : 0;
}
void main() {

	struct drive_parameters dp[26];

	int i;

	for (i = 0; i < 26; i++) {

		if (getdriveparameters(i, &dp[i]) == 0) {

			if (dp[i].flags & 1) {

				union REGS regs;

				struct SREGS sregs;

				memset(&regs, 0, sizeof(regs));
				memset(&sregs, 0, sizeof(sregs));

				regs.h.ah = 0x0E;
				regs.h.al = 'A' + i;
				regs.h.bh = 0x00;
				regs.h.bl = 0x07;

				int86(0x10, &regs, &regs);

				regs.h.ah = 0x0E;
				regs.h.al = ':';
				regs.h.bh = 0x00;
				regs.h.bl = 0x07;

				int86(0x10, &regs, &regs);

				regs.h.ah = 0x0E;
				regs.h.al = ' ';
				regs.h.bh = 0x00;
				regs.h.bl = 0x07;

				int86(0x10, &regs, &regs);

				regs.h.ah = 0x0E;
				regs.h.al = '\r';
				regs.h.bh = 0x00;
				regs.h.bl = 0x07;

				int86(0x10, &regs, &regs);

				regs.h.ah = 0x0E;
				regs.h.al = '\n';
				regs.h.bh = 0x00;
				regs.h.bl = 0x07;

				int86(0x10, &regs, &regs);
			}
		}
	}

	// Wait for a key press.
	// http://www.ctyme.com/intr/rb-2974.htm
	// http://www.ctyme.com/intr/rb-2975.htm

	union REGS regs;

	struct SREGS sregs;

	memset(&regs, 0, sizeof(regs));
	memset(&sregs, 0, sizeof(sregs));

	regs.h.ah = 0x00;

	int86(0x16, &regs, &regs);
}