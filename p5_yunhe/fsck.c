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
#define T_DIR 1
#define T_FILE 2
#define T_DEV 3
#define BLOCKSIZE 512
int fs_size;
int num_inode;
int num_block;
int fs_img_fd;
int find_parent_dir(int inum, struct dinode* inodes);
int link_counter(int inum, struct dinode* inodes);
int set_bit(char *bitmap, int bit, int val){
	if(val == 0){
		bitmap[bit/8] = bitmap[bit/8] & (int)(pow(2,8) -1 - pow(2,(8 - bit%8 +1)));
	}
	else if(val == 1){
		bitmap[bit/8] = bitmap[bit/8] |  (1 << (bit % 8));
	}	
	return 0;
}
int 
read_bit(char *bitmap, int bit){
	return (bitmap[bit/8] & (1 << (bit % 8))) != 0;
}
int superblock_reader(struct superblock *sb){
	//printf("According the superblock, the fs img has %d blocks\n",sb->size);
	//printf("According the superblock, the fs img has %d data blocks\n",sb->nblocks);
	//printf("According the superblock, the fs img has %d inodes\n",sb->ninodes);
	fs_size = sb->size;
	num_inode = sb->ninodes;
	num_block = sb->nblocks;
	//check num_block + num_inode_blocks + bitmap < fs_size
	if( sb->size < sb->nblocks + sb->ninodes*64/BLOCKSIZE + 1){
		fprintf(stderr, "Error\n");
		exit(1);
	}
	struct stat buf;
	fstat(fs_img_fd, &buf);
	sb->size = buf.st_blocks;
	pwrite(fs_img_fd, sb, BSIZE, BSIZE);
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
				//inode.addrs[i]-29;
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
		//bad type ~ delete it
		if(inode.type != T_DIR && inode.type != T_FILE && inode.type != T_DEV){
			inodes[i].type = 0;
		}
		if(inode.type == T_FILE || inode.type == T_DIR){
			if(inode.nlink != link_counter(i,inodes)){
				inodes[i].type = 0;
				//printf("Aha\n");
			}
		}
	}
	pwrite(fs_img_fd, inodes, 200 * sizeof(struct dinode), BSIZE * 2);
	return 0;
}
int dir_checker(struct dinode* inodes){
	int i;
	for(i = 1; i < num_inode; i++){
		struct dinode inode = inodes[i];
		if(inode.type == T_DIR){
			struct dirent entries[32];
			pread(fs_img_fd, &entries, BSIZE, BSIZE * inode.addrs[0]);
			int j;
			for(j = 0; j < 32; j++){
				printf("for the %d entry ", j);
				printf("inode num is %d ,", entries[j].inum);
				printf("name of the file is %s \n", entries[j].name);
			}
			//if(!strcmp(entries[0].name,".")){
				sprintf(entries[0].name, ".");
				entries[0].inum = i;
			//}
			//if(!strcmp(entries[1].name,"..")){
				sprintf(entries[1].name, "..");
				entries[1].inum = find_parent_dir(i,inodes);
			//}
			pwrite(fs_img_fd, &entries, BSIZE, BSIZE * inode.addrs[0]);
			printf("\n");
		}
	}
	return 0;
}
int find_parent_dir(int inum, struct dinode* inodes){
	if(inum == 1)
		return 1;
	int i;
	for(i = 1; i < num_inode; i++){
		struct dinode inode = inodes[i];
		if(inode.type == T_DIR && i != inum){
			struct dirent entries[32];
			pread(fs_img_fd, &entries, BSIZE, BSIZE * inode.addrs[0]);
			int j;
			for(j = 0; j < 32; j++){
				if(entries[j].inum == inum)
					return i;
			}
		}
	}
	return -1;
}
int link_counter(int inum, struct dinode* inodes){
	int i;
	int counter = 0;
	for(i = 1; i < num_inode; i++){
		struct dinode inode = inodes[i];
		if(inode.type == T_DIR){
			struct dirent entries[32];
			pread(fs_img_fd, &entries, BSIZE, BSIZE * inode.addrs[0]);
			int j;
			for(j = 0; j < 32 ; j++){
				if(entries[j].inum == inum && (!strcmp(entries[j].name,".")))
					counter++;
			}
		}
	}
	return counter;
}
int main(int argc, char *argv[]){
	assert((fs_img_fd = open (argv[1], O_RDWR)) > 0);
	assert(lseek(fs_img_fd, BSIZE, SEEK_SET) >= 0);
  	struct superblock sb;
	assert(read(fs_img_fd, &sb, sizeof(struct superblock)) ==  sizeof(struct superblock));
	superblock_reader(&sb);

	struct dinode inodes[200];
	assert(lseek(fs_img_fd, BSIZE * 2, SEEK_SET) > 0);
	assert(read(fs_img_fd, inodes, 200 * sizeof(struct dinode)) == 200 * sizeof(struct dinode));
	char bitmap_from_inode[BLOCKSIZE];
	bitmap_generator(inodes,bitmap_from_inode);

	char bitmap[BLOCKSIZE];
 	assert(lseek(fs_img_fd, BSIZE * 28, SEEK_SET) > 0);
	assert(read(fs_img_fd, bitmap, BSIZE) ==  BSIZE);

	int i;
	for(i = 0; i < fs_size; i++){
		printf("%d",read_bit(bitmap,i));
	}
	printf("\n");
	dir_checker(inodes);
	inodes_checker(inodes);
 	return 0;
}


