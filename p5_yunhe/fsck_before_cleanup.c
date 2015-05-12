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
int fs_size;
int num_inode;
int num_block;
int fs_img_fd;
int dir_checker(struct dinode* inodes);
int superblock_reader(struct superblock *sb);
int find_parent_dir(int inum, struct dinode* inodes);
int link_counter(int inum, struct dinode* inodes);
int inodes_checker(struct dinode* inodes);
int set_bit(char *bitmap, int bit, int val);
int read_bit(char *bitmap, int bit);
int bitmap_generator(struct dinode *inodes, char *bitmap);
int delete_entry_with_inode(int inum, struct dinode* inodes);
int 
main(int argc, char *argv[]){
	assert((fs_img_fd = open (argv[1], O_RDWR)) > 0);
	assert(lseek(fs_img_fd, BSIZE, SEEK_SET) >= 0);
  	struct superblock sb;
	assert(read(fs_img_fd, &sb, sizeof(struct superblock)) ==  sizeof(struct superblock));
	superblock_reader(&sb);

	struct dinode inodes[200];
	assert(lseek(fs_img_fd, BSIZE * 2, SEEK_SET) > 0);
	assert(read(fs_img_fd, inodes, 200 * sizeof(struct dinode)) == 200 * sizeof(struct dinode));
	char bitmap_from_inode[BSIZE];
	bitmap_generator(inodes,bitmap_from_inode);
	dir_checker(inodes);
	inodes_checker(inodes);
 	
	char bitmap[BSIZE];
 	assert(lseek(fs_img_fd, BSIZE * 28, SEEK_SET) > 0);
	assert(read(fs_img_fd, bitmap, BSIZE) ==  BSIZE);
	
	/**int i;
	printf("on disk bitmap: \n");
        for(i = 0; i < fs_size; i++){
                printf("%d",read_bit(bitmap,i));
        }
	printf("\n");
	printf("bitmap build from inode info: \n");
	for(i = 0; i < fs_size; i++){
                printf("%d",read_bit(bitmap_from_inode,i));
        }
	printf("\n");
	**/
	if(memcmp(bitmap,bitmap_from_inode,BSIZE)){
		printf("repairing bitmap~\n");
		//write the correct version bitmap to the fs img
		assert(pwrite(fs_img_fd, bitmap_from_inode, BSIZE, BSIZE * 28) == BSIZE);
	}	
	return 0;
}

int 
set_bit(char *bitmap, int bit, int val){
	//TODO
	if(val == 0){
		bitmap[bit/8] = bitmap[bit/8] & (int)(pow(2,8) -1 - pow(2,(8 - bit%8 +1)));
		//NEED TO FIX, not working
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
int
superblock_reader(struct superblock *sb){
	fs_size = sb->size;
	num_inode = sb->ninodes;
	num_block = sb->nblocks;
	//check num_block + num_inode_blocks + bitmap < fs_size
	if( sb->size < sb->nblocks + sb->ninodes*64/BSIZE + 1){
		fprintf(stderr, "Error\n");
		exit(1);
	}
	struct stat buf;
	fstat(fs_img_fd, &buf);
	sb->size = buf.st_blocks;
	pwrite(fs_img_fd, sb, BSIZE, BSIZE);
	return 0;
}
int 
dir_checker(struct dinode* inodes){
	int i;
	for(i = 1; i < num_inode; i++){
		struct dinode inode = inodes[i];
		if(inode.type == T_DIR){
			int num_ent_block = inode.size / BSIZE;
			if(inode.size % BSIZE != 0)
				num_ent_block++;
			struct dirent entries[32*num_ent_block];
			int n;
			for(n = 0; n < num_ent_block; n++){
				assert(pread(fs_img_fd, &entries[32*n], BSIZE, BSIZE * inode.addrs[n]) == BSIZE);
			}
			int j;
			for(j = 0; (j < 32 * num_ent_block) && entries[j].inum != 0; j++){
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
int
find_parent_dir(int inum, struct dinode* inodes){
	if(inum == 1)
		return 1;
	int i;
	for(i = 1; i < num_inode; i++){
		struct dinode inode = inodes[i];
		if(inode.type == T_DIR && i != inum){
			int num_ent_block = inode.size / BSIZE;
			if(inode.size % BSIZE != 0)
				num_ent_block++;
			struct dirent entries[32*num_ent_block];
			int n;
			for(n = 0; n < num_ent_block; n++){
				pread(fs_img_fd, &entries[32*n], BSIZE, BSIZE * inode.addrs[n]);
			}
			int j;
			for(j = 0; (j < 32 * num_ent_block) && entries[j].inum != 0; j++){
				if(entries[j].inum == inum)
					return i;
			}
		}
	}
	return -1;
}
int
delete_entry_with_inode(int inum, struct dinode* inodes){
	int i;
	for(i = 1; i < num_inode; i++){
		struct dinode inode = inodes[i];
		if(inode.type == T_DIR && i != inum){
			int num_ent_block = inode.size / BSIZE;
			if(inode.size % BSIZE != 0)
				num_ent_block++;
			struct dirent entries[32*num_ent_block];
			int n;
			for(n = 0; n < num_ent_block; n++){
				assert(pread(fs_img_fd, &entries[32*n], BSIZE, BSIZE * inode.addrs[n]) == BSIZE);
			}
			int j;
			for(j = 0; (j < 32 * num_ent_block) && entries[j].inum != 0; j++){
				if(entries[j].inum == inum){
					memset(&(entries[j]),0,sizeof(struct dirent));
					for(n = 0; n < num_ent_block; n++){
						assert(pwrite(fs_img_fd, &entries[32*n], BSIZE, BSIZE * inode.addrs[n]) == BSIZE);
					}
					return 0;
				}
			}
		}
	}
	return -1;
}
int 
inodes_checker(struct dinode* inodes){
	//TODO check the data filed in each inode, skip the first one
	int i;
	for(i = 1; i < num_inode; i++){
		struct dinode inode = inodes[i];
		if(inode.type != T_DIR && inode.type != T_FILE && inode.type != T_DEV){
			inodes[i].type = 0;
			delete_entry_with_inode(i,inodes);
		}
		if(inode.type == T_FILE || inode.type == T_DIR){
			//printf("For the %d inode ", i);
			//printf("inode says~ link count is %d || ", inode.nlink);
			//printf(" link_counter says~ link count is %d\n", link_counter(i,inodes));
			int counted_link = link_counter(i,inodes);
			if(inode.nlink != counted_link){	
				if(counted_link == 0){
					printf("At least I'm trying to fix\n");
					int num_ent_block = inodes[1].size / BSIZE;
					if(inodes[1].size % BSIZE != 0)
						num_ent_block++;
					struct dirent entries[32*num_ent_block];
					printf("how many Root DIRENT? %d \n",32*num_ent_block);
					int n;
					for(n = 0; n < num_ent_block; n++){
						pread(fs_img_fd, &entries[32*n], BSIZE, BSIZE * inodes[1].addrs[n]);
					}
					int j;
					for(j = 0; (j < 32 * num_ent_block) && entries[j].inum != 0;  j++){
						if(!strcmp(entries[j].name,"lost+found")){
							struct dirent entries_x[32];
							assert(pread(fs_img_fd,entries_x,BSIZE,BSIZE * (inodes[entries[j].inum].addrs[0])) == BSIZE);
							inodes[entries[j].inum].size += sizeof(struct dirent);
							int k;
							for(k = 0; k < 32; k++){
								printf("Aha, inside lost+found %d entry\n", k);
								if(entries_x[k].inum == 0){
									entries_x[k].inum = i;
									sprintf(entries_x[k].name, "lost_%d",i);
									break;
								}
							}
							assert(pwrite(fs_img_fd,entries_x,BSIZE,BSIZE * inodes[entries[j].inum].addrs[0]) == BSIZE);
							//after fix check
							struct dirent eee[32];
							assert(pread(fs_img_fd,eee,BSIZE,BSIZE * inodes[entries[j].inum].addrs[0]) == BSIZE);
							int x;
							for(x=0; x<32  && eee[x].inum != 0;x++){
								printf("name: %s , inum %d \n",eee[x].name,eee[x].inum);
							}


						}	
					}
				}
				else{
					inodes[i].nlink = counted_link;
					//printf("Aha\n");
				}
			}
		}
	
	}
	pwrite(fs_img_fd, inodes, 200 * sizeof(struct dinode), BSIZE * 2);
	return 0;
}
int 
link_counter(int inum, struct dinode* inodes){
	int i;
	int counter = 0;
	for(i = 1; i < num_inode; i++){
		struct dinode inode = inodes[i];
		if(inode.type == T_DIR){
			int num_ent_block = inode.size / BSIZE;
			if(inode.size % BSIZE != 0)
				num_ent_block++;
			struct dirent entries[32*num_ent_block];
			int n;
			for(n = 0; n < num_ent_block; n++){
				pread(fs_img_fd, &entries[32*n], BSIZE, BSIZE * inode.addrs[n]);
			}

			int j;
			for(j = 0; (j < 32 * num_ent_block) && entries[j].inum != 0;  j++){
				if((entries[j].inum == inum) && (strcmp(entries[j].name,"."))){
					counter++;
					//printf("increament counter beacuse at dir with inum %d found an entry with name %s with inum %d\n",i,entries[j].name,inum);
				}
			}
		}
	}
	return counter;
}
int
 bitmap_generator(struct dinode *inodes, char *bitmap){
	// Init bitmap
	memset(bitmap,0,BSIZE);
	int i;
	for(i = 0; i < 29; i++){
		set_bit(bitmap,i,1);
	} 
	//traverse the list of inodes, but skip the first one
	
	for(i = 1; i < 200; i++){
		struct dinode inode = inodes[i];
		if(inode.type == 1 || inode.type == 2){
			//TODO
			//printf("Currently check the inode #%d, type is %d, and size is %d\n",i,inode.type,inode.size);
			int j;
			for(j = 0; j < NDIRECT + 1 && inode.addrs[j] != 0; j++){
				//printf("inode #%d address %d : %d\n",i,j,inode.addrs[j]);
				set_bit(bitmap,inode.addrs[j],1);
				if(j == NDIRECT){
					//printf("HEY HEY\n");
					//printf("THE NUMBRE IS %d \n",BSIZE/sizeof(uint));
					uint addrs[BSIZE/sizeof(uint)];
					assert(pread(fs_img_fd, addrs, BSIZE, BSIZE * inode.addrs[NDIRECT]) == BSIZE);
					int k;
					for(k = 0; k < (BSIZE/sizeof(uint)) && addrs[k] != 0; k++){
						//printf("inode #%d address %d : %d\n",i,j+1+k,addrs[k]);
						set_bit(bitmap,addrs[k],1);
					}
				}
			}
		}
	}
	return 0;
}
