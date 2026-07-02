#include "fat32.h"
#include "drivers/sd_card.h"
#include <stdbool.h>
#include <stdint.h>

/*
  Used combination of wikipedia,
  https://www.pjrc.com/tech/8051/ide/fat32.html ,
  and https://wiki.osdev.org/FAT
  as guides to come up with this implementation
*/

static dir_entries_list_t dir_entries_list = {0};

typedef struct {
  uint32_t partition_lba;
  uint32_t fat_begin_lba;
  uint32_t cluster_begin_lba;
  uint32_t sectors_per_cluster;
  uint32_t root_dir_first_cluster;
} fs_t;

static fs_t fs;
static uint8_t sector_buff[512];

static bool parse_mbr(void);
static bool parse_vbr(void);
static bool parse_root_dir(void);

fat32_result_t fat32_mount(void) {
  if (!parse_mbr())
    return FAT32_ERR_MBR_PARSE;
  if (!parse_vbr())
    return FAT32_ERR_VBR_PARSE;
  if (!parse_root_dir())
    return FAT32_ERR_DIR_PARSE;

  return FAT32_OK;
}

const dir_entries_list_t *fat32_get_dir_entries_list(void) {
  return &dir_entries_list;
}

// mbr == "Master Boot Record", is first sector of disk
static bool parse_mbr(void) {
  card_result_t res = sd_card_read_sector(0, sector_buff);
  if (res != CARD_OK) {
    return false;
  }
  /*
    Per wikipedia: https://en.wikipedia.org/wiki/Master_boot_record

    MBR partition 1 entry (16 bytes) starting at 0x01BE, ei byte 446

    Within that entry:
     - LBA of first absolute sector at offset 0x08 (4 bytes long)
     - Num of sectors in partition at offset 0x0C (4 bytes long)

    All values stored in little endian, so fine to directly cast as uint32

    The last 2 bytes should be 0x55 and 0xAA.
  */
  uint16_t mbr_signature = *(uint16_t *)&sector_buff[512 - 2];
  // becomes AA55 (swapped) when cast to a 16bit num
  if (mbr_signature != 0xAA55) {
    return false;
  }

  // for simplicity, always using the first partition of SD card
  fs.partition_lba = *(uint32_t *)&sector_buff[446 + 8];
  return true;
}

// based on data table from https://wiki.osdev.org/FAT
typedef struct __attribute__((packed)) {
  uint8_t __unused_1[11];
  uint16_t bytes_per_sector; // offset 0x0B
  uint8_t sectors_per_cluster;
  uint16_t num_reserved_sectors;
  uint8_t num_fats; // offset 0x10, ie 17th item, value is often '2'
  uint8_t __unused_2[19];
  uint32_t sectors_per_fat; // offset 0x24
  uint8_t __unused_3[4];
  uint32_t root_dir_first_cluster; // offset 0x02C, value is often '2'
  uint8_t __unused_4[462];
  uint16_t signature; // offset 0x1FE, should be 0xAA55
} vbr_t;

_Static_assert(sizeof(vbr_t) == 512, "vbr_t must be exactly 512 bytes");

static uint32_t get_cluster_lba(uint32_t cluster_num) {
  return fs.cluster_begin_lba + (cluster_num - 2) * fs.sectors_per_cluster;
}

// vbr == "Volume Boot Record", is first sector of the partition.
// In this case, should contain the FAT32 FS info.
static bool parse_vbr(void) {
  card_result_t res = sd_card_read_sector(fs.partition_lba, sector_buff);
  if (res != CARD_OK) {
    return false;
  }

  vbr_t *vbr = (vbr_t *)sector_buff;

  if (vbr->signature != 0xAA55) {
    return false;
  }

  if (vbr->bytes_per_sector != 512) {
    return false;
  }

  /*
    Per https://www.pjrc.com/tech/8051/ide/fat32.html
  */
  fs.fat_begin_lba = fs.partition_lba + vbr->num_reserved_sectors;
  fs.cluster_begin_lba =
      fs.fat_begin_lba + (vbr->num_fats * vbr->sectors_per_fat);
  fs.sectors_per_cluster = vbr->sectors_per_cluster;
  fs.root_dir_first_cluster = vbr->root_dir_first_cluster;

  return true;
}

// skip long file names
#define LONG_FILE_ATTRIB 0x0F

typedef struct __attribute__((packed)) {
  char short_name[11];
  uint8_t attrib_byte;
  uint8_t __unused_1[8];
  uint16_t first_cluster_high; // offset 0x14
  uint8_t __unused_2[4];
  uint16_t first_cluster_low; // offset 0x1A
  uint32_t file_size;
} dir_entry_raw_t;
_Static_assert(sizeof(dir_entry_raw_t) == 32,
               "dir_entry_t must be exactly 32 bytes");

static inline void copy_entry_to_buffered_list(dir_entry_raw_t *raw);
static bool parse_root_dir(void) {
  uint32_t root_lba = get_cluster_lba(fs.root_dir_first_cluster);
  card_result_t res = sd_card_read_sector(root_lba, sector_buff);
  if (res != CARD_OK) {
    return false;
  }

  dir_entries_list.buffered_count = 0;
  dir_entries_list.total_entries = 0;

  uint32_t offset = 0;
  dir_entry_raw_t *temp;
  for (; offset < (512 - 32); offset += 32) {
    temp = (dir_entry_raw_t *)&sector_buff[offset];

    if (temp->attrib_byte & LONG_FILE_ATTRIB) {
      continue;
    }

    if (temp->file_size == 0) {
      continue;
    }

    // TODO, filter to only proper file extensions

    dir_entries_list.total_entries++;

    if (dir_entries_list.buffered_count < BUFF_ENTRIES_CAP) {
      copy_entry_to_buffered_list(temp);
    }
  }

  return true;
}

static inline void copy_entry_to_buffered_list(dir_entry_raw_t *raw) {
  dir_entry_t *p =
      &dir_entries_list.buffered_entries[dir_entries_list.buffered_count++];

  for (int i = 0; i < 11; i++) {
    p->short_name[i] = raw->short_name[i];
  }
  p->short_name[11] = '\0';

  p->first_cluster =
      ((raw->first_cluster_high << 8) | (raw->first_cluster_low));
}
