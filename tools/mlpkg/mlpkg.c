#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <curl/curl.h>
#include <zip.h>
#include <json-c/json.h>
#include <pwd.h>

#define BUFFER_SIZE 4096
#define HOME_BUFFER 256
#define PKG_PATH "/.local/mila/packages"
#define MIRROR_PATH "/.local/mila/mirrorlist.json"

typedef struct {
    char name[256];
    char link[512];
    char recent_marker[64];
    char command[256];
} Mirror;

typedef struct {
    Mirror *mirrors;
    int count;
} MirrorList;

char* get_home_dir(char *buffer) {
    char *home = getenv("HOME");
    if (!home) home = getpwuid(getuid())->pw_dir;
    strcpy(buffer, home);
    return buffer;
}

int create_dir_if_needed(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        return mkdir(path, 0755);
    }
    return 0;
}

MirrorList* load_mirrors() {
    char home[HOME_BUFFER];
    char mirror_file[HOME_BUFFER + 64];
    
    get_home_dir(home);
    snprintf(mirror_file, sizeof(mirror_file), "%s%s", home, MIRROR_PATH);
    
    json_object *root = json_object_from_file(mirror_file);
    if (!root) {
        fprintf(stderr, "error: cannot open or parse mirror list at %s\n", mirror_file);
        return NULL;
    }
    
    if (!json_object_is_type(root, json_type_array)) {
        fprintf(stderr, "error: mirror list must be array\n");
        json_object_put(root);
        return NULL;
    }
    
    MirrorList *list = malloc(sizeof(MirrorList));
    list->count = json_object_array_length(root);
    list->mirrors = malloc(sizeof(Mirror) * list->count);
    
    for (int i = 0; i < list->count; i++) {
        json_object *item = json_object_array_get_idx(root, i);
        
        const char *name = json_object_get_string(json_object_object_get(item, "name"));
        const char *link = json_object_get_string(json_object_object_get(item, "link"));
        const char *marker = json_object_get_string(json_object_object_get(item, "recent-marker"));
        const char *cmd = json_object_get_string(json_object_object_get(item, "command"));
        
        if (name) strcpy(list->mirrors[i].name, name);
        if (link) strcpy(list->mirrors[i].link, link);
        if (marker) strcpy(list->mirrors[i].recent_marker, marker);
        if (cmd) strcpy(list->mirrors[i].command, cmd);
        else list->mirrors[i].command[0] = '\0';
    }
    
    json_object_put(root);
    return list;
}

char* resolve_url(const char *link, const char *package, const char *version) {
    char *url = malloc(512);
    strcpy(url, link);
    
    char found_pack=0, found_ver=0;

    char *pos;
    while ((pos = strstr(url, "<package>")) != NULL) {
        memmove(pos + strlen(package), pos + 9, strlen(pos + 9) + 1);
        memcpy(pos, package, strlen(package));
        found_pack = 1;
    }
    
    while ((pos = strstr(url, "<version>")) != NULL) {
        memmove(pos + strlen(version), pos + 9, strlen(pos + 9) + 1);
        memcpy(pos, version, strlen(version));
        found_ver = 1;
    }
    
    return found_pack && found_ver && strlen(version) ? url : NULL;
}

int download_file(const char *url, const char *output_path) {
    CURL *curl = curl_easy_init();
    if (!curl) return -1;
    
    FILE *fp = fopen(output_path, "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        return -1;
    }

    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    CURLcode res = curl_easy_perform(curl);
    int http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    fclose(fp);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK && http_code == 200) ? 0 : http_code * 1000 + res;
}

// extract zip, stripping single root dir (e.g. github's "pkg-master/")
int extract_zip(const char *zip_path, const char *extract_path) {
    int err = 0;
    zip_t *z = zip_open(zip_path, 0, &err);
    if (!z) {
        fprintf(stderr, "error: cannot open zip file\n");
        return -1;
    }

    int num_files = zip_get_num_entries(z, 0);

    // detect single root dir
    char root_prefix[256] = {0};
    const char *first = zip_get_name(z, 0, 0);
    if (first) {
        const char *slash = strchr(first, '/');
        if (slash) {
            size_t len = slash - first + 1;
            strncpy(root_prefix, first, len);
            root_prefix[len] = '\0';
        }
    }
    printf("stripping root prefix: '%s'\n", root_prefix);

    if (create_dir_if_needed(extract_path)) {
        fprintf(stderr, "warning: Failed to create extract path\n  ->%s\n", extract_path);
    }

    for (int i = 0; i < num_files; i++) {
        const char *name = zip_get_name(z, i, 0);
        zip_file_t *file = zip_fopen_index(z, i, 0);

        // strip root prefix if every entry has it
        const char *rel = name;
        if (root_prefix[0] && strncmp(name, root_prefix, strlen(root_prefix)) == 0)
            rel = name + strlen(root_prefix);

        if (rel[0] == '\0') { zip_fclose(file); continue; }

        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", extract_path, rel);

        if (rel[strlen(rel) - 1] == '/') {
            mkdir(filepath, 0755);
        } else {
            char dirpath[512];
            strcpy(dirpath, filepath);
            char *last = strrchr(dirpath, '/');
            if (last) { *last = '\0'; mkdir(dirpath, 0755); }

            FILE *fp = fopen(filepath, "wb");
            if (fp) {
                char buf[BUFFER_SIZE];
                int n;
                while ((n = zip_fread(file, buf, BUFFER_SIZE)) > 0)
                    fwrite(buf, 1, n, fp);
                fclose(fp);
            }
        }
        zip_fclose(file);
    }

    zip_close(z);
    return 0;
}

void parse_package(const char *pkg_spec, char *package, char *version) {
    char *colon = strchr(pkg_spec, ':');
    if (colon) {
        strncpy(package, pkg_spec, colon - pkg_spec);
        package[colon - pkg_spec] = '\0';
        strcpy(version, colon + 1);
    } else {
        strcpy(package, pkg_spec);
        strcpy(version, "");
    }
}

int install_package(const char *pkg_spec) {
    char package[256], version[64];
    parse_package(pkg_spec, package, version);

    char is_recent = !strlen(version);
    
    MirrorList *mirrors = load_mirrors();
    if (!mirrors || mirrors->count == 0) {
        fprintf(stderr, "error: no mirrors available\n");
        return -1;
    }

    for (int mirror=0; mirror < mirrors->count; ++mirror) {
        printf("Trying mirror: %s...\n", mirrors->mirrors[mirror].name);
        if (is_recent) {
            strcpy(version, mirrors->mirrors[mirror].recent_marker);
        }
        
        char home[HOME_BUFFER];
        get_home_dir(home);
        
        char pkg_dir[1024];
        snprintf(pkg_dir, sizeof(pkg_dir), "%s%s/%s", home, PKG_PATH, package);
        create_dir_if_needed(pkg_dir);
        
        char url[512];
        char* meep = resolve_url(mirrors->mirrors[mirror].link, package, version);
        if (!meep) {
            fprintf(stderr, "error: url for '%s' didnt construct. [invalid mirror link]\n", mirrors->mirrors[mirror].name);
            continue;
        }
        strcpy(url, meep);
        printf("Mirror Link Constructed for %s\n  from %s\n    to %s\n", mirrors->mirrors[mirror].name, mirrors->mirrors[mirror].link, url);
        
        char zip_path[512];
        snprintf(zip_path, sizeof(zip_path), "/tmp/%s-%s.zip", package, version);
        
        printf("Downloading %s:%s...\n", package, strlen(version) ? version : mirrors->mirrors[mirror].name);
        int curl_err = CURLE_OK;
        if ((curl_err=download_file(url, zip_path))) {
            fprintf(stderr, "error: download failed");
            if (curl_err%1000)
                fprintf(stderr, " [curl %d: %s, http %i]", curl_err%1000, curl_easy_strerror(curl_err%1000), curl_err/1000);
            else
                fprintf(stderr, " [http %i]", curl_err/1000);
            fprintf(stderr, ", trying next mirror\n");
            continue;
        }
        
        printf("Extracting...\n");
        if (extract_zip(zip_path, pkg_dir) != 0) {
            fprintf(stderr, "error: extraction failed [invalid zip file], trying next mirror\n");
            continue;
        }
        
        if (strlen(mirrors->mirrors[mirror].command) > 0) {
            printf("running command: \n  %s\nConfirm? [y/n]: ", mirrors->mirrors[mirror].command);
            if (getchar() == 'y') {
                chdir(pkg_dir);
                system(mirrors->mirrors[mirror].command);
            } else {
                printf("warning: post download command skipped.\n");
            }
        }
        
        unlink(zip_path);
        printf("done: %s installed\n", package);
        return 0;
    }
    fprintf(stderr, "error: package not found in mirror list. [package doesnt exist]\n");
    return 1;
}

int delete_package(const char *package) {
    char home[HOME_BUFFER];
    get_home_dir(home);
    
    char pkg_dir[512];
    snprintf(pkg_dir, sizeof(pkg_dir), "%s%s/%s", home, PKG_PATH, package);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", pkg_dir);
    if (system(cmd) == 0) {
        printf("deleted: %s\n", pkg_dir);
        return 0;
    }
    return -1;
}

int copy_dir(const char *src, const char *dst) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "cp -r %s %s", src, dst);
    return system(cmd) == 0 ? 0 : -1;
}

int remove_dir(const char *path) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    return system(cmd) == 0 ? 0 : -1;
}

int add_to_packs(const char *package) {
    char home[HOME_BUFFER];
    get_home_dir(home);
    
    char src[512];
    snprintf(src, sizeof(src), "%s%s/%s", home, PKG_PATH, package);
    
    char packs_dir[512];
    snprintf(packs_dir, sizeof(packs_dir), "./packs");
    create_dir_if_needed(packs_dir);
    
    char dst[512];
    snprintf(dst, sizeof(dst), "./packs/%s", package);
    
    if (copy_dir(src, dst) == 0) {
        printf("added to packs: %s\n", package);
        return 0;
    }
    fprintf(stderr, "error: failed to copy package to packs\n");
    return -1;
}

int remove_from_packs(const char *package) {
    char packs_path[512];
    snprintf(packs_path, sizeof(packs_path), "./packs/%s", package);
    
    if (remove_dir(packs_path) == 0) {
        printf("removed from packs: %s\n", package);
        return 0;
    }
    fprintf(stderr, "error: package not found in packs\n");
    return -1;
}

int find_package(const char *pkg_spec) {
    char package[256], version[64];
    parse_package(pkg_spec, package, version);

    MirrorList *mirrors = load_mirrors();
    if (!mirrors || mirrors->count == 0) {
        fprintf(stderr, "error: no mirrors available\n");
        return -1;
    }
    
    printf("Looking for package %s version %s\n", package, strlen(version) ? version : "<latest>");

    for (int i = 0; i < mirrors->count; i++) {
        char *url = resolve_url(mirrors->mirrors[i].link, package, strlen(version) ? version : mirrors->mirrors[i].recent_marker);
        int err = download_file(url, "/tmp/.temppackage.zip");
        if (!err) {
            printf("Found package %s in '%s' [Mirror #%d]\n", package, mirrors->mirrors[i].name, i+1);
            printf("    %s\n", url);
        }
        free(url);
    }
    return 0;
}

int update_package(const char *package) {
    char home[HOME_BUFFER];
    get_home_dir(home);
    
    char pkg_dir[512];
    snprintf(pkg_dir, sizeof(pkg_dir), "%s%s/%s", home, PKG_PATH, package);
    
    struct stat st = {0};
    if (stat(pkg_dir, &st) == -1) {
        fprintf(stderr, "error: package %s not installed\n", package);
        return -1;
    }
    
    if (delete_package(package) != 0) return -1;
    return install_package(package);
}

int update_all_packages() {
    char home[HOME_BUFFER];
    get_home_dir(home);
    
    char pkg_path[512];
    snprintf(pkg_path, sizeof(pkg_path), "%s%s", home, PKG_PATH);
    
    DIR *dir = opendir(pkg_path);
    if (!dir) {
        fprintf(stderr, "error: cannot open packages directory\n");
        return -1;
    }
    
    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
            printf("updating %s...\n", entry->d_name);
            if (update_package(entry->d_name) == 0) count++;
        }
    }
    closedir(dir);
    
    printf("updated %d packages\n", count);
    return 0;
}

int update_packs() {
    DIR *dir = opendir("./packs");
    if (!dir) {
        fprintf(stderr, "error: packs directory not found\n");
        return -1;
    }
    
    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
            char home[HOME_BUFFER];
            get_home_dir(home);
            
            char pkg_path[1024];
            snprintf(pkg_path, sizeof(pkg_path), "%s%s/%s", home, PKG_PATH, entry->d_name);
            
            printf("updating %s...\n", pkg_path);
            if (update_package(entry->d_name) == 0) {
                char packs_pkg[512];
                snprintf(packs_pkg, sizeof(packs_pkg), "./packs/%s", entry->d_name);
                remove_dir(packs_pkg);
                add_to_packs(entry->d_name);
                count++;
            }
        }
    }
    closedir(dir);
    
    printf("updated %d packages in packs\n", count);
    return 0;
}

void print_help() {
    printf(
        "usage: mlpkg [options] args...\n\n"
        "options:\n"
        "  -i <package>\n    Installs given package\n"
        "  -I <package>\n    Add the given package to packs\n"
        "  -d <package>\n    Deletes package from system\n"
        "  -r <package>\n    Removes package from packs\n"
        "  -f <package>\n    Find first occurrence of package from mirror list.\n"
        "  -u\n    Update every package\n"
        "  -U\n    Update every package in packs\n"
        "  -h | --help\n    Print this text.\n"
    );
}

int main(int argc, char *argv[]) {
    char path[1024] = {0};
    get_home_dir(path);
    strcat(path, PKG_PATH);
    if (create_dir_if_needed(path)) {
        fprintf(stderr, "error: Couldnt create PKG_PATH: %s\n", path);
        return 1;
    }
    if (argc < 2) {
        print_help();
        return 0;
    }
    
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }
    
    if (strcmp(argv[1], "-i") == 0 && argc > 2) {
        return install_package(argv[2]);
    }
    
    if (strcmp(argv[1], "-I") == 0 && argc > 2) {
        return add_to_packs(argv[2]);
    }
    
    if (strcmp(argv[1], "-d") == 0 && argc > 2) {
        return delete_package(argv[2]);
    }
    
    if (strcmp(argv[1], "-r") == 0 && argc > 2) {
        return remove_from_packs(argv[2]);
    }
    
    if (strcmp(argv[1], "-f") == 0 && argc > 2) {
        return find_package(argv[2]);
    }
    
    if (strcmp(argv[1], "-u") == 0) {
        return update_all_packages();
    }
    
    if (strcmp(argv[1], "-U") == 0) {
        return update_packs();
    }
    
    printf("unknown operation\n");
    return 1;
}