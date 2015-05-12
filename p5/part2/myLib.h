#ifndef MYLIB_H
#define	MYLIB_H

#define BSIZE 4096  // block size
int readFile(char *);
checkpoint* readCheckPoint(int);
inodeMap* readFirstIMap(int, int);
inode* getRootInode(int, int);
void printInode(inode*);
void printInodeMap(inodeMap*);
void printCheckPoint(checkpoint *);
//void ls2(char[], char[]);
//void cat2(char[], char[]);

#endif // MYLIB_H
