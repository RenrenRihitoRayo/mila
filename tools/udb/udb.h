#ifndef UDB_H
#define UDB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#define UDB_MAGIC "udb-"
#define UDB_MAGIC_LEN 4

typedef struct {
    uint16_t name_length;
    char *name;
    uint64_t offset;
    uint64_t size;
} UDB_Entry;

typedef struct {
    char magic[UDB_MAGIC_LEN];
    uint16_t root_name_length;
    char *root_name;
    uint16_t detail_length;
    char *detail;
    uint64_t timestamp;
    uint32_t file_count;
    UDB_Entry *entries;
} UDB_Header;

// Endianness helpers
static inline uint16_t read_u16(FILE *f) {
    uint8_t b[2];
    fread(b, 1, 2, f);
    return b[0] | (b[1] << 8);
}

static inline uint32_t read_u32(FILE *f) {
    uint8_t b[4];
    fread(b, 1, 4, f);
    return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
}

static inline uint64_t read_u64(FILE *f) {
    uint8_t b[8];
    fread(b, 1, 8, f);
    return b[0] | ((uint64_t)b[1] << 8) | ((uint64_t)b[2] << 16) | ((uint64_t)b[3] << 24) |
           ((uint64_t)b[4] << 32) | ((uint64_t)b[5] << 40) | ((uint64_t)b[6] << 48) | ((uint64_t)b[7] << 56);
}

static inline void write_u16(FILE *f, uint16_t v) {
    uint8_t b[2] = {(uint8_t)(v & 0xff), (uint8_t)((v >> 8) & 0xff)};
    fwrite(b, 1, 2, f);
}

static inline void write_u32(FILE *f, uint32_t v) {
    uint8_t b[4] = {(uint8_t)(v & 0xff), (uint8_t)((v >> 8) & 0xff), (uint8_t)((v >> 16) & 0xff), (uint8_t)((v >> 24) & 0xff)};
    fwrite(b, 1, 4, f);
}

static inline void write_u64(FILE *f, uint64_t v) {
    uint8_t b[8];
    for (int i = 0; i < 8; i++) b[i] = (v >> (i * 8)) & 0xff;
    fwrite(b, 1, 8, f);
}

// Read header from UDB blob
UDB_Header* udb_read_header(FILE *f) {
    UDB_Header *hdr = (UDB_Header*)malloc(sizeof(UDB_Header));
    fread(hdr->magic, 1, 4, f);
    
    hdr->root_name_length = read_u16(f);
    hdr->root_name = (char*)malloc(hdr->root_name_length + 1);
    if (hdr->root_name_length > 0) fread(hdr->root_name, 1, hdr->root_name_length, f);
    hdr->root_name[hdr->root_name_length] = '\0';
    
    hdr->detail_length = read_u16(f);
    hdr->detail = (char*)malloc(hdr->detail_length + 1);
    if (hdr->detail_length > 0) fread(hdr->detail, 1, hdr->detail_length, f);
    hdr->detail[hdr->detail_length] = '\0';
    
    hdr->timestamp = read_u64(f);
    hdr->file_count = read_u32(f);
    
    hdr->entries = (UDB_Entry*)malloc(sizeof(UDB_Entry) * hdr->file_count);
    for (uint32_t i = 0; i < hdr->file_count; i++) {
        hdr->entries[i].name_length = read_u16(f);
        hdr->entries[i].name = (char*)malloc(hdr->entries[i].name_length + 1);
        fread(hdr->entries[i].name, 1, hdr->entries[i].name_length, f);
        hdr->entries[i].name[hdr->entries[i].name_length] = '\0';
        hdr->entries[i].offset = read_u64(f);
        hdr->entries[i].size = read_u64(f);
    }
    
    return hdr;
}

// Free header memory
void udb_free_header(UDB_Header *hdr) {
    if (!hdr) return;
    free(hdr->root_name);
    free(hdr->detail);
    for (uint32_t i = 0; i < hdr->file_count; i++) free(hdr->entries[i].name);
    free(hdr->entries);
    free(hdr);
}

// Display header metadata
void udb_print_metadata(UDB_Header *hdr) {
    printf("UDB Metadata:\n");
    printf("  Root: %s\n", hdr->root_name_length > 0 ? hdr->root_name : "(current dir)");
    printf("  Detail: %s\n", hdr->detail_length > 0 ? hdr->detail : "(none)");
    printf("  Timestamp: %lu\n", hdr->timestamp);
    printf("  Files: %u\n", hdr->file_count);
    printf("\nEntries:\n");
    uint64_t size =0 ;
    for (uint32_t i = 0; i < hdr->file_count; i++) {
        printf("  %s (offset: %lu, size: %lu)\n", hdr->entries[i].name, 
               hdr->entries[i].offset, hdr->entries[i].size);
        size += hdr->entries[i].size;
    }
    printf("\nTotal size: %lu\n", size);
}

// Read single file from blob
int udb_read_file(FILE *f, UDB_Header *hdr, const char *path, uint8_t **data, uint64_t *size) {
    for (uint32_t i = 0; i < hdr->file_count; i++) {
        if (strcmp(hdr->entries[i].name, path) == 0) {
            if (hdr->entries[i].name[strlen(hdr->entries[i].name) - 1] == '/') return -1;
            
            long blob_start = ftell(f);
            fseek(f, blob_start + hdr->entries[i].offset, SEEK_SET);
            
            *size = hdr->entries[i].size;
            *data = (uint8_t*)malloc(*size);
            fread(*data, 1, *size, f);
            return 0;
        }
    }
    return -1;
}

// Recursive directory scanning for packing
typedef struct {
    char **paths;
    uint32_t count;
    uint32_t capacity;
} PathList;

static void collect_paths(const char *base, const char *rel, PathList *list) {
    char full_path[4096];
    if (strlen(rel) > 0) {
        snprintf(full_path, sizeof(full_path), "%s/%s", base, rel);
    } else {
        snprintf(full_path, sizeof(full_path), "%s", base);
    }
    
    DIR *d = opendir(full_path);
    if (!d) {
        if (list->count >= list->capacity) {
            list->capacity *= 2;
            list->paths = (char**)realloc(list->paths, sizeof(char*) * list->capacity);
        }
        list->paths[list->count] = (char*)malloc(strlen(rel) + 1);
        strcpy(list->paths[list->count], rel);
        list->count++;
        return;
    }
    
    if (strlen(rel) > 0) {
        if (list->count >= list->capacity) {
            list->capacity *= 2;
            list->paths = (char**)realloc(list->paths, sizeof(char*) * list->capacity);
        }
        list->paths[list->count] = (char*)malloc(strlen(rel) + 2);
        strcpy(list->paths[list->count], rel);
        strcat(list->paths[list->count], "/");
        list->count++;
    }
    
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        
        char new_rel[4096];
        if (strlen(rel) > 0) {
            snprintf(new_rel, sizeof(new_rel), "%s/%s", rel, ent->d_name);
        } else {
            snprintf(new_rel, sizeof(new_rel), "%s", ent->d_name);
        }
        collect_paths(base, new_rel, list);
    }
    closedir(d);
}

// Pack directory to UDB blob
int udb_pack(const char *src_dir, const char *output_file, const char *root_name, const char *detail) {
    FILE *out = fopen(output_file, "wb");
    if (!out) return -1;
    
    PathList list = {(char**)malloc(sizeof(char*) * 256), 0, 256};
    collect_paths(src_dir, "", &list);
    
    // Write header
    fwrite(UDB_MAGIC, 1, 4, out);
    uint16_t root_len = root_name ? strlen(root_name) : 0;
    write_u16(out, root_len);
    if (root_len) fwrite(root_name, 1, root_len, out);
    
    uint16_t detail_len = detail ? strlen(detail) : 0;
    write_u16(out, detail_len);
    if (detail_len) fwrite(detail, 1, detail_len, out);
    
    uint64_t ts = (uint64_t)time(NULL) * 1000;
    write_u64(out, ts);
    write_u32(out, list.count);
    
    // Write entries and collect offsets
    uint64_t blob_offset = 0;
    for (uint32_t i = 0; i < list.count; i++) {
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", src_dir, list.paths[i]);
        
        uint64_t size = 0;
        int is_dir = (list.paths[i][strlen(list.paths[i]) - 1] == '/');
        
        if (!is_dir) {
            struct stat st;
            stat(full_path, &st);
            size = st.st_size;
        }
        
        write_u16(out, strlen(list.paths[i]));
        fwrite(list.paths[i], 1, strlen(list.paths[i]), out);
        write_u64(out, is_dir ? 0 : blob_offset);
        write_u64(out, size);
        
        if (!is_dir) blob_offset += size;
    }
    
    // Write blob data
    for (uint32_t i = 0; i < list.count; i++) {
        if (list.paths[i][strlen(list.paths[i]) - 1] == '/') continue;
        
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", src_dir, list.paths[i]);
        FILE *in = fopen(full_path, "rb");
        if (!in) continue;
        
        uint8_t buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
            fwrite(buf, 1, n, out);
        }
        fclose(in);
    }
    
    fclose(out);
    for (uint32_t i = 0; i < list.count; i++) free(list.paths[i]);
    free(list.paths);
    return 0;
}

// Unpack UDB blob to directory
int udb_unpack(const char *blob_file, const char *dest_dir) {
    FILE *f = fopen(blob_file, "rb");
    if (!f) return -1;
    
    UDB_Header *hdr = udb_read_header(f);
    long blob_start = ftell(f);
    
    char target[4096];
    if (hdr->root_name_length > 0) {
        mkdir(hdr->root_name, 0755);
        snprintf(target, sizeof(target), "%s/%s", dest_dir, hdr->root_name);
        fprintf(stderr, "udb: made root '%s'\n", target);
        mkdir(target, 0755);
    } else {
        strcpy(target, dest_dir);
    }
    
    for (uint32_t i = 0; i < hdr->file_count; i++) {
        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", target, hdr->entries[i].name);
        
        if (hdr->entries[i].name[strlen(hdr->entries[i].name) - 1] == '/') {
            mkdir(path, 0755);
        } else {
            char dir[4096];
            strcpy(dir, path);
            char *last = strrchr(dir, '/');
            if (last) {
                *last = '\0';
                mkdir(dir, 0755);
            }
            
            FILE *out = fopen(path, "wb");
            if (!out) {
                fprintf(stderr, "udb: file '%s' could not be opened!\n", path);
                udb_free_header(hdr);
                fclose(f);
                return 1;
            }
            
            fseek(f, blob_start + hdr->entries[i].offset, SEEK_SET);
            uint8_t buf[4096];
            uint64_t remaining = hdr->entries[i].size;
            while (remaining > 0) {
                size_t to_read = (remaining > sizeof(buf)) ? sizeof(buf) : remaining;
                fread(buf, 1, to_read, f);
                fwrite(buf, 1, to_read, out);
                remaining -= to_read;
            }
            fclose(out);
        }
    }
    
    udb_free_header(hdr);
    fclose(f);
    return 0;
}

// Write/replace file in blob (rebuilds entire blob)
int udb_write_file(const char *blob_file, const char *path, const uint8_t *data, uint64_t size) {
    FILE *f = fopen(blob_file, "rb");
    if (!f) return -1;
    
    UDB_Header *hdr = udb_read_header(f);
    long blob_start = ftell(f);
    
    int found = -1;
    for (uint32_t i = 0; i < hdr->file_count; i++) {
        if (strcmp(hdr->entries[i].name, path) == 0) {
            found = i;
            break;
        }
    }
    
    if (found == -1) {
        hdr->file_count++;
        hdr->entries = (UDB_Entry*)realloc(hdr->entries, sizeof(UDB_Entry) * hdr->file_count);
        found = hdr->file_count - 1;
        hdr->entries[found].name = (char*)malloc(strlen(path) + 1);
        strcpy(hdr->entries[found].name, path);
    }
    
    hdr->entries[found].size = size;
    hdr->timestamp = (uint64_t)time(NULL) * 1000;
    
    FILE *tmp = fopen(".udb_tmp", "wb");
    
    fwrite(UDB_MAGIC, 1, 4, tmp);
    write_u16(tmp, hdr->root_name_length);
    if (hdr->root_name_length) fwrite(hdr->root_name, 1, hdr->root_name_length, tmp);
    write_u16(tmp, hdr->detail_length);
    if (hdr->detail_length) fwrite(hdr->detail, 1, hdr->detail_length, tmp);
    write_u64(tmp, hdr->timestamp);
    write_u32(tmp, hdr->file_count);
    
    uint64_t blob_offset = 0;
    for (uint32_t i = 0; i < hdr->file_count; i++) {
        write_u16(tmp, hdr->entries[i].name_length);
        fwrite(hdr->entries[i].name, 1, hdr->entries[i].name_length, tmp);
        write_u64(tmp, hdr->entries[i].offset == 0 && hdr->entries[i].size > 0 ? blob_offset : 0);
        write_u64(tmp, hdr->entries[i].size);
        if (hdr->entries[i].size > 0 && hdr->entries[i].name[strlen(hdr->entries[i].name) - 1] != '/') {
            blob_offset += hdr->entries[i].size;
        }
    }
    
    fseek(f, blob_start, SEEK_SET);
    uint8_t buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        fwrite(buf, 1, n, tmp);
    }
    
    fseek(tmp, blob_start + hdr->entries[found].offset, SEEK_SET);
    fwrite(data, 1, size, tmp);
    
    fclose(f);
    fclose(tmp);
    remove(blob_file);
    rename(".udb_tmp", blob_file);
    
    udb_free_header(hdr);
    return 0;
}

#endif