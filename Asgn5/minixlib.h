#ifndef MINIXLIB_H
#define MINIXLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>

/* useful constant 
 * form the specs 
 */
#define lOCATION_PARTITION 0x1BE 
#define PARTITION_TYPE 0x81 
#define BYTE_510_BOOT_SECTOR 0x55 
#define BYTE_511_BOOT_SECTOR 0xAA  
#define MINIX_MAGIC 0x4D5A 
#define MINIX_MAGIC_REVERSED 0x5A4D 
#define INODE_SIZE 64 
#define DIR_SIZE 64 

#define SECTOR_SIZE 512
#define BLOCK_SIZE 1024

#define MASK_REG(m) (((m)&0170000)==0100000)
#define MASK_DIR(m) (((m)&0170000)==0040000)

#define MASK_IRUSR 0400
#define MASK_IWUSR 0200
#define MASK_IXUSR 0100
#define MASK_IRGRP 0040
#define MASK_IWGRP 0020
#define MASK_IXGRP 0010
#define MASK_IROTH 0004
#define MASK_IWOTH 0002
#define MASK_IXOTH 0001

#define DIRECT_ZONES 7
#define NAME_SIZE 60
#define PERM_SIZE 10

/* pre defined struct */
typedef struct par_table {
   uint8_t  bootind;    /* boot magig num */
   uint8_t  start_head; /* start of partition */
   uint8_t  start_sec;
   uint8_t  start_cyl;
   uint8_t  type;       /* type of parti */
   uint8_t  end_head;   /* end of parti */
   uint8_t  end_sec;
   uint8_t  end_cyl;
   uint32_t lFirst;     /* first sector */
   uint32_t size;       /* size of partition in sectors */
} partable_t;

typedef struct superblock {
	uint32_t ninodes; /* # of inodes in this filesystem */
	uint16_t pad1; /* make things line up properly */
	int16_t i_blocks; /* # of blocks used by inode bit map */
	int16_t z_blocks; /* # of blocks used by zone bit map */
	uint16_t firstdata; /* number of first info zone */
	int16_t log_zone_size; /* log2 of blocks per zone */
	int16_t pad2; /* make things line up again */
	uint32_t max_file; /* maximum file size */
	uint32_t zones; /* number of zones on disk */
	int16_t magic; /* magic number */
	int16_t pad3; /* make things line up again */
	uint16_t blocksize; /* block size in bytes */
	uint8_t subversion; /* filesystem sub–version */
}superblock_t;

typedef struct inode {
	uint16_t mode;
	uint16_t links;
	uint16_t uid;
	uint16_t gid;
	uint32_t size;
	int32_t atime;
	int32_t mtime;
	int32_t ctime;
	uint32_t zone[DIRECT_ZONES];
    uint32_t indirect;
	uint32_t two_indirect;
	uint32_t unused;
}inode_t;

typedef struct entry{
    uint32_t inode;
    uint8_t  name[NAME_SIZE];
}entry_t;

typedef struct file {
    inode_t node;
    entry_t entry;
    uint8_t *contents;
    entry_t *entries;
    size_t numEntries;
    char *path;
} file_t;

/* info on the requested info 
 * useful to pass around multiple 
 * variable 
 */
typedef struct info {
    const char *image;
    const char *path;
    const char *host;
    int primaryPart;
    int subPart;
    int varbose;
    FILE *file;
   
    long start;
    long zoneSize;
    superblock_t sblock;
} info_t;

/* some core function */
/* printing */
void printOption(void);
void printPerm(uint16_t mode);
void printInode(const inode_t *node);
void print_partable(partable_t *primaryPart);
void printSuperblock(const superblock_t *sblock);
/* image */
void openImage(info_t *info);
void closeImage(info_t *info);
/* partition */
void partition(info_t *info, int primaryPart);
void open_partition(info_t *info);
/* super block */
void read_superblock(info_t *info);
/* inode */
uint32_t getInode(const info_t *info, const char *path, uint32_t inode);
uint32_t getInode_wpath(const info_t *info, const char *path);
void readInode(const info_t *info, inode_t *node, uint32_t num);
/* files */
void loadFile(const info_t *info, file_t *file);
void loadDir(const info_t *info, file_t *dir);
file_t * openFile_winode(const info_t *info, uint32_t node);
file_t *openFile_wpath(const info_t *info,const char *path);
void closeFile(file_t *file);

#endif