#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "myfs_api.h"

/*
 * Format fs. Clear all pre header and pre data blocks area.
 */
void fs_format(FILE* data_file)
{   
    // Initializing pre headers region
    fseek(data_file, 0, SEEK_SET);
    char h_pre[HEADERS_COUNT] = { 0 };
    fwrite(h_pre, 1, HEADERS_COUNT, data_file);
    
    // Initializing pre data blocks region
    fseek(data_file, HEADERS_COUNT + sizeof(header_t) * HEADERS_COUNT, SEEK_SET);
    char db_pre[DATA_BLOCKS_COUNT] = { 0 };
    fwrite(db_pre, 1, DATA_BLOCKS_COUNT, data_file);
}

/*
 * Find free id for header, write it and return id.
 */
int fs_create_header(FILE* data_file, header_t header)
{
    // Read pre headers block to memory
    fseek(data_file, 0, SEEK_SET);
    char h_pre[HEADERS_COUNT];
    fread(h_pre, 1, HEADERS_COUNT, data_file);

    // Search for free index
    int index = -1;
    for (int i = 0; i < HEADERS_COUNT; i++)
    {
        if (h_pre[i] == 0)
        {
            index = i;
            break;
        }
    }
    
    // No free index
    if (index == -1)
    {
        return -1;
    }
    
    fseek(data_file, index, SEEK_SET);
    fputc(1, data_file);
    
    fseek(data_file, HEADERS_COUNT + sizeof(header_t) * index, SEEK_SET);
    fwrite(&header, sizeof(header_t), 1, data_file);
    
    return index;
}

/*
 * Read header by id.
 */
header_t fs_read_header(FILE* data_file, int header_id)
{
    header_t result;
    fseek(data_file, HEADERS_COUNT + sizeof(header_t) * header_id, SEEK_SET);
    fread(&result, sizeof(header_t), 1, data_file);
    return result;
}

/*
 * Update header by id.
 */
void fs_update_header(FILE* data_file, int index, header_t header)
{
    fseek(data_file, HEADERS_COUNT + sizeof(header_t) * index, SEEK_SET);
    fwrite(&header, sizeof(header_t), 1, data_file);
}

/*
 * Remove header by id.
 */
void fs_delete_header(FILE* data_file, int header_id)
{
    fseek(data_file, header_id, SEEK_SET);
    fputc(0, data_file);
}

/*
 * Search header by name.
 */
int fs_search_header(FILE* data_file, const char* name)
{    
    fseek(data_file, 0, SEEK_SET);
    char pre_h[HEADERS_COUNT];
    fread(pre_h, 1, HEADERS_COUNT, data_file);
    
    int result = -1;
    for (int i = 0; i < HEADERS_COUNT; i++)
    {
        if (pre_h[i])   // Header exists
        {
            header_t header = fs_read_header(data_file, i);
            if (strcmp(header.name, name) == 0) // Similar name
            {
                result = i;
            }
        }
    }
    
    return result;
}
