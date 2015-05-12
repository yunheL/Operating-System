#ifndef MYMEM_H
#define	MYMEM_H

#define BSIZE 4096  // block size
int readFile(char *);
checkpoint* readCheckPoint(int);
inodeMap* readFirstIMap(int, int);
inode* getRootInode(int, int);
void printInode(inode*);
void printInodeMap(inodeMap*);
void printCheckPoint(checkpoint *);
void ls(char[], char[]);
void cat(char[], char[]);

#endif // MYMEM_H
