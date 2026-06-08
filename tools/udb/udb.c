#include "udb.h"

#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void udb_error(const char *cmd, const char *fmt, ...)
{
    va_list ap;

    fprintf(stderr, "udb: %s: ", cmd);

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fputc('\n', stderr);
}

static FILE *open_blob(const char *cmd, const char *path)
{
    FILE *f = fopen(path, "rb");

    if (!f)
        udb_error(cmd, "cannot open '%s'", path);

    return f;
}

void print_usage(void)
{
    puts("Usage:");
    puts("  udb pack   [options] <dir> <output.udb>");
    puts("  udb unpack <input.udb> <dir>");
    puts("  udb list   <input.udb>");
    puts("  udb info   <input.udb>");
    puts("  udb read   <input.udb> <path>");
    puts("  udb write  <input.udb> <path> <file>");
    puts("");
    puts("Options:");
    puts("  -r, --root <name>      Root directory name");
    puts("  -d, --detail <text>    Description text");
}

void cmd_pack(int argc, char **argv)
{
    if (argc < 2) {
        udb_error("pack", "missing <dir> <output.udb>");
        return;
    }

    const char *src = argv[0];
    const char *out = argv[1];
    const char *root = NULL;
    const char *detail = NULL;

    struct option opts[] = {
        {"root",   required_argument, NULL, 'r'},
        {"detail", required_argument, NULL, 'd'},
        {0, 0, 0, 0}
    };

    optind = 2;

    int opt;
    while ((opt = getopt_long(argc, argv, "r:d:", opts, NULL)) != -1) {
        switch (opt) {
            case 'r':
                root = optarg;
                break;

            case 'd':
                detail = optarg;
                break;

            default:
                return;
        }
    }

    if (udb_pack(src, out, root, detail) != 0) {
        udb_error("pack", "failed to create blob");
        return;
    }

    FILE *f = fopen(out, "rb");
    if (!f) {
        printf("Created %s\n", out);
        return;
    }

    UDB_Header *hdr = udb_read_header(f);

    if (!hdr) {
        fclose(f);
        printf("Created %s\n", out);
        return;
    }

    printf("Created %s\n\n", out);

    udb_print_metadata(hdr);

    udb_free_header(hdr);
    fclose(f);
}

void cmd_unpack(int argc, char **argv)
{
    if (argc < 2) {
        udb_error("unpack", "missing <input.udb> <dir>");
        return;
    }

    if (udb_unpack(argv[0], argv[1]) == 0) {
        printf("Extracted to %s\n", argv[1]);
    } else {
        udb_error("unpack", "failed to extract blob");
    }
}

void cmd_list(int argc, char **argv)
{
    if (argc < 1) {
        udb_error("list", "missing <input.udb>");
        return;
    }

    FILE *f = open_blob("list", argv[0]);
    if (!f)
        return;

    UDB_Header *hdr = udb_read_header(f);

    if (!hdr) {
        fclose(f);
        udb_error("list", "invalid blob");
        return;
    }

    size_t width = strlen("Path");

    for (uint32_t i = 0; i < hdr->file_count; i++) {
        size_t len = strlen(hdr->entries[i].name);

        if (len > width)
            width = len;
    }

    if (width > 80)
        width = 80;

    printf("Blob: %s\n\n", argv[0]);

    printf("%-*s  %12s\n",
        (int)width,
        "Path",
        "Size");

    for (size_t i = 0; i < width + 14; i++)
        putchar('-');

    putchar('\n');

    for (uint32_t i = 0; i < hdr->file_count; i++) {
        printf("%-*s  %12llu\n",
            (int)width,
            hdr->entries[i].name,
            (unsigned long long)hdr->entries[i].size);
    }

    printf("\n%u file(s)\n", hdr->file_count);

    udb_free_header(hdr);
    fclose(f);
}

void cmd_info(int argc, char **argv)
{
    if (argc < 1) {
        udb_error("info", "missing <input.udb>");
        return;
    }

    FILE *f = open_blob("info", argv[0]);
    if (!f)
        return;

    UDB_Header *hdr = udb_read_header(f);

    if (!hdr) {
        fclose(f);
        udb_error("info", "invalid blob");
        return;
    }

    printf("Blob: %s\n\n", argv[0]);

    udb_print_metadata(hdr);

    udb_free_header(hdr);
    fclose(f);
}

void cmd_read(int argc, char **argv)
{
    if (argc < 2) {
        udb_error("read", "missing <input.udb> <path>");
        return;
    }

    FILE *f = open_blob("read", argv[0]);
    if (!f)
        return;

    UDB_Header *hdr = udb_read_header(f);

    if (!hdr) {
        fclose(f);
        udb_error("read", "invalid blob");
        return;
    }

    uint8_t *data = NULL;
    uint64_t size = 0;

    if (udb_read_file(f, hdr, argv[1], &data, &size) != 0) {
        udb_error("read", "file not found: %s", argv[1]);
    } else {
        fwrite(data, 1, size, stdout);
        free(data);
    }

    udb_free_header(hdr);
    fclose(f);
}

void cmd_write(int argc, char **argv)
{
    if (argc < 3) {
        udb_error("write", "missing <input.udb> <path> <file>");
        return;
    }

    FILE *src = fopen(argv[2], "rb");

    if (!src) {
        udb_error("write", "cannot open '%s'", argv[2]);
        return;
    }

    fseek(src, 0, SEEK_END);
    uint64_t size = ftell(src);
    fseek(src, 0, SEEK_SET);

    uint8_t *data = malloc(size);

    if (!data) {
        fclose(src);
        udb_error("write", "out of memory");
        return;
    }

    fread(data, 1, size, src);
    fclose(src);

    if (udb_write_file(argv[0], argv[1], data, size) == 0) {
        printf("Updated %s (%llu bytes)\n",
            argv[1],
            (unsigned long long)size);
    } else {
        udb_error("write", "failed to update file");
    }

    free(data);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage();
        return 1;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "pack") == 0)
        cmd_pack(argc - 2, argv + 2);
    else if (strcmp(cmd, "unpack") == 0)
        cmd_unpack(argc - 2, argv + 2);
    else if (strcmp(cmd, "list") == 0)
        cmd_list(argc - 2, argv + 2);
    else if (strcmp(cmd, "info") == 0)
        cmd_info(argc - 2, argv + 2);
    else if (strcmp(cmd, "read") == 0)
        cmd_read(argc - 2, argv + 2);
    else if (strcmp(cmd, "write") == 0)
        cmd_write(argc - 2, argv + 2);
    else {
        udb_error("main", "unknown command '%s'", cmd);
        print_usage();
        return 1;
    }

    return 0;
}