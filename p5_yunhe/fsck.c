#include <stdio.h>
#include <stdlib.h>
#include "fs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#define BLOCKSIZE 512
int fs_size;
int num_inode;
int num_block;
int set_bit(char *bitmap, int bit, int val){
	if(val == 0){
		bitmap[bit/8] = bitmap[bit/8] & (int)(pow(2,8) -1 - pow(2,(8 - bit%8 +1)));
	}
	else if(val == 1){
		bitmap[bit/8] = bitmap[bit/8] |  (1 << (7 - bit % 8));
	}	
	return 0;
}
int 
read_bit(char *bitmap, int bit){
	return (bitmap[bit/8] & (1 << (7 - bit % 8))) > 0;
}
int superblock_reader(struct superblock *sb){
	printf("According the superblock, the fs img has %d blocks\n",sb->size);
	printf("According the superblock, the fs img has %d data blocks\n",sb->nblocks);
	printf("According the superblock, the fs img has %d inodes\n",sb->ninodes);
	fs_size = sb->size;
	num_inode = sb->ninodes;
	num_block = sb->nblocks;
	//check num_block + num_inode_blocks + bitmap < fs_size
	if( sb->size < sb->nblocks + sb->ninodes*64/BLOCKSIZE + 1){
		fprintf(stderr, "Superblock has been corrupted\n");
		exit(1);
	}
	return 0;
}
int bitmap_generator(struct dinode *inodes, char *bitmap){
	// Init bitmap
	memset(bitmap,0,BLOCKSIZE/8);
	//traverse the list of inodes, but skip the first one
	int i;
	for(i = 1; i < 200; i++){
		struct dinode inode = inodes[i];
		if(inode.type == 1 || inode.type == 2){
			//TODO
			printf("Currently check the inode #%d, type is %d, and size is %d\n",i,inode.type,inode.size);
			int j;
			for(j = 0; j < NDIRECT + 1; j++){
				//inode.addrs[i]-DATA_BLK_STR_ADDR
			}
		}
	}
	return 0;
}
int inodes_checker(struct dinode* inodes){
	//TODO check the data filed in each inode, skip the first one
	int i;
	for(i = 1; i < num_inode; i++){
		struct dinode inode = inodes[i];
		if(inode.type != 0){
			//check size
			//if(inode.size * 1.0 / BLOCKSIZE > num_block)
		}
	}
	
}
int main(int argc, char *argv[]){
	int fd = open ("fs.img", O_RDWR);
	assert(fd > 0);
	assert(lseek(fd, BSIZE, SEEK_SET) >= 0);
  	struct superblock sb;
	assert(read(fd, &sb, sizeof(struct superblock)) ==  sizeof(struct superblock));
	superblock_reader(&sb);

	struct dinode inodes[200];
	assert(lseek(fd, BSIZE * 2, SEEK_SET) > 0);
	assert(read(fd, inodes, 200 * sizeof (struct dinode)) == 200 * sizeof (struct dinode));
	char bitmap_from_inode[BLOCKSIZE];
	bitmap_generator(inodes,bitmap_from_inode);

	char bitmap[BLOCKSIZE];
 	assert(lseek(fd, BSIZE * 28, SEEK_SET) > 0);
	assert(read(fd, bitmap, BSIZE) ==  BSIZE);

	int i;
	for(i = 0; i < fs_size; i++){
		printf("%d",read_bit(bitmap,i));
	}
	printf("\n");
	inodes_checker(inodes);
 	return 0;
}


