#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <fuse.h>

#include <stdio.h>
#include <string.h>

#include "myfs_api.h"

#define FUSE_USE_VERSION 26
#define LOG_FILE "/Users/Alexander/vsu_os/file_system/myfs/log.txt"

static void log(const char *msg)
{
    FILE* log = fopen(LOG_FILE, "ab+");
    fprintf(log, "%s\n", msg);
    fclose(log);
}

static int myfs_getattr(const char *path, struct stat *stbuf)
{    
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) // Root
    {
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 3;            
    }
    else
    {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        
        FILE *data_file = fopen(DATA_FILE, "r+");
        int id = fs_search_header(data_file, path + 1);
        
        if (id != -1)
        {
            header_t header = fs_read_header(data_file, id);
            stbuf->st_size = header.size;                        
        }
        
        fclose(data_file);    
        
        if (id == -1)
        {
            return -ENOENT;
        }                
    }
    
    return 0;
}

static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
              off_t offset, struct fuse_file_info *fi)
{
    log("readdir");
    log(path);
    
    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0);           /* Current directory (.)  */
    filler(buf, "..", NULL, 0);          /* Parent directory (..)  */

    // Read all headers
    FILE* data_file = fopen(DATA_FILE, "r+");
    
    fseek(data_file, 0, SEEK_SET);
    char pre_h[HEADERS_COUNT];
    fread(pre_h, 1, HEADERS_COUNT, data_file);
    
    int i;
    for (i = 0; i < HEADERS_COUNT; i++)
    {
        if (pre_h[i])
        {
            header_t header = fs_read_header(data_file, i);
            filler(buf, header.name, NULL, 0);
        }
    }
    
    fclose(data_file);
    // Read all headers

    return 0;
}

static int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    log("create");
    log(path);
    
    FILE* data_file = fopen(DATA_FILE, "r+");
    
    // File already exists
    int id = fs_search_header(data_file, path + 1);
    if (id != -1)
    {
        return -ENOENT;
    }
    
    header_t header = {0, "", 0, 0};
    strcpy(header.name, path + 1);
    fs_create_header(data_file, header);
    
    fclose(data_file);
        
	return 0;
}

static int myfs_unlink(const char *path)
{
    log("unlink");
    log(path);
    
    FILE* data_file = fopen(DATA_FILE, "r+");
    
    int id = fs_search_header(data_file, path + 1);
    if (id == -1)   // File not exists
    {
        return -1;
    }
    
    fs_delete_header(data_file, id);
    
    fclose(data_file);
    
    return 0;
}

static int myfs_open(const char *path, struct fuse_file_info *fi)
{
    log("open");
    log(path);
    return 0;
}

static int myfs_opendir(const char *path, struct fuse_file_info *fi)
{
    log("opendir");
    log(path);
	return 0;
}

static int myfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    FILE* log = fopen(LOG_FILE, "ab+");
    fprintf(log, "rd, off: %lld size: %zu path: %s\n", offset, size, path);
    fclose(log);
    
    FILE* data_file = fopen(DATA_FILE, "r+");
    
    int h_id = fs_search_header(data_file, path + 1);
    if (h_id == -1)    // No file with same name
    {
        return -ENOENT;
    }
    
    fseek(data_file, HEADERS_COUNT + HEADERS_COUNT * sizeof(header_t)
        + DATA_BLOCKS_COUNT + DATA_BLOCK_SIZE * h_id + offset
        , SEEK_SET);
    
    fread(buf, 1, size, data_file);    
    
    fclose(data_file);
	return size;
}

static int myfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    FILE* log = fopen(LOG_FILE, "ab+");
    fprintf(log, "wt, off: %lld size: %zu path: %s\n", offset, size, path);
    fclose(log);
    
    FILE* data_file = fopen(DATA_FILE, "r+");
    
    int h_id = fs_search_header(data_file, path + 1);
    if (h_id == -1)    // No file with same name
    {
        return -1;
    }
    
    fseek(data_file, HEADERS_COUNT + HEADERS_COUNT * sizeof(header_t)
        + DATA_BLOCKS_COUNT + DATA_BLOCK_SIZE * h_id + offset
        , SEEK_SET);
    
    fwrite(buf, 1, size, data_file);    
    
    header_t header = fs_read_header(data_file, h_id);
    header.size = offset + size > header.size ? offset + size : header.size;
    fs_update_header(data_file, h_id, header);

    fclose(data_file);
	return 0;
}

static int myfs_rename(const char *path, const char *new_path)
{
    log("rename");
    log(path);
    log(new_path);
    
    FILE* data_file = fopen(DATA_FILE, "r+");
    
    int h_id = fs_search_header(data_file, path + 1);
    if (h_id == -1)    // No file with same name
    {
        return -1;
    } 
    
    header_t header = fs_read_header(data_file, h_id);
    strcpy(header.name, new_path);
    fs_update_header(data_file, h_id, header);

    fclose(data_file);

	return 0;
}

static int myfs_statfs(const char *path, struct statvfs *stbuf)
{
    // stbuf->f_bsize  = 1024 * 1024;  /* 1MB */
    // stbuf->f_frsize = 1024 * 1024;  /* MAXPHYS */
    
    stbuf->f_blocks = 100; /* aim for a lot; this is 32-bit though  */
    stbuf->f_bfree  = 30;
    stbuf->f_bavail = 30;
    // inodes
    stbuf->f_files  = 100;
    stbuf->f_ffree = 50;
    stbuf->f_favail = 50;
    return 0;
}

static struct fuse_operations myfs_filesystem_operations = {
    .getattr = myfs_getattr,
    
    .opendir = myfs_opendir,
    .readdir = myfs_readdir,
    
    .create  = myfs_create,
    .unlink  = myfs_unlink,
    
    .open    = myfs_open,
    
    .read    = myfs_read,
    .write   = myfs_write,
    
    .rename  = myfs_rename,
    
    .statfs  = myfs_statfs,
};

int main(int argc, char **argv)
{
    myfs_create("/hello.txt", 0, NULL);
    myfs_write("/hello.txt", "hello, world!", sizeof("hello, world!"), 0, NULL);
    myfs_write("/hello.txt", "hello, world!", sizeof("hello, world!"), 1, NULL);
    
    return fuse_main(argc, argv, &myfs_filesystem_operations, NULL);
}
