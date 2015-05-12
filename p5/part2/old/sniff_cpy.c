#include <stdio.h>
#include <stdlib.h>
#include "fs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string.h>
#include <assert.h>

#define BSIZE 4096  // block size
int readFile(char *);
checkpoint* readCheckPoint(int);
inodeMap* readFirstIMap(int, int);
inode* getRootInode(int, int);
void printInode(inode*);
void printInodeMap(inodeMap*);
void printCheckPoint(checkpoint *);

char** str_split(char* a_str, const char a_delim) {
	char** result = 0;
	size_t count = 0;
	char* tmp = a_str;
	char* last_comma = 0;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;

	/* Count how many elements will be extracted. */
	while (*tmp) {
		if (a_delim == *tmp) {
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

	/* Add space for trailing token. */
	count += last_comma < (a_str + strlen(a_str) - 1);

	/* Add space for terminating null string so caller
	 knows where the list of returned strings ends. */
	count++;

	result = malloc(sizeof(char*) * count);

	if (result) {
		size_t idx = 0;
		char* token = strtok(a_str, delim);

		while (token) {
			assert(idx < count);
			*(result + idx++) = strdup(token);
			token = strtok(0, delim);
		}
		assert(idx == count - 1);
		*(result + idx) = 0;
	}

	return result;
}

int main(int argc, char* argv[]) {

	char fileName[] = "dir/test";
	char** tokens;

	printf("fileName=[%s]\n\n", fileName);

	tokens = str_split(fileName, '/');

	if (tokens) {
		int i;
		for (i = 0; *(tokens + i); i++) {
			printf("fileName=[%s]\n", *(tokens + i));
			free(*(tokens + i));
		}
		printf("\n");
		free(tokens);
	}

	int i = 0;
	//int j = 0;

	int fd = readFile("example.img");
	if (fd < 0) {
		printf("ERROR: SRG YOU ARE IN TROUBLE!");
		return -1;
	}

	checkpoint* sb = readCheckPoint(fd);
	if (sb == NULL) {
		printf("ERROR: SRG YOU ARE IN TROUBLE!");
		return -1;
	}
	//printCheckPoint(sb);

	int firstIMapAddress = sb->iMapPtr[0];
	inodeMap* first_iMap = readFirstIMap(fd, firstIMapAddress);
	if (first_iMap == NULL) {
		printf("ERROR: SRG YOU ARE IN TROUBLE!");
		return -1;
	}

	printInodeMap(first_iMap);

	// 0th inode is root!
	inode* rootInode = getRootInode(fd, first_iMap->inodePtr[0]);

	printf("\n ****** SRG: ROOT INODE\n\n");
	printInode(rootInode);

	for (i = 0; i < 14; i++) {
		/*if (rootInode->ptr[i] == -1) {
		 continue;
		 }*/

		dirEnt* oneDirEnt;
		oneDirEnt = (dirEnt*) (malloc(sizeof(dirEnt)));
		if (lseek(fd, rootInode->ptr[i], SEEK_SET) < 0) {
			//printf("ERROR: lseek failed\n");
		}
		if (read(fd, oneDirEnt, sizeof(dirEnt)) < 0) {
			printf("\tERROR: dirEnt cast failed\n");
		}

		if (oneDirEnt->inum == -1) {
			break;
		}
		printf("\n\t\t [inum = %d] %s", oneDirEnt->inum, oneDirEnt->name);

	}
	printf("\n");

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

inodeMap* readFirstIMap(int fd, int firstIMapAddress) {
	inodeMap* first_iMap;
	first_iMap = (inodeMap*) (malloc(sizeof(inodeMap)));

	if (lseek(fd, firstIMapAddress, SEEK_SET) < 0) {
		printf("ERROR: lseek failed @ readFirstIMap\n");
	} else {
		if (read(fd, first_iMap, sizeof(inodeMap)) < 0) {
			printf("ERROR: read failed @ readFirstIMap\n");
		} /*else {
		 printf("INFO: first inodeMap casted\n");
		 }*/
	}
	return first_iMap;
}

int readFile(char* fileName) {
	int fd = open("example.img", O_RDWR);
	if (fd < 0) {
		printf("ERROR: fd < 0\n");
	}
	return fd;
}

inode* getRootInode(int fd, int aInodePtr) {

	inode* rootInode;
	rootInode = (inode*) (malloc(sizeof(inode)));

	if (lseek(fd, aInodePtr, SEEK_SET) < 0) {
		printf("ERROR: lseek failed\n");
	}

	if (read(fd, rootInode, sizeof(inode)) < 0) {
		printf("ERROR: inode cast failed\n");
	}
	return rootInode;
}

void printInode(inode* anInode) {
	printf("\nINFO [iNode info]");
	printf("\n\tsize= %d\n\ttype=%d", anInode->size, anInode->type);
	int j = 0;
	for (j = 0; j < 14; j++) {
		if (anInode->ptr[j] == -1) {
			continue;
		}
		printf("\n\tptr[%d]=%d", j, anInode->ptr[j]);
	}
	printf("\n\n");

}

void printInodeMap(inodeMap* anInodeMap) {
	int i = 0;
	printf("\nINFO [iNodeMap info]");
	for (i = 0; i < 16; i++) {
		if (anInodeMap->inodePtr[i] == -1) {
			continue;
		}
		printf("\n\tinodePtr[%d] = %d", i, anInodeMap->inodePtr[i]);
	}
	printf("\n\n");

}
void printCheckPoint(checkpoint * aCheckPoint) {
	int i = 0;
	printf("\nINFO [checkPoint info]");
	printf("\n\tsize = %d", aCheckPoint->size);
	for (i = 0; i < INODEPIECES; i++) {
		printf("\n\tiMapPtr[%d] = %d", i, aCheckPoint->iMapPtr[i]);
	}
	printf("\n\n");
}
