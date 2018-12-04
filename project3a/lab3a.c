/*
NAME: Alejandra Cervantes,Ryan Miyahara
EMAIL: alecer@ucla.edu,rmiyahara144@gmail.com
ID: 104844623,804585999
*/
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ext2_fs.h"
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

//Global Variables
int mount_fd;
int superblock_offset = 1024;
struct ext2_super_block thuper;
__uint32_t block_count;
__uint32_t block_size;
__uint32_t inode_count;
__uint32_t inode_size;
struct ext2_group_desc groupie;
struct ext2_inode inode;
struct ext2_dir_entry dir;

void print_error_and_exit(char *msg){
  fprintf(stderr, "ERROR: %s\nErrno reports: %s\n Error number: %d", msg, strerror(errno), errno);
  exit(2);
}

char* format_time(__uint32_t clock){
    time_t casted_time = clock;
    struct tm* time = gmtime(&casted_time);
    char* ans = (char*)malloc(sizeof(char) * 128);
    //strftime function help here: http://man7.org/linux/man-pages/man3/strftime.3.html
    strftime(ans, 128, "%m/%d/%y %H:%M:%S", time);
    return ans;
}

void matryoshka_baby(__uint32_t parent, __uint32_t loop, __uint32_t offset) {
    __uint32_t i;
    for (i = 0; i < loop; i++) {
        int hold_me;
        if (pread(mount_fd, &hold_me, 4, offset + i * 4) < 0)
            print_error_and_exit("Unable to pread for directory entries.");
        if (hold_me) {
            __uint32_t curr_offset = block_size * hold_me;
            __uint32_t init_offset = curr_offset;
            while (curr_offset < init_offset + block_size) {
                if (pread(mount_fd, &dir, sizeof(struct ext2_dir_entry), curr_offset) < 0)
                    print_error_and_exit("Unable to pread for directory entries.");
                __uint32_t inode_num = dir.inode;
                __uint32_t name_len = dir.name_len;
                __uint32_t rec_len = dir.rec_len;
                __uint32_t diff = curr_offset - init_offset;
                curr_offset += rec_len;
                if (!(inode_num)) continue;
                fprintf(stdout, "DIRENT,%u,%u,%u,%u,%u,'", parent, diff, inode_num, rec_len, name_len);
                __uint32_t j;
                for (j = 0; j < name_len; j++)
                    fprintf(stdout, "%c", dir.name[j]);
                fprintf(stdout, "'\n");
            }
        }
    }
    return;
}

void matryoshka_mother(__uint32_t parent, __uint32_t offset, int level) {
    int hold_me;
    if (pread(mount_fd, &hold_me, 4, offset) < 0)
        print_error_and_exit("Unable to pread for directory entries.");
    if (hold_me && !(level))
        matryoshka_baby(parent, block_size / 4, block_size * hold_me);
    else if (hold_me) {
        __uint32_t i;
        for (i = 0; i < block_count / 4; i++)
            matryoshka_mother(parent, hold_me * block_size + i * 4, level--);
    }
    return;
}

void indirect_block_reference0(__uint32_t block_num, __uint32_t inode_num) { //TODO
  __uint32_t offset = block_num * block_size;
  __uint32_t block_value; 
  __uint32_t i;
  for (i = 0; i < block_size / 4; i++){
    if (pread(mount_fd, &block_value, sizeof(block_value), offset + i*4) < 0)
      print_error_and_exit("Unable to pread for indirect level 1");
    if (block_value)
      fprintf(stdout, "INDIRECT,%u,%u,%u,%u,%u\n", inode_num, 1, i + 12, block_num, block_value);
  }
    return;
}

void indirect_block_reference1(__uint32_t block_num, __uint32_t inode_num) { //TODO
  __uint32_t offset_1 = block_num * block_size;
  __uint32_t offset, block_value_1, block_value, i, j; 
  for (i = 0; i < block_size / 4; i++){
    if (pread(mount_fd, &block_value_1, sizeof(block_value_1), offset_1 + i * 4) < 0)
      print_error_and_exit("Unable to pread for indirect level 2");

    offset = block_value_1 * block_size; 
    
    if (block_value_1){
      fprintf(stdout, "INDIRECT,%u,%u,%u,%u,%u\n", inode_num, 2,  12 + block_size / 4 + i * block_size / 4,
	      block_num, block_value_1);
      
      for(j = 0; j < block_size / 4; j++){
	if (pread(mount_fd, &block_value, sizeof(block_value), offset + j * 4) < 0)
	  print_error_and_exit("Unable to pread for indirect level 2.2");

	if (block_value)
	  fprintf(stdout, "INDIRECT,%u,%u,%u,%u,%u\n", inode_num, 1, 
		  12 + block_size / 4 + i * block_size / 4 + j, block_value_1, block_value);
      }
    }
    
  }

    return;
}

void indirect_block_reference2(__uint32_t block_num, __uint32_t inode_num) {
    __uint32_t offset2 = block_num * block_size;
    __uint32_t i;
    for (i = 0; i < block_size / 4; i++) {
        __uint32_t block_num1;
        if (pread(mount_fd, &block_num1, sizeof(__uint32_t), offset2 + i * 4) < 0)
            print_error_and_exit("Unable to pread for indirect block references.");
        __uint32_t offset1 = block_num1 * block_size;
        if (block_num1) {
            __uint32_t block_passed = (block_size * block_size / 16) + (block_size / 4) + 12 + i * (block_size * block_size / 8);
            fprintf(stdout, "INDIRECT,%u,3,%u,%u,%u\n", inode_num, block_passed, block_num, block_num1);
            __uint32_t j;
            for (j = 0; j < block_size / 4; j++) {
                __uint32_t block_num0;
                if (pread(mount_fd, &block_num0, sizeof(__uint32_t), offset1 + j * 4) < 0)
                    print_error_and_exit("Unable to pread for indirect block references.");
                __uint32_t offset0 = block_num0 * block_size;
                if (block_num0) {
                    fprintf(stdout, "INDIRECT,%u,2,%u,%u,%u\n", inode_num, block_passed + j * block_size / 4, block_num1, block_num0);
                    __uint32_t k;
                    for (k = 0; k < block_size / 4; k++) {
                        __uint32_t block_rip;
                        if (pread(mount_fd, &block_rip, sizeof(__uint32_t), offset0 + k * 4) < 0)
                            print_error_and_exit("Unable to pread for indirect block references.");
                        if (block_rip)
                            fprintf(stdout, "INDIRECT,%u,1,%u,%u,%u\n", inode_num, block_passed + j * block_size / 4 + k, block_num0, block_rip);
                    }
                }
            }
        }
    }
    
    return;
}

void directory_entries(__uint32_t parent, __uint32_t offset) {
    matryoshka_baby(parent, 12, offset + 40);
    matryoshka_mother(parent, offset + 40 + 48, 0);
    matryoshka_mother(parent, offset + 40 + 52, 1);
    matryoshka_mother(parent, offset + 40 + 56, 2);
    return;
}
void superblock_summary() {
    if (pread(mount_fd, &thuper, sizeof(struct ext2_super_block), superblock_offset) < 0)
      print_error_and_exit("Unable to pread for superblock.");

    block_count = thuper.s_blocks_count;
    inode_count = thuper.s_inodes_count;
    block_size = EXT2_MIN_BLOCK_SIZE << thuper.s_log_block_size;
    inode_size = thuper.s_inode_size;
    fprintf(stdout, "SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", block_count, inode_count, block_size, inode_size, thuper.s_blocks_per_group, thuper.s_inodes_per_group, thuper.s_first_ino);
    return;
}

void group_summary() {
    if (pread(mount_fd, &groupie, sizeof(struct ext2_group_desc), superblock_offset + sizeof(struct ext2_super_block)) < 0)
      print_error_and_exit("Unable to pread for group."); 

    fprintf(stdout, "GROUP,0,%u,%u,%u,%u,%u,%u,%u\n", block_count, inode_count, groupie.bg_free_blocks_count, groupie.bg_free_inodes_count, groupie.bg_block_bitmap, groupie.bg_inode_bitmap, groupie.bg_inode_table);
    return;
}

void free_check(__uint32_t bitmap, __uint32_t count, char xdlmao) {
    __uint32_t i;
    for (i = 0; i < count; i++) {
        unsigned char offset = (i >> 3) & 255;
        unsigned char buffer;
        if (pread(mount_fd, &buffer, sizeof(unsigned char), bitmap * block_size + offset) < 0)
            print_error_and_exit("Unable to pread for free check.");

        bool not_free = ((buffer >> (i & 7)) & 1);
        if (!(not_free))
            fprintf(stdout, "%cFREE,%d\n", xdlmao, i + 1);
    }
    return;
}

void inode_summary(__uint32_t count){
    __uint32_t i;
    for (i = 0; i < count; i++){
        unsigned char offset = (i >> 3) & 255;
        unsigned char buffer;
        if (pread(mount_fd, &buffer, sizeof(unsigned char), groupie.bg_inode_bitmap * block_size + offset) < 0)
            print_error_and_exit("Unable to pread inode summary."); 

        bool not_free = ((buffer >> (i & 7)) & 1); 
        if (!(not_free))
            (void) i; 

        if (pread(mount_fd, &inode, inode_size, 1024 + (groupie.bg_inode_table - 1) * block_size + i * sizeof(struct ext2_inode)) < 0)
            print_error_and_exit("Unable to pread inode summary 2."); 

        char file_type = '?'; 
        if (inode.i_mode == 0 || inode.i_links_count == 0) continue; 
        if ((inode.i_mode & 0xF000) == 0xA000) file_type = 's';
        else if ((inode.i_mode & 0xF000) == 0x8000) file_type = 'f';
        else if ((inode.i_mode & 0xF000) == 0x4000) file_type = 'd';

        char* c_time = format_time(inode.i_ctime);
        char* m_time = format_time(inode.i_mtime);
        char* a_time = format_time(inode.i_atime);
        fprintf(stdout, "INODE,%d,%c,%o,%u,%u,%u,%s,%s,%s,%d,%d", 
            i+1, file_type, (inode.i_mode & 0xFFF), inode.i_uid, inode.i_gid, 
            inode.i_links_count, c_time, m_time,
            a_time, inode.i_size, inode.i_blocks);
        free (c_time);
        free (m_time);
        free (a_time);

        if (!(file_type == 's' && inode.i_size < 60)) {
            __uint32_t j;
            for (j = 0; j < 15; j++)
                fprintf(stdout, ",%u", inode.i_block[j]);
        }
        else fprintf(stdout, ",%u", inode.i_block[0]);
        fprintf(stdout, "\n");

        if (file_type == 'd')
            directory_entries(i + 1, block_size * (groupie.bg_inode_table - 1) + i * (sizeof(struct ext2_inode)) + 1024);

        if (!(file_type == 's' && inode.i_size < 60)){
            if (inode.i_block[12]) indirect_block_reference0(inode.i_block[12], i + 1);
            if (inode.i_block[13]) indirect_block_reference1(inode.i_block[13], i + 1);
            if (inode.i_block[14]) indirect_block_reference2(inode.i_block[14], i + 1);
        }
    }
    return;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Incorrect usage of the program. Please use the program in the following format: ./lab3a FILE_SYSTEM_IMAGE\n");
        exit(1);
    }
    
    mount_fd = open(argv[1], O_RDONLY);
    if (mount_fd < 0) {
        fprintf(stderr, "Invalid argument.\n");
        exit(1);
    }
      
    superblock_summary();
    group_summary();
    free_check(groupie.bg_block_bitmap, block_count, 'B');
    free_check(groupie.bg_inode_bitmap, inode_count, 'I');
    inode_summary(inode_count);
    exit(0); //Successful exit
}
