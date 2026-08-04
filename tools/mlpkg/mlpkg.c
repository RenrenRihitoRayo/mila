#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <curl/curl.h>
#include <zip.h>
#include <json-c/json.h>
#include <pwd.h>

#define BUFFER_SIZE 4096
#define HOME_BUFFER 256
#define PKG_PATH "/.local/mila/packages"
#define MIRROR_PATH "/.local/mila/mirrorlist.json"
#define INSTALLED_LIST "/.local/mila/installed-list.txt"

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

void record_installed_package(const char *package, const char *version) {
    char home[HOME_BUFFER];
    char installed_file[HOME_BUFFER + 64];
    
    get_home_dir(home);
    snprintf(installed_file, sizeof(installed_file), "%s%s", home, INSTALLED_LIST);
    
    FILE *fp = fopen(installed_file, "a");
    if (fp) {
        fprintf(fp, "%s:%s\n", package, version ? version : "");
        fclose(fp);
    }
}

void remove_from_installed_list(const char *package) {
    char home[HOME_BUFFER];
    char installed_file[HOME_BUFFER + 64];
    char temp_file[HOME_BUFFER + 70];
    
    get_home_dir(home);
    snprintf(installed_file, sizeof(installed_file), "%s%s", home, INSTALLED_LIST);
    snprintf(temp_file, sizeof(temp_file), "%s.tmp", installed_file);
    
    FILE *fp_read = fopen(installed_file, "r");
    FILE *fp_write = fopen(temp_file, "w");
    
    if (!fp_read || !fp_write) {
        if (fp_read) fclose(fp_read);
        if (fp_write) fclose(fp_write);
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), fp_read)) {
        char pkg_name[256];
        sscanf(line, "%255[^:]", pkg_name);
        if (strcmp(pkg_name, package) != 0) {
            fputs(line, fp_write);
        }
    }
    
    fclose(fp_read);
    fclose(fp_write);
    rename(temp_file, installed_file);
}

int load_dependencies(const char *package, char **deps, int *dep_count) {
    char home[HOME_BUFFER];
    char pkg_dir[1024];
    char dep_file[1024];
    
    get_home_dir(home);
    snprintf(pkg_dir, sizeof(pkg_dir), "%s%s/%s", home, PKG_PATH, package);
    snprintf(dep_file, sizeof(dep_file), "%s/dependencies.txt", pkg_dir);
    
    FILE *fp = fopen(dep_file, "r");
    if (!fp) return 0;
    
    char line[512];
    *dep_count = 0;
    while (fgets(line, sizeof(line), fp) && *dep_count < 32) {
        line[strcspn(line, "\n")] = 0;
        if (line[0] != '\0' && line[0] != '#') {
            deps[*dep_count] = malloc(strlen(line) + 1);
            strcpy(deps[(*dep_count)++], line);
        }
    }
    fclose(fp);
    return *dep_count;
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

int get_temp_dir(char *temp_path) {
    if (access("/tmp", W_OK) == 0) {
        strcpy(temp_path, "/tmp");
        return 0;
    }
    
    char home[HOME_BUFFER];
    get_home_dir(home);
    snprintf(temp_path, 256, "%s/.cache", home);
    
    if (mkdir(temp_path, 0755) == -1 && errno != EEXIST) {
        fprintf(stderr, "error: cannot create temp directory\n");
        return -1;
    }
    return 0;
}

// Callback for curl to write data to file
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    return fwrite(contents, size, nmemb, (FILE *)userp);
}

char* resolve_url(const char *link, const char *package, const char *version) {
    char *url = malloc(512);
    if (!url) {
        fprintf(stderr, "error: malloc failed for url\n");
        return NULL;
    }
    strcpy(url, link);
    
    fflush(stderr);
    
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
    
    fflush(stderr);
    
    return found_pack && found_ver && strlen(version) ? url : NULL;
}

int download_file(const char *url, const char *output_path) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "error: failed to initialize curl\n");
        return -1;
    }
    
    FILE *fp = fopen(output_path, "wb");
    if (!fp) {
        fprintf(stderr, "error: failed to open output file %s\n", output_path);
        curl_easy_cleanup(curl);
        return -1;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    fclose(fp);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK && http_code == 200) ? 0 : http_code * 1000 + res;
}

int extract_zip(const char *zip_path, const char *extract_path, int do_log) {
    int err = 0;
    zip_t *z = zip_open(zip_path, 0, &err);
    if (!z) {
        fprintf(stderr, "error: cannot open zip file\n");
        return -1;
    }

    int num_files = zip_get_num_entries(z, 0);

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
    if (do_log) printf("stripping root prefix: '%s'\n", root_prefix);

    if (create_dir_if_needed(extract_path)) {
        fprintf(stderr, "warning: Failed to create extract path\n  ->%s\n", extract_path);
    }

    for (int i = 0; i < num_files; i++) {
        const char *name = zip_get_name(z, i, 0);
        zip_file_t *file = zip_fopen_index(z, i, 0);

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

int install_package(const char *pkg_spec, int do_log) {
    char package[256], version[64];
    parse_package(pkg_spec, package, version);

    char is_latest = !strlen(version);
    
    MirrorList *mirrors = load_mirrors();
    if (!mirrors || mirrors->count == 0) {
        fprintf(stderr, "error: no mirrors available\n");
        return -1;
    }

    for (int mirror=0; mirror < mirrors->count; ++mirror) {
        if (do_log) printf("Trying mirror: %s...\n", mirrors->mirrors[mirror].name);
        
        char resolved_version[64];
        strcpy(resolved_version, version);
        if (is_latest) {
            strcpy(resolved_version, mirrors->mirrors[mirror].recent_marker);
        }
        
        char home[HOME_BUFFER];
        get_home_dir(home);
        
        char pkg_dir[1024];
        snprintf(pkg_dir, sizeof(pkg_dir), "%s%s/%s", home, PKG_PATH, package);
        create_dir_if_needed(pkg_dir);
        
        char url[512];
        char* meep = resolve_url(mirrors->mirrors[mirror].link, package, resolved_version);
        if (!meep) {
            fprintf(stderr, "error: url for '%s' didnt construct. [invalid mirror link]\n", mirrors->mirrors[mirror].name);
            continue;
        }
        strcpy(url, meep);
        if (do_log) printf("Mirror Link Constructed for %s\n  from %s\n    to %s\n", mirrors->mirrors[mirror].name, mirrors->mirrors[mirror].link, url);
        
        char zip_path[512];
        char temp_dir[256];
        if (get_temp_dir(temp_dir) != 0) {
            fprintf(stderr, "error: cannot determine temp directory\n");
            continue;
        }
        snprintf(zip_path, sizeof(zip_path), "%s/%s-%s.zip", temp_dir, package, resolved_version);
        
        if (do_log) fprintf(stderr, "Downloading %s:%s...\n", package, strlen(resolved_version) ? resolved_version : mirrors->mirrors[mirror].name);
        int curl_err = CURLE_OK;
        if ((curl_err=download_file(url, zip_path))) {
            if (do_log)
            {
                fprintf(stderr, "error: download failed");
                if (curl_err%1000)
                    fprintf(stderr, " [curl %d: %s, http %i]", curl_err%1000, curl_easy_strerror(curl_err%1000), curl_err/1000);
                else
                    fprintf(stderr, " [http %i]", curl_err/1000);
                fprintf(stderr, ", trying next mirror\n");
            }
            continue;
        }
        
        if (do_log) fprintf(stderr, "Extracting...\n");
        if (extract_zip(zip_path, pkg_dir, do_log) != 0) {
            fprintf(stderr, "error: extraction failed [invalid zip file], trying next mirror\n");
            continue;
        }
        
        if (strlen(mirrors->mirrors[mirror].command) > 0) {
            fprintf(stderr, "running command: \n  %s\nConfirm? [y/n]: ", mirrors->mirrors[mirror].command);
            if (getchar() == 'y') {
                chdir(pkg_dir);
                system(mirrors->mirrors[mirror].command);
            } else {
                fprintf(stderr, "warning: post download command skipped.\n");
            }
        }
        
        // Install dependencies if present (before recording)
        char *deps[32];
        int dep_count = 0;
        int deps_ok = 1;
        if (load_dependencies(package, deps, &dep_count) > 0) {
            fprintf(stderr, "Found %d dependencies (for %s)\n", dep_count, pkg_spec);
            for (int i = 0; i < dep_count; i++) {
                fprintf(stderr, "  '%s'\n", deps[i]);
            }
            fprintf(stderr, "Installing dependencies:\n");
            for (int i = 0; i < dep_count; i++) {
                fprintf(stderr, "  installing dependency: '%s'\n", deps[i]);
                if (install_package(deps[i], 0) != 0) {
                    fprintf(stderr, "  error: dependency %s failed to install\n", deps[i]);
                    deps_ok = 0;
                    break;
                }
                free(deps[i]);
            }
        }
        
        if (!deps_ok) {
            fprintf(stderr, "error: %s installation aborted due to failed dependencies\n", package);
            unlink(zip_path);
            return 1;
        }
        
        // Record install only if deps succeeded
        record_installed_package(package, is_latest ? "" : resolved_version);
        
        unlink(zip_path);
        if (do_log) fprintf(stderr, "done: %s installed\n", package);
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
        remove_from_installed_list(package);
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

    char temp_dir[256];
    if (get_temp_dir(temp_dir) != 0) {
        fprintf(stderr, "error: cannot determine temp directory\n");
        return -1;
    }

    for (int i = 0; i < mirrors->count; i++) {
        char *url = resolve_url(mirrors->mirrors[i].link, package, strlen(version) ? version : mirrors->mirrors[i].recent_marker);
        char temp_file[512];
        snprintf(temp_file, sizeof(temp_file), "%s/.temppackage.zip", temp_dir);
        int err = download_file(url, temp_file);
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
    return install_package(package, 0);
}

int update_all_packages() {
    char home[HOME_BUFFER];
    char installed_file[HOME_BUFFER + 64];
    
    get_home_dir(home);
    snprintf(installed_file, sizeof(installed_file), "%s%s", home, INSTALLED_LIST);
    
    FILE *fp = fopen(installed_file, "r");
    if (!fp) {
        fprintf(stderr, "warning: no installed packages list found\n");
        return 0;
    }
    
    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        if (line[0] == '\0' || line[0] == '#') continue;
        
        char package[256], version[64];
        parse_package(line, package, version);
        
        // Only update packages with empty version (marked as "latest")
        if (strlen(version) == 0) {
            printf("updating %s...\n", package);
            if (update_package(package) == 0) count++;
        }
    }
    fclose(fp);
    
    printf("updated %d packages\n", count);
    return 0;
}

int list_all_packages() {
    char home[HOME_BUFFER];
    char installed_file[HOME_BUFFER + 64];
    
    get_home_dir(home);
    snprintf(installed_file, sizeof(installed_file), "%s%s", home, INSTALLED_LIST);
    
    FILE *fp = fopen(installed_file, "r");
    if (!fp) {
        fprintf(stderr, "warning: no installed packages list found\n");
        return 0;
    }
    
    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        if (line[0] == '\0' || line[0] == '#') continue;
        printf("%s\n", line);
        count ++;
    }
    fclose(fp);
    
    printf("%d packages\n", count);
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

int list_update_packs() {
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
        "  -u\n    Update packages marked for update (empty version in installed-list.txt)\n"
        "  -U\n    Update every package in packs\n"
        "  -l\n    List global packages\n"
        "  -L\n    List local packages\n"
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
        return install_package(argv[2], 1);
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
    
    if (strcmp(argv[1], "-l") == 0) {
        return list_all_packages();
    }
    
    printf("unknown operation\n");
    return 1;
}