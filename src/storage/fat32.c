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

fat32_result_t fat32_mount(void) {
  if (!parse_mbr())
    return FAT32_ERR_MBR_PARSE;
  if (!parse_vbr())
    return FAT32_ERR_VBR_PARSE;

  return FAT32_OK;
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
  uint8_t unused_1[11];
  uint16_t bytes_per_sector; // offset 0x0B
  uint8_t sectors_per_cluster;
  uint16_t num_reserved_sectors;
  uint8_t num_fats; // offset 0x10, ie 17th item, value is often '2'
  uint8_t unused_2[19];
  uint32_t sectors_per_fat; // offset 0x24
  uint8_t unused_3[4];
  uint32_t root_dir_first_cluster; // offset 0x02C, value is often '2'
  uint8_t unused_4[462];
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

  uint32_t root_lba = get_cluster_lba(fs.root_dir_first_cluster);
  (void)root_lba;
  return true;
}

// static void parse_root_dir(void) { sd_card_read_sector() }
