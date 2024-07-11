#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define EOC 0xFFFF

/*build structure for all data structures*/
struct SuperBlock{
	char signature[8];
	uint16_t blocknumber;
	uint16_t rootindex;
	uint16_t datablockindex;
	uint16_t datablocknumber;
	uint8_t FATblocks;
	uint8_t unused[4079];
}__attribute__((packed));

struct FAT{
	uint16_t fat_entry;
}__attribute__((packed));

struct RootDirectoryEntry{
	char FileName[16];
	uint32_t filesize;
	uint16_t indexfirst;
	uint8_t unused[10];
}__attribute__((packed));

struct file_descriptor{
	size_t offset;
	const char* FileName;
	int index;
}__attribute__((packed));

/*some global variables*/
struct SuperBlock superblock;
struct RootDirectoryEntry Root_directory[128];
struct FAT *fat_block;
int root_entry = 0;
int mounted=0;
struct file_descriptor filedescriptor[FS_OPEN_MAX_COUNT];


static int fat_find_entry(uint16_t value)
{
	int i = 0;
	for (i = 0; i < superblock.datablocknumber; i++) {
		if (fat_block[i].fat_entry == value) {
			return i;
		}
	}
	return -1;
}

int fs_mount(const char *diskname)
{
	/* mount, initialize the block */
	int vdisk = block_disk_open(diskname);
	if(vdisk == -1){
		return -1;
	}

	if((block_read(0,&superblock)) == -1){
		return -1;
	}

	if(strncmp(superblock.signature,"ECS150FS",8)!=0){
		return -1;
	}

	if(superblock.blocknumber!=block_disk_count()){
		return -1;
	}

	fat_block = malloc(BLOCK_SIZE * superblock.FATblocks);
	for(int count = 0;count<superblock.FATblocks;count++){
		if((block_read(1+count,(uint8_t*)fat_block+BLOCK_SIZE*count)) == -1){
			return -1;
		}
	}

	if((block_read(superblock.rootindex,Root_directory)) == -1){
		return -1;
	}
	
	mounted = 1;
    return 0;
}

int fs_umount(void)
{
	/* unmount, error handling*/
	if((block_write(0,&superblock)) == -1){
		return -1;
	}

	for(int count = 0;count<superblock.FATblocks;count++){
		if((block_write(1+count,(void*)fat_block+BLOCK_SIZE*count)) == -1){
			return -1;
		}
	}

	if((block_write(superblock.rootindex,(void*)Root_directory)) == -1){
		return -1;
	}

	if(block_disk_close() == -1){
		return -1;
	}
    return 0;
}

int fs_info(void)
{
	/* get data info, the free datablocks and the free root directory*/
    int free_datablock = 0;
	for(int count = 0; count<superblock.datablocknumber; count++){
		if(fat_block[count].fat_entry==0){
			free_datablock++;
		}
	}
	int root_num = 0;
	for(int count = 0; count<FS_FILE_MAX_COUNT;count++){
		if(Root_directory[count].FileName[0]==0){
			root_num++;
		}
	}
	/*print the result*/
	printf("FS Info:\n");
	printf("total_blk_count=%d\n",superblock.blocknumber);
	printf("fat_blk_count=%d\n",superblock.FATblocks);
	printf("rdir_blk=%d\n",superblock.rootindex);
	printf("data_blk=%d\n",superblock.datablockindex);
	printf("data_blk_count=%d\n",superblock.datablocknumber);
	printf("fat_free_ratio=%d/%d\n",free_datablock,superblock.datablocknumber);
	printf("rdir_free_ratio=%d/%d\n",root_num,128);
    return 0;
}

int fs_create(const char *filename)
{
	/* create the file info in root directory*/
    int count = 0;
	while(count<128){
		if(Root_directory[count].FileName[0]==0){
			strcpy(Root_directory[count].FileName,filename);
			Root_directory[count].filesize = 0;
			Root_directory[count].indexfirst = EOC;
			root_entry++;
			return 0;
			break;
		}
		count++;
	}
	return -1;

}

int fs_delete(const char *filename)
{
	/*delete the file info from root directory*/
	int index = -1;
	uint16_t fat_entry;
	for(int count = 0;count<128;count++){
		if(strncmp(Root_directory[count].FileName,filename,16)==0){
			index = Root_directory[count].indexfirst;
			Root_directory[count].indexfirst = EOC;
			Root_directory[count].filesize = 0;
			Root_directory[count].FileName[0] = '\0';
			root_entry--;
			break;
		}
	}
	if(index<0){
		return -1;
	}
	if(index==EOC){
		return 0;
	}
	fat_entry = fat_block[index].fat_entry;
	/*get the first index value from the fat entry, and get the next block index of this file. Free the current datablock and set the corresponding fat entry to 0*/
	while(fat_entry!=EOC){
		fat_block[index].fat_entry = 0;
		index = fat_entry;
		fat_entry = fat_block[index].fat_entry;
	}
	fat_block[index].fat_entry = 0;

	return 0;
	
}

int fs_ls(void)
{
	 /*list the file info by printf*/
	 for(int count = 0; count<FS_FILE_MAX_COUNT;count++){
		if(Root_directory[count].FileName[0]!=0){
			printf("file: %s,size:%d,data_blk:%d\n",Root_directory[count].FileName,Root_directory[count].filesize,Root_directory[count].indexfirst);
		}
	}
	return 0;
	
}

int fs_open(const char *filename)/*open the file*/
{   
    /* basic error handling*/
	if (mounted == 0){
		return -1;
	}
	if (!filename){
		return -1;
	}

	int F_index=-1;
	/*find filename in root directory*/
	for(int count = 0; count < FS_FILE_MAX_COUNT; count++) {
        if(strncmp(Root_directory[count].FileName,filename,16)==0){
        	F_index = count;
        	break;
        }
    }
	if(F_index<0){
		return -1;
	}
	/*find the first NULL space in file_descriptor list and store the filenmae*/
	for (int count=0; count< FS_OPEN_MAX_COUNT; count++){
    	if (filedescriptor[count].FileName==NULL){
			filedescriptor[count].FileName=filename;
            filedescriptor[count].offset=0;
            filedescriptor[count].index=F_index;
            F_index=count;
            break;
            }
        }
	return F_index;
}

int fs_close(int fd)
{
	/* close the file, basic error handling*/
    if (mounted == 0){
		return -1;
	}
	if(fd>FS_OPEN_MAX_COUNT || fd<0){
	    return -1;
	}
	filedescriptor[fd].FileName=NULL;
	filedescriptor[fd].offset=0;
	filedescriptor[fd].index=0;
	return 0;
	
}

int fs_stat(int fd)
{
	/* Get the current size of the file pointed by file descriptor, basic error handling*/
    if (mounted == 0){
		return -1;
	}
	if(fd>FS_OPEN_MAX_COUNT || fd<0){
	    return -1;
	}

	int count = 0;
	while(count < FS_FILE_MAX_COUNT){
	    if (!strcmp(Root_directory[count].FileName, filedescriptor[fd].FileName)) {
            return Root_directory[count].filesize;
        }
		count++;
	}
	return -1;

}

int fs_lseek(int fd, size_t offset)
{
	/*Set the file offset (used for read and write operations) associated with file descriptor @fd to the argument @offset.*/
    if (mounted == 0){
		return -1;
	}
	 if(fd>FS_OPEN_MAX_COUNT || fd<0){
	    return -1;
	}
	filedescriptor[fd].offset=offset;
	return 0;

}

int fs_write(int fd, void *buf, size_t count)
{
	
	int merge_last_block = 0;
	if (filedescriptor[fd].offset + count < Root_directory[filedescriptor[fd].index].filesize) {
		merge_last_block = 1;/*when we donot need to reach the end of the file*/
	}

	int start_block = filedescriptor[fd].offset / BLOCK_SIZE;
	int start_block_offset = filedescriptor[fd].offset % BLOCK_SIZE;

	int tmp_buf_length = (start_block_offset + count + BLOCK_SIZE) / BLOCK_SIZE * BLOCK_SIZE; /*to create another buffer that can copy all of the whole blocks*/
	uint8_t *tmp_buf = malloc(tmp_buf_length);

	int num = 0;/*to count the blocks*/
	uint16_t fat_index = Root_directory[filedescriptor[fd].index].indexfirst;
	int success = 1;
	for (num = 0; ; ++num, fat_index = fat_block[fat_index].fat_entry) {
		if (num == start_block) {
			block_read(superblock.datablockindex + fat_index, tmp_buf);
			memcpy(tmp_buf + start_block_offset, buf, count);/*copy the whole  blocks except for the last one to tmp_buf*/
		}
		if (num >= start_block) {
			if (merge_last_block == 1) {
				if (fat_block[fat_index].fat_entry == EOC) {
					block_read(superblock.datablockindex + fat_index, tmp_buf + tmp_buf_length - BLOCK_SIZE);
					memcpy(tmp_buf + start_block_offset, buf, count);/*copy from start to last*/
				}
			}

			block_write(superblock.datablockindex + fat_index, tmp_buf + BLOCK_SIZE * (num - start_block));/*write to space*/
			
			if (tmp_buf_length > BLOCK_SIZE * (num + 1 - start_block)) {
				if (fat_block[fat_index].fat_entry == EOC) {
					int fat_next_index = fat_find_entry(0);/*arrange next available block*/
					if (fat_next_index == -1) {
						success = 0;
						break;
					} else {
						fat_block[fat_index].fat_entry = fat_next_index;
					}
				}
			} else {
				break;
			}
		}
	}
	if (merge_last_block != 1) {
		Root_directory[filedescriptor[fd].index].filesize = filedescriptor[fd].offset + count;
	}

	if (success == 1) {
		filedescriptor[fd].offset += count;
		return count;/*enough space for count, return*/
	} else {
		filedescriptor[fd].offset += BLOCK_SIZE * (num + 1 - start_block) - start_block_offset;
		return BLOCK_SIZE * (num + 1 - start_block) - start_block_offset;
	}/*no enough space for size count */
}

int fs_read(int fd, void *buf, size_t count)
{
	if(mounted==0||fd<0){
		return -1;
	}
    /* check if file opened*/
    if(filedescriptor[fd].FileName==NULL){
        return -1;
    }
    /* check if fd out of bounds  */
    if (fd >FS_OPEN_MAX_COUNT){
        return -1;
    }
    
    size_t offset = filedescriptor[fd].offset;     
    /* find index of first block  */
    int first_index = Root_directory[filedescriptor[fd].index].indexfirst;
    int file_size = Root_directory[filedescriptor[fd].index].filesize;
    int num_block=file_size/BLOCK_SIZE;
    char bounce[BLOCK_SIZE];    
    size_t pos = offset % BLOCK_SIZE;  //find start position
    int read_length = 0;
    /* for small operation */
    if(num_block<=1){
      /* start reading data  */
    	for(unsigned block_index = 0;block_index<count;block_index++){
                
			if (block_read(first_index + superblock.datablockindex, bounce)) {
				return -1;
			}
					//copy data in to buf
			memcpy(buf, &bounce[pos+block_index],1);
		
			buf++;
			offset++;
					
       }

        first_index = fat_block[first_index].fat_entry;
        read_length = offset-filedescriptor[fd].offset;
        filedescriptor[fd].offset = offset;   //update 
        }
     else{
         /* for big operation */
        int first_block_remain=BLOCK_SIZE-pos;      
        if (block_read(first_index + superblock.datablockindex, bounce)) {
            return -1;
            }
                //copy data in to buf
            memcpy(buf+first_block_remain, &bounce[pos],first_block_remain);
            offset+=first_block_remain;
            int next_block=fat_block[first_index].fat_entry;
                
                /* start reading next block */
            for(int block_index = 1;block_index<num_block;block_index++){                   
                if (block_read(next_block + superblock.datablockindex, bounce)) {
                        return -1;
                }
                if(block_index!=num_block-1){
                        memcpy(buf+next_block, &bounce,BLOCK_SIZE);
                        offset+=BLOCK_SIZE;
                }
                else if(block_index==num_block-1){
                        /* reading data of last block */
                        int remaining=file_size-first_block_remain- ((num_block-2)*BLOCK_SIZE);
                        memcpy(buf+next_block, &bounce,remaining);
                        offset+=remaining;
                    
                }
                                      
                if(fat_block[next_block].fat_entry!=EOC){
                        next_block= fat_block[next_block].fat_entry;
                }
                   
                    
                }
        }   
        return read_length;   

}

