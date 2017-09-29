#include "minixlib.h"

void printUsageInfo(){
	printf("usage: minget  [ -v ] [ -p num [ -s num ] ]"
		"imagefile srcpath [ dstpath ]\n");
	printOption();
}

void initInfo(info_t *info){
    info->image = NULL;
    info->path = NULL;
    info->host = NULL;
    info->primaryPart = -1;
    info->subPart = -1;
    info->varbose = 0;
    info->start  = 0;
}

void parseArgs(info_t *info, int argc, char * argv[]){
	int i;
    
	/* parse the args */
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
  		else if(!info->path){
  			info->path = argv[i];
  		}
  		else{
  			info->host = argv[i];
  		}
  	}

  	if(!info->image || !info->path ){
  		printUsageInfo();
  		exit(1);
  	}
}

void getFile(info_t *info){
	file_t *file;
   
   	file = openFile_wpath(info, info->path);
   
  	if(!file){
  		fprintf(stderr,"%s: File not found.\n", info->path);
      	closeImage(info);
      	exit(1);
   	}
   	else{
      	if(MASK_DIR(file->node.mode)){
	  		fprintf(stderr,"%s: Not a regular file.\n", info->path);
        	closeImage(info);
        	exit(1);
      	}
  		loadFile(info, file);
      	if(info->host){ 
         	FILE *fout = fopen(info->host, "wb");
        	if(!fout){
            	fprintf(stderr,"Could not open outfile\n");
            	closeFile(file);
            	closeImage(info);
            	exit(1);
        	}
         
        	fwrite(file->contents, file->node.size, 1, fout);
        	fclose(fout);
      	}
      	else{
         	write(1, file->contents, file->node.size);
      	}
      
      	closeFile(file);
      	
   	}
}

int main(int argc, char *argv[]){
	info_t info;

    initInfo(&info);
	parseArgs(&info,argc, argv);
	openImage(&info);
	open_partition(&info);
	read_superblock(&info);
	getFile(&info);
	closeImage(&info);
	return 0;
}