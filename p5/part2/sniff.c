#include <stdio.h>
#include <stdlib.h>
#include "fs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BSIZE 4096  // block size
//remember to check function return value

checkpoint* readCheckPoint(int);
inodeMap* readFirstIMap(int, int);
int readFile(char *);


int main(int argc, char* argv[]) {

	int i = 0;
	int j = 0;

	int fd = readFile("example.img");
	if(fd<0){
		printf("ERROR: SRG YOU ARE IN TROUBLE!");
		return -1;
	}

	checkpoint* sb = readCheckPoint(fd);
	if(sb == NULL){
		printf("ERROR: SRG YOU ARE IN TROUBLE!");
		return -1;
	}

	int rootAddress = sb->iMapPtr[0];
	inodeMap* first_iMap = readFirstIMap(fd, rootAddress);
	if(first_iMap == NULL){
		printf("ERROR: SRG YOU ARE IN TROUBLE!");
		return -1;
	}

	printf("=====================\nIMAP info:\n");
	printf("=====================\n");
	//printf("size = %d\n", sb->size);
	for (i = 0; i < 16; i++) {

		if (first_iMap->inodePtr[i] == -1) {
			continue;
		}
		printf("INFO [IMAP info]  inodePtr[%d] = %d\n", i,
				first_iMap->inodePtr[i]);

		inode* oneInode;
		oneInode = (inode*) (malloc(sizeof(inode)));

		if (lseek(fd, first_iMap->inodePtr[i], SEEK_SET) < 0) {
			printf("ERROR: lseek failed\n");
		}

		if (read(fd, oneInode, sizeof(inode)) < 0) {
			printf("\tERROR: inode cast failed\n");
		} else {
			printf("\tINFO: inode casted\n");
		}

		printf("\tINFO [inode info]\n\t\tsize= %d\n\t\ttype=%d\n\t\tpts:\n",
				oneInode->size, oneInode->type);

		if (oneInode->type == 1) {
			dirEnt* oneDirEnt;
			oneDirEnt = (dirEnt*) (malloc(sizeof(dirEnt)));
			if (lseek(fd, first_iMap->inodePtr[i], SEEK_SET) < 0) {
				printf("ERROR: lseek failed\n");
			}
			printf("\t\t\t\tinum=%d\n", oneDirEnt->inum);
			printf("\t\t\t\tname=%s\n", oneDirEnt->name);
		}
		for (j = 0; j < 14; j++) {
			if (oneInode->ptr[j] == -1) {
				continue;
			}
			printf("\t\t\tpts[%d]=%d\n", j, oneInode->ptr[j]);

			/*if (lseek(fd, oneInode->ptr[j], SEEK_SET) < 0) {
			 printf("ERROR: lseek failed\n");
			 }

			 dirEnt* oneDirEnt;
			 oneDirEnt = (dirEnt*) (malloc(sizeof(dirEnt)));

			 if (read(fd, oneDirEnt, sizeof(dirEnt)) < 0) {
			 printf("\tERROR: dirEnt cast failed\n");
			 } else {
			 printf("\tINFO: dirEnt casted\n");
			 }

			 printf("\t\t\t\tinum=%d\n", oneDirEnt->inum);
			 printf("\t\t\t\tname=%s\n", oneDirEnt->name);*/
		}

	}
	printf("=====================\n\n");

	return 0;

}



checkpoint* readCheckPoint(int fd) {

	checkpoint* sb;
	sb = (checkpoint*) (malloc(sizeof(checkpoint)));

	if (read(fd, sb, sizeof(checkpoint)) < 0) {
		printf("ERROR: checkpoint cast failed\n");

	} else {
		printf("INFO: checkpoint casted\n");
		int i = 0;
		printf("\tINFO [CR info]  size = %d\n", sb->size);
		for (i = 0; i <= 0; i++) {
			//for (i = 0; i < INODEPIECES; i++) {
			printf("\tINFO [CR info]  iMapPtr[%d] = %d\n", i, sb->iMapPtr[i]);
		}
	}

	return sb;
}

inodeMap* readFirstIMap(int fd, int rootAddress) {
	inodeMap* first_iMap;
	first_iMap = (inodeMap*) (malloc(sizeof(inodeMap)));

	if (lseek(fd, rootAddress, SEEK_SET) < 0) {
		printf("ERROR: lseek failed @ readFirstIMap\n");
	} else {
		if (read(fd, first_iMap, sizeof(inodeMap)) < 0) {
			printf("ERROR: read failed @ readFirstIMap\n");
		} else {
			printf("INFO: first inodeMap casted\n");
		}
	}
	return first_iMap;
}


int readFile(char* fileName){
	int fd = open("example.img", O_RDWR);
	if (fd < 0) {
		printf("ERROR: fd < 0\n");
	}
	return fd;
}
