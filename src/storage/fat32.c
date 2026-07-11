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

#define END_OF_CLUSTER_CHAIN 0 // safe as sentinel - cluster num 0 is reserved

static files_list_t files_list = {0};

typedef struct {
  uint32_t partition_lba;
  uint32_t fat_begin_lba;
  uint32_t cluster_begin_lba;
  uint32_t sectors_per_cluster;
  uint32_t bytes_per_cluster;
  uint32_t root_dir_first_cluster;
} fs_t;

static fs_t fs;
static uint8_t sector_buff[512];

typedef struct {
  uint32_t curr_cluster_num;
  uint32_t size_in_bytes;
  uint32_t pos;
  bool is_initialized;
} open_file_t;

static open_file_t open_file = {0};

static bool parse_mbr(void);
static bool parse_vbr(void);
static bool parse_root_dir(void);

static uint32_t get_cluster_lba(uint32_t cluster_num);
static uint32_t get_next_cluster(uint32_t cluster_num);

/*
  Public API
*/

fat32_result_t fat32_mount(void) {
  if (!parse_mbr())
    return FAT32_ERR_MBR_PARSE;
  if (!parse_vbr())
    return FAT32_ERR_VBR_PARSE;
  if (!parse_root_dir())
    return FAT32_ERR_DIR_PARSE;

  open_file.is_initialized = false;

  return FAT32_OK;
}

const files_list_t *fat32_get_files_list(void) { return &files_list; }

void fat32_open_file(const file_t *file) {
  open_file.curr_cluster_num = file->first_cluster;
  open_file.size_in_bytes = file->size_in_bytes;
  open_file.pos = 0;
  open_file.is_initialized = true;
}

file_result_t fat32_read_file(uint8_t *buff, uint32_t buff_len,
                              uint32_t *bytes_read) {
  *bytes_read = 0;
  if (!open_file.is_initialized) {
    return FILE_READ_ERR_NO_FILE;
  }

  file_result_t res = FILE_READ_OK;
  uint32_t remaining_in_file = open_file.size_in_bytes - open_file.pos;

  uint32_t to_read =
      (buff_len < remaining_in_file) ? buff_len : remaining_in_file;

  while (*bytes_read < to_read) {
    uint32_t cluster_lba = get_cluster_lba(open_file.curr_cluster_num);
    uint32_t pos_in_cluster = open_file.pos % fs.bytes_per_cluster;
    uint32_t curr_sector_num = pos_in_cluster / 512;
    uint32_t offset_within_sector = pos_in_cluster % 512;

    // when possible, stream the sector bytes directly to final buffer
    // otherwise will need to use the 512b sector_buff and copy partial data
    uint32_t space_left_in_buffer = buff_len - *bytes_read;
    uint8_t *_buffer =
        (space_left_in_buffer >= 512 && offset_within_sector == 0)
            ? &buff[*bytes_read]
            : sector_buff;

    if (sd_card_read_sector(cluster_lba + curr_sector_num, _buffer) !=
        CARD_OK) {
      res = FILE_READ_IO_ERR;
      break;
    }

    if (_buffer == sector_buff) {
      // copy partial sector data to final buffer
      uint32_t sector_bytes_to_copy = 512 - offset_within_sector;
      if (to_read - *bytes_read < sector_bytes_to_copy) {
        sector_bytes_to_copy = to_read - *bytes_read;
      }
      uint32_t idx = offset_within_sector;
      for (uint32_t remaining = sector_bytes_to_copy; remaining > 0;
           remaining--) {
        buff[(*bytes_read)++] = sector_buff[idx++];
      }
      open_file.pos += sector_bytes_to_copy;
    } else {
      open_file.pos += 512;
      *bytes_read += 512;
    }

    if (open_file.pos % fs.bytes_per_cluster == 0) {
      open_file.curr_cluster_num = get_next_cluster(open_file.curr_cluster_num);
    }
  }

  return res;
}

/*
  Helpers
*/

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
  fs.bytes_per_cluster = fs.sectors_per_cluster * 512;
  fs.root_dir_first_cluster = vbr->root_dir_first_cluster;

  return true;
}

// Per https://www.pjrc.com/tech/8051/ide/fat32.html
#define LONG_FILE_ATTRIB_MASK 0x0F
#define HIDDEN_FILE_ATTRIB_BIT (0x1 << 1)
#define DIR_ATTRIB_BIT (0x1 << 4)

typedef struct __attribute__((packed)) {
  uint8_t short_name[11];
  uint8_t attrib_byte;
  uint8_t __unused_1[8];
  uint16_t first_cluster_high; // offset 0x14
  uint8_t __unused_2[4];
  uint16_t first_cluster_low; // offset 0x1A
  uint32_t size_in_bytes;
} dir_entry_raw_t;
_Static_assert(sizeof(dir_entry_raw_t) == 32,
               "dir_entry_raw_t must be exactly 32 bytes");

static inline void copy_entry_to_list(dir_entry_raw_t *raw);

typedef struct {
  uint8_t buff[512];
  uint32_t cached_sector_lba; // which FAT sector is currently loaded
} fat_cache_t;

static fat_cache_t fat_cache = {
    0}; // safe to init to zero - clusters 0 and 1 are never used

static uint32_t get_next_cluster(uint32_t cluster_num) {
  uint32_t entry_offset_bytes =
      cluster_num * 4; // each entry is 32 bits, 4 bytes
  uint32_t req_sector_lba = fs.fat_begin_lba + (entry_offset_bytes / 512);
  uint32_t entry_offset_within_sector = entry_offset_bytes % 512;

  if (!(req_sector_lba == fat_cache.cached_sector_lba)) {
    card_result_t res = sd_card_read_sector(req_sector_lba, fat_cache.buff);
    if (res != CARD_OK) {
      // TODO: better error handling
    }
    fat_cache.cached_sector_lba = req_sector_lba;
  }

  uint32_t raw = *(uint32_t *)&fat_cache.buff[entry_offset_within_sector];

  if (raw >= 0xFFFFFFF8) {
    return END_OF_CLUSTER_CHAIN;
  }

  return raw & 0x0FFFFFFF; // mask off top 4 bits, per
                           // https://www.pjrc.com/tech/8051/ide/fat32.html
}

typedef enum { DIR_SCAN_CONTINUE, DIR_SCAN_DONE } dir_scan_result_t;

static bool is_pic_file(dir_entry_raw_t *e) {
  char *required_ext = "PIC";
  for (int i = 8, j = 0; j < 3; i++, j++) {
    if (e->short_name[i] != required_ext[j]) {
      return false;
    }
  }

  return true;
}

static dir_scan_result_t process_dir_sector(void) {
  dir_entry_raw_t *entry;
  // each dir entry is 32 bytes
  for (uint32_t offset = 0; offset <= (512 - 32); offset += 32) {
    entry = (dir_entry_raw_t *)&sector_buff[offset];

    if (entry->short_name[0] == 0x00) {
      return DIR_SCAN_DONE; // marks end of dir
    } else if (entry->short_name[0] == 0xE5) {
      continue; // deleted file
    }

    if ((entry->attrib_byte & LONG_FILE_ATTRIB_MASK) == LONG_FILE_ATTRIB_MASK) {
      continue;
    }

    if (entry->attrib_byte & HIDDEN_FILE_ATTRIB_BIT) {
      continue;
    }

    if (entry->attrib_byte & DIR_ATTRIB_BIT) {
      continue; // keep things simple, all files should be placed in root
                // dir
    }

    if (entry->size_in_bytes == 0) {
      continue;
    }

    if (!is_pic_file(entry)) {
      continue;
    }

    if (files_list.count >= ENTRIES_CAP) {
      return DIR_SCAN_DONE;
    } else {
      copy_entry_to_list(entry);
    }
  }

  return DIR_SCAN_CONTINUE;
}

static bool parse_root_dir(void) {

  files_list.count = 0;

  uint32_t cluster_num = fs.root_dir_first_cluster;
  while (cluster_num != END_OF_CLUSTER_CHAIN) {
    uint32_t cluster_lba = get_cluster_lba(cluster_num);
    for (uint32_t s = 0; s < fs.sectors_per_cluster; s++) {
      card_result_t res = sd_card_read_sector(cluster_lba + s, sector_buff);
      if (res != CARD_OK) {
        return false;
      }

      if (process_dir_sector() == DIR_SCAN_DONE) {
        return true;
      }
    }
    cluster_num = get_next_cluster(cluster_num);
  }

  return true;
}

static inline void copy_entry_to_list(dir_entry_raw_t *raw) {
  if (files_list.count >= ENTRIES_CAP)
    return;

  file_t *p = &files_list.files[files_list.count++];

  p->first_cluster =
      ((raw->first_cluster_high << 16) | (raw->first_cluster_low));

  p->size_in_bytes = raw->size_in_bytes;
}
