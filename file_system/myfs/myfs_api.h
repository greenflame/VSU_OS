#ifndef MYFS_API_H
#define MYFS_API_H

#define DATA_FILE "/home/alexander/vsu_ofile_system/myfs/mydata.bin"
#define HEADERS_COUNT 128
#define DATA_BLOCKS_COUNT 128
#define DATA_BLOCK_SIZE (10*1024*1024)

typedef struct header_s
{
    int is_folder;      // Folder flag  
    char name[256];      // Name
    long long size;     // Data size
    int db_id;          // Data block index
}
header_t;


void fs_format(FILE* data_file);

int fs_create_header(FILE* data_file, header_t header);
header_t fs_read_header(FILE* data_file, int header_id);
void fs_update_header(FILE* data_file, int index, header_t header);
void fs_delete_header(FILE* data_file, int header_id);
int fs_search_header(FILE* data_file, const char* name);

#endif
