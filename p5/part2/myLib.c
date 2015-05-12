#include <stdio.h>
#include <stdlib.h>
#include "fs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "myLib.h"

#include <string.h>
#include <assert.h>

int fd;
checkpoint* sb;
int firstIMapAddress;
inodeMap* first_iMap;
inode* rootInode;
char** str_split(char*, const char);
int doStuff(char*);

int getType(int inum) {
	int address = first_iMap->inodePtr[inum];
	inode* anInode;
	anInode = (inode*) (malloc(sizeof(inode)));

	if (lseek(fd, address, SEEK_SET) < 0) {
		printf("Error!\n");
		exit(0);
	}

	if (read(fd, anInode, sizeof(inode)) < 0) {
		printf("Error!\n");
		exit(0);
	}
	return anInode->type;
}

void printFile(inode* anInode) {

	if (anInode->type != MFS_REGULAR_FILE) {
		printf("Error!\n");
		exit(0);
	}
	int i = 0;
	for (i = 0; i < 14; i++) {

		if (anInode->ptr[i] == -1) {
			continue;
		}
		char someData[anInode->size];
		if (lseek(fd, anInode->ptr[i], SEEK_SET) < 0) {
			printf("Error!\n");
			exit(0);
		}
		if (read(fd, someData, anInode->size) < 0) {
			printf("Error!\n");
			exit(0);
		}
		printf("%s", someData);
	}
}

void printDir(inode* anInode) {

	if (anInode->type == MFS_REGULAR_FILE) {
		printf("Error!\n");
		exit(0);
	}

	int i = 0;
	int count = 0;
	for (i = 0; i < 14; i++) {

		if (anInode->ptr[i] <= -1) {
			continue;
		}

		do {
			dirEnt* oneDirEnt;
			oneDirEnt = (dirEnt*) (malloc(sizeof(dirEnt)));
			if (lseek(fd, anInode->ptr[i] + (sizeof(dirEnt) * count), SEEK_SET)
					< 0) {
				printf("Error!\n");
				exit(0);
			}
			if (read(fd, oneDirEnt, sizeof(dirEnt)) < 0) {
				printf("Error!\n");
				exit(0);
			}

			if (oneDirEnt->inum <= -1) {
				break;
			}
			count = count + 1;
			if (getType(oneDirEnt->inum) == MFS_REGULAR_FILE) {
				printf("%s\n", oneDirEnt->name);
			} else {
				printf("%s/\n", oneDirEnt->name);
			}
		} while (1);
	}
}

void find(inode* anInode, char* fName, int work) {
	if (work == 1 && strcmp(fName, "") == 0) {
		//print root
		printDir(anInode);
		return;
	} else if (work == 0 && strcmp(fName, "") == 0) {
		//printing empty file
		printf("Error!\n");
		exit(0);
	}

	char * pch;
	pch = strtok(fName, "/");
	int curInum;
	while (pch != NULL) {

		int i = 0;
		int count = 0;
		int isFound = 0;

		for (i = 0; i < 14; i++) {

			if (anInode->ptr[i] <= -1) {
				continue;
			}

			do {
				dirEnt* oneDirEnt;
				oneDirEnt = (dirEnt*) (malloc(sizeof(dirEnt)));
				if (lseek(fd, anInode->ptr[i] + sizeof(dirEnt) * count,
						SEEK_SET) < 0) {
					printf("Error!\n");
					exit(0);
				}
				if (read(fd, oneDirEnt, sizeof(dirEnt)) < 0) {
					printf("Error!\n");
					exit(0);
				}

				if (oneDirEnt->inum == -1) {
					break;
				}
				count = count + 1;

				if (strcmp(pch, oneDirEnt->name) == 0) {

					int address = first_iMap->inodePtr[oneDirEnt->inum];

					if (lseek(fd, address, SEEK_SET) < 0) {
						printf("Error!\n");
						exit(0);
					}
					if (read(fd, anInode, sizeof(inode)) < 0) {
						printf("Error!\n");
						exit(0);
					}

					curInum = oneDirEnt->inum;
					isFound = 1;
					break;
				}
			} while (1);
		}

		if (isFound == 0) {
			printf("Error!\n");
			exit(0);
		}

		pch = strtok(NULL, "/");
	}

	if (work == 1) {
		printDir(anInode);
	} else {
		printFile(anInode);
	}
}


void ls(char* path, char* img) {
	if (doStuff(img) == -1) {
		printf("Error!\n");
		exit(0);
	}
	find(rootInode, path, 1);
}

void cat(char* path, char* img) {

	if (doStuff(img) == -1) {
		printf("Error!\n");
		exit(0);
	}
	find(rootInode, path, 0);
}


int main(int argc, char* argv[]) {
	if(strcmp(argv[1],"ls")==0){
		ls(argv[2],argv[3]);
	}else if(strcmp(argv[1],"cat")==0){
		cat(argv[2],argv[3]);
	}else {
		printf("Error!\n");
	}

/*
	char l1[] = "";
	printf("\n\nSRG TEST: ls - %s\n", l1);
	ls(l1, "example.img");

	char l2[] = "/";
	printf("\n\nSRG TEST: ls - %s\n", l2);
	ls(l2, "example.img");

	char l3[] = "/dir";
	printf("\n\nSRG TEST: ls - %s\n", l3);
	ls(l3, "example.img");

	char l4[] = "/code/";
	printf("\n\nSRG TEST: ls - %s\n", l4);
	ls(l4, "example.img");

	char l5[] = "/nested/";
	printf("\n\nSRG TEST: ls - %s\n", l5);
	ls(l5, "example.img");

	char l6[] = "/nested/directory/example";
	printf("\n\nSRG TEST: ls - %s\n", l6);
	ls(l6, "example.img");

	char l7[] = "/nested/directory";
	printf("\n\nSRG TEST: ls - %s\n", l7);
	ls(l7, "example.img");


	char c0[] = "/code/a.out";
	printf("\n\nSRG TEST: cat - %s\n", c0);
	cat(c0, "example.img");

	char c1[] = "/code/helloworld.c";
	printf("\n\nSRG TEST: cat - %s\n", c1);
	cat(c1, "example.img");

	char c2[] = "/veryveryveryveryveryveryveryveryveryverylengthyfilename.txt";
	printf("\n\nSRG TEST: cat - %s\n", c2);
	cat(c2, "example.img");


	char c3[] = "/nested/directory/example/sample.c";
	printf("\n\nSRG TEST: cat - %s\n", c3);
	cat(c3, "example.img");*/

	return 0;

}

checkpoint* readCheckPoint(int fd) {

	checkpoint* sb;
	sb = (checkpoint*) (malloc(sizeof(checkpoint)));

	if (read(fd, sb, sizeof(checkpoint)) < 0) {
		printf("Error!\n");
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
		}
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

char** str_split(char* a_str, const char a_delim) {
	char** result = 0;
	size_t count = 0;
	char* tmp = a_str;
	char* last_comma = 0;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;

	while (*tmp) {
		if (a_delim == *tmp) {
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

	count += last_comma < (a_str + strlen(a_str) - 1);
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

int doStuff(char * img) {

	//fd = readFile("example.img");
	fd = readFile(img);
	if (fd < 0) {
		printf("Error!\n");
		return -1;
	}

	sb = readCheckPoint(fd);
	if (sb == NULL) {
		printf("Error!\n");
		return -1;
	}
	//printCheckPoint(sb);

	firstIMapAddress = sb->iMapPtr[0];
	first_iMap = readFirstIMap(fd, firstIMapAddress);
	if (first_iMap == NULL) {
		printf("Error!\n");
		return -1;
	}

	//printInodeMap(first_iMap);

	// 0th inode is root!
	rootInode = getRootInode(fd, first_iMap->inodePtr[0]);
	if (rootInode == NULL) {
		printf("Error!\n");
		return -1;
	}
	return 0;
}

