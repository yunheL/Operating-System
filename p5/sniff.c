#include <stdio.h>
#include <stdlib.h>
#include "fs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void printdn(struct dinode* dn);


//remember to check function return value

int 
main(int argc, char* argv[])
{

  int fd = open("fs.img", O_RDWR);
  if (fd < 0)
  {
	printf("error: fd < 0\n");
  }

  struct superblock* sb;
  sb = (struct superblock*)(malloc(sizeof(struct superblock)));

  if (lseek(fd, BSIZE, SEEK_SET) < 0)
  {
	printf("error: lseek failed\n");
  }

  if (read(fd, sb, sizeof(struct superblock)) < 0)
  {
	printf("error: read failed\n");
  }
 
  printf("=====================\nSuperblock info:");
  printf("size = %d\n nblocks = %d\n ninodes = %d\n", sb->size, sb->nblocks, sb->ninodes);
  printf("=====================\n");



  //inode struct
  struct dinode* dn;
  dn = (struct dinode*)(malloc(sizeof(struct dinode)));

  if ( lseek(fd, BSIZE * 2, SEEK_SET) < 0)
  {
       printf("error: lseek failed\n");
  }

  if (read(fd, dn, 200 * sizeof(struct dinode)) < 0)
  {
        printf("error: read failed\n");
  }

  printf("inode0 is reserved. Not printing.\n");


  int inode_cnt;
  for(inode_cnt = 1; inode_cnt < 200; inode_cnt++)
  {
	dn++;
	printf("inode%d\n", inode_cnt);
	printdn(dn);
  }

/*
	dn++;
	printf("inode%d\n", 1);
	printdn(dn);
  
	dn++;
	printf("inode%d\n", 2);
	printdn(dn);
  
	dn++;
	printf("inode%d\n", 3);
	printdn(dn);
  
	dn++;
	printf("inode%d\n", 4);
	printdn(dn);
*/  






  return 0;



  
}


void
printdn(struct dinode* dn)
{

  printf("=====================\ndinode info:");
  printf("type = %u\n major = %u\n minor = %u\n nlink = %u\n size = %d\n", dn->type, dn->major, dn->minor, dn->nlink, dn->size);
  printf("Data block addresses(%d entries):\n", NDIRECT+1);

  int i;
  for(i = 0; i < NDIRECT + 1; i++)
  {
	printf("addrs[%d] = %d\n",i, dn->addrs[i]);
  }

  printf("=====================\n");

}
