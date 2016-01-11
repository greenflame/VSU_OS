#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "myfs_api.h"

void print_header(header_t header)
{
    printf("isf: %d, name: %s, size: %lld, db_id: %d\n",
        header.is_folder, header.name, header.size, header.db_id);
}

int main(int argc, char *argv[])
{   
    if (argc == 2 && strcmp(argv[1], "list") == 0)
    {
        FILE* data_file = fopen(DATA_FILE, "r+");
        
        fseek(data_file, 0, SEEK_SET);
        char pre_h[HEADERS_COUNT];
        fread(pre_h, 1, HEADERS_COUNT, data_file);
        
        for (int i = 0; i < HEADERS_COUNT; i++)
        {
            if (pre_h[i])
            {
                header_t header = fs_read_header(data_file, i);
                printf("id: %d, ", i);
                print_header(header);
            }
        }
        
        fclose(data_file);
        printf("that is all\n");
    }
    
    if (argc == 2 && strcmp(argv[1], "format") == 0)
    {
        FILE* data_file = fopen(DATA_FILE, "r+");
        fs_format(data_file);
        fclose(data_file);
        printf("formated\n");
    }
    
    if (argc == 3 && strcmp(argv[1], "add") == 0)
    {
        FILE* data_file = fopen(DATA_FILE, "r+");
        
        header_t header = {0, "", 0, 0};
        strcpy(header.name, argv[2]);
        fs_create_header(data_file, header);
        
        fclose(data_file);
        printf("inserted\n");
    }
    
    if (argc == 3 && strcmp(argv[1], "search") == 0)
    {
        FILE* data_file = fopen(DATA_FILE, "r+");
        
        int id = fs_search_header(data_file, argv[2]);
        printf("hid: %d\n", id);
        
        fclose(data_file);
        printf("completed\n");
    }
    
    return 0;
}