#include <stdint.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

/** 
 * File layout:
 * - File header
 * - Code, rodata and data relocation table headers
 * - Code segment
 * - Rodata segment
 * - Loadable (non-BSS) part of the data segment
 * - Code relocation table
 * - Rodata relocation table
 * - Data relocation table
 *
 * Memory layout before relocations are applied:
 * [0..codeSegSize)             -> code segment
 * [codeSegSize..rodataSegSize) -> rodata segment
 * [rodataSegSize..dataSegSize) -> data segment
 *
 * Memory layout after relocations are applied: well, however the loader sets it up :)
 * The entrypoint is always the start of the code segment.
 * The BSS section must be cleared manually by the application.
 */
enum THREEDSX_Error {
    ERROR_NONE = 0,
    ERROR_READ = 1,
    ERROR_FILE = 2,
    ERROR_ALLOC = 3
};
static const u32 RELOCBUFSIZE = 512;

// File header
static const u32 THREEDSX_MAGIC = 0x58534433; // '3DSX'
#pragma pack(1)
struct THREEDSX_Header
{
    u32 magic;
    u16 header_size, reloc_hdr_size;
    u32 format_ver;
    u32 flags;

    // Sizes of the code, rodata and data segments +
    // size of the BSS section (uninitialized latter half of the data segment)
    u32 code_seg_size, rodata_seg_size, data_seg_size, bss_size;
};

// Relocation header: all fields (even extra unknown fields) are guaranteed to be relocation counts.
struct THREEDSX_RelocHdr
{
    // # of absolute relocations (that is, fix address to post-relocation memory layout)
    u32 cross_segment_absolute; 
    // # of cross-segment relative relocations (that is, 32bit signed offsets that need to be patched)
    u32 cross_segment_relative; 
    // more?

    // Relocations are written in this order:
    // - Absolute relocations
    // - Relative relocations
};

// Relocation entry: from the current pointer, skip X words and patch Y words
struct THREEDSX_Reloc
{
    u16 skip, patch;
};
#pragma pack()

struct THREEloadinfo
{
    u8* seg_ptrs[3]; // code, rodata & data
    u32 seg_addrs[3];
    u32 seg_sizes[3];
};

