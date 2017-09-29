/* minls.c 
 * Joseph Alfredo
 */

#include "minixlib.h"

void printUsageInfo(){
	printf("usage: minls  [ -v ] [ -p num [ -s num ] ] "
		"imagefile [ path ]\n");
	printOption();
}

void initInfo(info_t *info){
	info->image = NULL;
	info->path = "/";
	info->primaryPart = -1;
	info->subPart = -1;
	info->varbose = 0;
	info->start  = 0;
}
void parseArg(info_t *info, int argc, char * argv[]){
	int i;

  	for(i = 1; i < argc;i++){
  		if(!strcmp(argv[i],"-v")){
  			info->varbose = 1;
  		}
  		else if(!strcmp(argv[i],"-p") && ++i < argc){
  			sscanf(argv[i]," %d ",&info->primaryPart);
  		}
  		else if(!strcmp(argv[i],"-s") && ++i < argc){
  			sscanf(argv[i]," %d ",&info->subPart);
  		}
  		else if(!strcmp(argv[i],"-h")){
  			printUsageInfo();
  			exit(1);
  		}
  		else if(!info->image){
  			info->image = argv[i];
  		}
  		else{
  			info->path = argv[i];
  		}
  	}

  	if(!info->image){
  		printUsageInfo();
  		exit(1);
  	}
}
/* entry is the name and the inode */
void printEntry(const info_t *info, const entry_t *entry){
	char *name;
	inode_t node;

	if(!(name = (char*)malloc(NAME_SIZE + 1))){
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	/* fetch the inode and the name from entry*/
	readInode(info,&node, entry->inode);
	memcpy(name,entry->name,NAME_SIZE);
	name[NAME_SIZE] = '\0';

	/* print permission */
	printPerm(node.mode);
	printf("%10d %s\n",node.size, name);
	free(name);
}

void printLS(info_t *info){
	file_t *file;
	int i;
	char first = info->path[0];

	file = openFile_wpath(info, info->path);

	if(!file){
		fprintf(stderr,"%s: File not found.\n",info->path);
		closeImage(info);
		exit(1);
	}
	else{
		/* see the option requested and display
		 * requested stuff */
		if(info->varbose){
			printInode(&file->node);
		}
		if(file->entries){
			/* complete entries */
			printf("%s:\n", info->path);
			entry_t *entry = file->entries;
			for(i = 0; i < file->numEntries;++i, ++entry){
				if(entry->inode){
					printEntry(info, entry);
				}
			} 
		}
		else{
			/* print permission */
			printPerm(file->node.mode);
			printf("%10d ",file->node.size);
			if(first == '/'){
				printf("%c",first);
			}
			printf("%s\n", file->path + 1);
		}
		closeFile(file);
	}
}

int main(int argc,char * argv[]){
	info_t info;

	initInfo(&info);
	parseArg(&info, argc, argv);
	openImage(&info);
	open_partition(&info);
	read_superblock(&info);
	printLS(&info);
	closeImage(&info);

	return 0;
}
