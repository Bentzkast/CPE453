/* minixlib.c 
 * Joseph Alfredo
 * contain logictic function calls.
 */
#include "minixlib.h"

/* printing */
void printOption(){
	/* both program seems to have same option */
	printf("Options:\n");
	printf("\t-p\tpart    --- select partition for filesystem" 
		"(default: none)\n");
	printf("\t-s\tsub     --- select subpartition for filesystem"
		"(default : none)\n");
	printf("\t-h\thelp    --- print usage information and exit\n");
	printf("\t-v\tverbose --- increase verbosity level\n");
}

void printPerm(uint16_t mode){
   	char perms[PERM_SIZE];
    int i = 0;
  	perms[i++] = MASK_DIR(mode)    ? 'd' : '-';
  	perms[i++] = mode & MASK_IRUSR ? 'r' : '-';
  	perms[i++] = mode & MASK_IWUSR ? 'w' : '-';
    perms[i++] = mode & MASK_IXUSR ? 'x' : '-';
    perms[i++] = mode & MASK_IRGRP ? 'r' : '-';
    perms[i++] = mode & MASK_IWGRP ? 'w' : '-';
    perms[i++] = mode & MASK_IXGRP ? 'x' : '-';
    perms[i++] = mode & MASK_IROTH ? 'r' : '-';
    perms[i++] = mode & MASK_IWOTH ? 'w' : '-';
    perms[i++] = mode & MASK_IXOTH ? 'x' : '-';
    perms[i] = '\0';
   
    printf("%s", perms);
}

void printInode(const inode_t *node){
    char tbuff[NAME_SIZE];
    time_t time;
    int i;
   
    printf("File inode:\n");
    printf("  unsigned short mode         0x%04x", node->mode);
    printf("\t(");
    printPerm(node->mode);
    printf(")\n");
   
    printf("  unsigned short links %13u\n", node->links);
    printf("  unsigned short uid %15u\n", node->uid);
    printf("  unsigned short gid %15u\n", node->gid);
    printf("  unsigned long  size %14u\n", node->size);
    
    time = node->atime;
    strftime(tbuff, NAME_SIZE, "%a %b %e %T %Y", localtime(&time));
    printf("  unsigned long  atime %13u\t--- %s\n", node->atime, tbuff);
   
    time = node->mtime;
    strftime(tbuff, NAME_SIZE, "%a %b %e %T %Y", localtime(&time));
    printf("  unsigned long  mtime %13u\t--- %s\n", node->mtime, tbuff);
   
    time = node->ctime;
    strftime(tbuff, NAME_SIZE, "%a %b %e %T %Y", localtime(&time));
    printf("  unsigned long  ctime %13u\t--- %s\n", node->ctime, tbuff);
   
    printf("\n  Direct zones:\n");
   	for (i = 0; i < DIRECT_ZONES; ++i)
      	printf("              zone[%d]   = %10u\n", i, node->zone[i]);
   
   	printf("  unsigned long  indirect %10u\n", node->indirect);
   	printf("  unsigned long  double %12u\n", node->unused);
}

void print_partable(partable_t *partition){
	int i;

    printf("       ----Start----      ------End-----\n");
    printf("  Boot head  sec  cyl Type head  sec  cyl      First       Size\n");
   
    for (i = 0; i < 4; ++i, ++partition) {
	    printf("  0x%02x ", partition->bootind);
	    printf("%4u ", partition->start_head);
        printf("%4u ", partition->start_sec);
        printf("%4u ", partition->start_cyl);
        printf("0x%02x ", partition->type);
	    printf("%4u ", partition->end_head);
        printf("%4u ", partition->end_sec);
	    printf("%4u ", partition->end_cyl);
       	printf("%10u ", partition->lFirst);
       	printf("%10u\n", partition->size);
   }
}

void printSuperblock(const superblock_t *super) {
   uint32_t size = super->blocksize << super->log_zone_size;
   
    printf("\nSuperblock Contents:\n");
    printf("Stored Fields:\n");
    printf("  ninodes %12u\n", super->ninodes);
    printf("  i_blocks %11d\n", super->i_blocks);
    printf("  z_blocks %11d\n", super->z_blocks);
    printf("  firstdata %10u\n", super->firstdata);
    printf("  log_zone_size %6d", super->log_zone_size);
    printf(" (zone size: %u)\n", size);
    printf("  max_file %11u\n", super->max_file);
    printf("  magic         0x%04x\n", super->magic);
    printf("  zones %14u\n", super->zones);
    printf("  blocksize %10u\n", super->blocksize);
    printf("  subversion %9u\n", super->subversion);
   
    printf("Computed Fields:\n");
    printf("  firstIblock %8u\n", 4);   
    printf("  zonesize %11u\n", size);
    printf("  ptrs_per_zone %6u\n", 1024);
    printf("  ino_per_block %6u\n", 64);   
    printf("  wrongended %9u\n\n", super->magic == MINIX_MAGIC_REVERSED);
}

/* image */
void openImage(info_t *info){
	info->file = fopen(info->image, "rb");

	if(!info->file){
		fprintf(stderr,"Unable to open disk image \" %s\".\n"
			,info->image);
		exit(3);
	}
}

void closeImage(info_t *info){
	if(fclose(info->file)!= 0){
        fprintf(stderr,"Unable to close disk image \" %s\".\n"
            ,info->image);
        exit(3);
    }
}

/* partition */
void partition(info_t *info, int partition){
   	uint8_t block[BLOCK_SIZE];
   	partable_t *ptable;

   	if(fseek(info->file, info->start, SEEK_SET) == -1){
   		perror(NULL);
   		exit(1);
   	}
   	fread((void*)block, BLOCK_SIZE, 1, info->file);
	if(ferror(info->file )!= 0){
		perror(NULL);
		exit(1);
	}
   
   	if(block[510] != BYTE_510_BOOT_SECTOR || 
   		block[511] != BYTE_511_BOOT_SECTOR){
     	fprintf(stderr,"Invalid partition table.\n");
      	fprintf(stderr,"Unable to open disk image \"%s\".\n", info->image);
      	closeImage(info);
      	exit(3);
   	}
   
   	ptable = (partable_t*)(block + lOCATION_PARTITION);
   	if(info->varbose){
      	print_partable(ptable);
   	}
   
   	ptable += partition;
   	if(ptable->type != PARTITION_TYPE){
      	fprintf(stderr,"Not a Minix subpartition.\n");
      	fprintf(stderr,"Unable to open disk image \"%s\".\n", info->image);
      	closeImage(info);
      	exit(3);
   	}
   	info->start = ptable->lFirst * SECTOR_SIZE;
}

void open_partition(info_t *info){
	/* check if there any partition specified */
	/* primary */
	if(info->primaryPart == -1){
		return;
	}
	if(info->varbose){
		printf("\nPartition table:\n");
	}
	partition(info, info->primaryPart);

	/* sub */
	if(info->subPart == -1){
		return;
	}
	if(info->varbose){
		printf("\nSubpartition table:\n");
	}
	partition(info,info->subPart);
}

/* super block */
void read_superblock(info_t *info){
	uint8_t block[BLOCK_SIZE];
   	superblock_t *sblock;
   	int ret;
   
   	/* read the superblock */
   	ret = fseek(info->file, info->start+BLOCK_SIZE, SEEK_SET);
   	if(ret == -1){
   		perror(NULL);
   		exit(255);
   	}
   	fread((void*)block, BLOCK_SIZE, 1, info->file);
	if(ferror(info->file )!= 0){
		perror(NULL);
		exit(1);
	}
   	sblock = (superblock_t*)block;
   
 	if(sblock->magic != MINIX_MAGIC){
    	fprintf(stderr,"Bad magic number. (0x%04x)\n", sblock->magic);
      	fprintf(stderr,"This doesn't look like a MINIX filesystem.\n");
      	closeImage(info);
      	exit(255);
   	}
   
   	info->zoneSize = sblock->blocksize << sblock->log_zone_size;
   	info->sblock = *sblock;
   
   	if(info->varbose){
      	printSuperblock(&info->sblock);
   	}
}

/* inode */
uint32_t getInode(const info_t *info, const char *path, uint32_t inode){
	char name[NAME_SIZE + 1];
	char *next = strstr(path, "/");
   	file_t *file = NULL;
   	entry_t *entry;
   	int i = 0;
   	int final;
   	
   	file = openFile_winode(info, inode);
   	if(!file){
		fprintf(stderr,"%s: File not found.\n",info->path);
		closeImage((info_t *)info);
		exit(1);
   	}

   	entry = file->entries;
   	final = (file->node.size + sizeof(entry_t) - 1)/ sizeof(entry_t);

   	if(next){
	    *next = '\0';
	    next++;
   	}
   	name[NAME_SIZE] = '\0';
   
   	for(i = 0; i < final; ++i, ++entry){
      	memcpy(name, entry->name, NAME_SIZE);
      	if(!strcmp(path, name)){
         	break;
      	}
    }
   
   	inode = entry->inode;
   	closeFile(file);
   
   	/* using a recursion to follow path until the end */
   	if(next && *next){
      	return getInode(info, next, inode);
   	}
   
   	return inode;
}

uint32_t getInode_wpath(const info_t *info, const char *path){
   	size_t size = strlen(path);
   	uint32_t inode = 1;
   
   	if(size > 1){
      	char *copy = (char*)malloc(size);
      	if(!copy){
      		perror(NULL);
      		exit(1);
      	}
      	strcpy(copy, path+1);
      	inode = getInode(info, copy, inode);
      	free(copy);
   	}
   	return inode;
}

void readInode(const info_t *info, inode_t *node, uint32_t num){
	long location = info->start + (num - 1) * sizeof(inode_t);
	location += info->sblock.blocksize * (2 + info->sblock.i_blocks 
        + info->sblock.z_blocks);

	if(fseek(info->file, location, SEEK_SET) == -1){
		perror(NULL);
		exit(1);
	}
	fread((void*)node, sizeof(inode_t),1,info->file);
	if(ferror(info->file )!= 0){
		perror(NULL);
		exit(1);
	}
}

/* files */
void loadFile(const info_t *info, file_t *file){
	long size = file->node.size;
   
    if(size){
    	int i;
      	uint8_t *current, *buffer;
      
      	buffer = (uint8_t*)malloc(info->zoneSize);
      	if(!buffer){
      		perror(NULL);
      		exit(1);
      	}
      	file->contents = (uint8_t*)malloc(size);
      	if(!file->contents){
      		perror(NULL);
      		exit(1);
      	}
      	current = file->contents;
      
      	for(i = 0; i < DIRECT_ZONES && size > 0; ++i){
      		/* clump it up */
         	long transfer = size < info->zoneSize ? size : 
                info->zoneSize;
         
         	if(file->node.zone[i]){
            	long padding = info->start + info->zoneSize 
            		* file->node.zone[i];
            
            	/* finding the stuff to fill buffer */
            	if(fseek(info->file, padding, SEEK_SET) == -1){
            		perror(NULL);
            		exit(1);
            	}
            	fread((void*)buffer, transfer, 1, info->file);
        		if(ferror(info->file )!= 0){
					perror(NULL);
					exit(1);
				}
            	memcpy(current, buffer, transfer);

         	}
         	else{
            	memset(current, NULL, transfer);
         	}
            size -= transfer;
            current += transfer;
      	}
      
      	free(buffer);
   	}
   	else{
      	file->contents = NULL;
   	}
}

void loadDir(const info_t *info, file_t *dir){
   
    loadFile(info, dir);
   
    if(dir->contents){
      	entry_t *current, *entry = (entry_t*)dir->contents;
      	int i;
      
      	dir->numEntries = (dir->node.size +sizeof(entry_t) - 1)
      		 / sizeof(entry_t);
   		current = (entry_t*)malloc(dir->numEntries * 
   			sizeof(entry_t));
   		if(!current){
   			perror(NULL);
   			exit(1);
   		}
      	dir->entries = current;
      	/* connect the stuff */
      	for (i = 0; i < dir->numEntries; i++,current++, entry++){
         	*current = *entry;
      	}
   	}
   	else{
		fprintf(stderr,"%s: File not found.\n",info->path);
		exit(1);
   	}
}

file_t * openFile_winode(const info_t *info, uint32_t node){
	file_t *file;

	if(!(file = (file_t*)malloc(sizeof(file_t)))){
		perror(NULL);
		exit(1);
	}

	readInode(info, &file->node, node);
	file->path = NULL;
	file->contents = NULL;
	file->entries = NULL;
	file->numEntries = 0;

	if(MASK_DIR(file->node.mode)){
		loadDir(info, file);
	}
	return file;
}

static void pathAdjustment(const char *path, file_t *file){

    /* Paths that do not include a leading ‘/’ 
     * are processed relative to the root directory. 
     */
    if(path[0] != '/'){
        if(!(file->path = (char*)malloc(strlen(path + 1)))){
            perror(NULL);
            exit(1);
        }
        file->path[0] = '/';
        strcpy(file->path + 1, path);
    }
    else{
        if(!(file->path = (char*)malloc(strlen(path)))){
            perror(NULL);
            exit(1);
        }
        strcpy(file->path, path);
    }
}

file_t *openFile_wpath(const info_t *info,const char *path){
	file_t *file;
	uint32_t inode;

	if(!(file = (file_t*)malloc(sizeof(file_t)))){
		perror(NULL);
		exit(1);
	}

    pathAdjustment(path,file);

	inode = getInode_wpath(info,file->path);
	if(!inode){
		free(file);
		return NULL;
	}

	readInode(info, &file->node, inode);
	file->contents = NULL;
	file->entries = NULL;
	file->numEntries = 0;

	if(MASK_DIR(file->node.mode)){
		loadDir(info, file);
	}
	else{
		loadFile(info,file);
	}
	return file;
}

void closeFile(file_t *file){
	free(file->contents);
	free(file->entries);
	free(file);
}
