#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "3dsx.h"

static size_t reloc(void* threedsx) {
	struct THREEDSX_Header* header = threedsx;
	uintptr_t startAddr = (uintptr_t) threedsx;
	u32 sectionLengths[] = {header->code_seg_size, header->rodata_seg_size, header->data_seg_size - header->bss_size};
	for (int section = 0; section < 3; section++) {
		printf("Relocing section %d\n", section);
		struct THREEDSX_RelocHdr* relocHeaders = (struct THREEDSX_RelocHdr*) (startAddr + header->header_size);
		uintptr_t sectionAddr = startAddr + header->header_size + (header->reloc_hdr_size*3);
		uintptr_t relocsAddr = sectionAddr;
		for (int s = 0; s < 3; s++) {
			relocsAddr += sectionLengths[s];
		}
		for (int s = 0; s < section; s++) {
			sectionAddr += sectionLengths[s];
			relocsAddr += (relocHeaders[s].cross_segment_absolute + relocHeaders[s].cross_segment_relative)*4;
		}
		printf("Start addr %x Section addr %x Relocs addr %x\n", startAddr, sectionAddr, relocsAddr);
		printf("A %x R %x\n", relocHeaders[section].cross_segment_absolute, relocHeaders[section].cross_segment_relative);
		printf("c %x r %x d %x\n", sectionLengths[0], sectionLengths[1], sectionLengths[2]);
		struct THREEDSX_RelocHdr relocHeader = relocHeaders[section];
		struct THREEDSX_Reloc* reloc = (struct THREEDSX_Reloc*) relocsAddr;
		u32* ptr = (u32*) sectionAddr;
		for (int relocIndex = 0; relocIndex < relocHeader.cross_segment_absolute; relocIndex++) {
			ptr += reloc->skip;
			for (int i = 0; i < reloc->patch; i++) {
				*ptr = (u32) ptr;
				ptr++;
			}
			reloc++;
		}
		for (int relocIndex = 0; relocIndex < relocHeader.cross_segment_relative; relocIndex++) {
			ptr += reloc->skip;
			for (int i = 0; i < reloc->patch; i++) {
				*ptr = (u32) ptr - (u32) startAddr;
				ptr++;
			}
			reloc++;
		}
	}
	return header->bss_size;
}

int main(int argc, char** argv) {
	int inputfd = open(argv[1], O_RDONLY);
	struct stat thestat;
	fstat(inputfd, &thestat);
	void** mm = mmap(NULL, ((thestat.st_size / 4096) + 1)*4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, inputfd, 0);
	size_t bss_alloc = reloc(mm);
	void** mm2 = mmap(((char*) mm)+((thestat.st_size / 4096) + 1)*4096, 0x1000000, 
		PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE |MAP_ANONYMOUS, -1, 0);
	char tempbuf[100];
	//snprintf(tempbuf, sizeof(tempbuf), "cat /proc/%d/maps", getpid());
	//system(tempbuf);
	void (*entry)() = (void (*)()) (((uintptr_t) mm) + 52);
	entry();
}
