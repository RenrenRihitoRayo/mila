#pragma once
#include "http.h"
#include "../../mila.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET(s) closesocket(s)
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #define CLOSE_SOCKET(s) close(s)
#endif

void http_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

void http_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

// Parse URL to extract host and path
static void parse_url(const char *url, char *host, char *path) {
    const char *p = url;
    if (strncmp(p, "http://", 7) == 0) p += 7;
    else if (strncmp(p, "https://", 8) == 0) p += 8;
    
    const char *slash = strchr(p, '/');
    if (!slash) {
        strcpy(host, p);
        strcpy(path, "/");
    } else {
        strncpy(host, p, slash - p);
        host[slash - p] = '\0';
        strcpy(path, slash);
    }
}

// Connect to host and return socket
static int connect_socket(const char *host) {
    struct hostent *he = gethostbyname(host);
    if (!he) return -1;
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr = *(struct in_addr *)he->h_addr;
    
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        CLOSE_SOCKET(sock);
        return -1;
    }
    return sock;
}

// Send HTTP request and read response
static HttpResponse send_request(int sock, const char *request) {
    HttpResponse resp = {0, NULL, 0, NULL};
    
    send(sock, request, strlen(request), 0);
    
    char *buffer = malloc(65536);
    int total = 0, n;
    while ((n = recv(sock, buffer + total, 65536 - total, 0)) > 0) {
        total += n;
    }
    CLOSE_SOCKET(sock);
    buffer[total] = '\0';
    
    // Parse status code
    sscanf(buffer, "HTTP/%*s %d", &resp.status_code);
    
    // Find headers/body split
    char *body_start = strstr(buffer, "\r\n\r\n");
    if (!body_start) body_start = strstr(buffer, "\n\n");
    
    int header_len = body_start ? (int)(body_start - buffer) : total;
    
    // Parse headers
    char *headers_copy = malloc(header_len + 1);
    strncpy(headers_copy, buffer, header_len);
    headers_copy[header_len] = '\0';
    
    char *line = strtok(headers_copy, "\r\n");
    line = strtok(NULL, "\r\n");
    
    int cap = 10;
    resp.headers = malloc(sizeof(HttpHeader) * cap);
    
    while (line && line[0] != '\0') {
        char *colon = strchr(line, ':');
        if (colon) {
            if (resp.header_count >= cap) {
                cap *= 2;
                resp.headers = realloc(resp.headers, sizeof(HttpHeader) * cap);
            }
            int key_len = colon - line;
            resp.headers[resp.header_count].key = malloc(key_len + 1);
            strncpy(resp.headers[resp.header_count].key, line, key_len);
            resp.headers[resp.header_count].key[key_len] = '\0';
            
            char *val = colon + 1;
            while (*val == ' ') val++;
            int val_len = strlen(val);
            if (val_len > 0 && val[val_len - 1] == '\r') val_len--;
            
            resp.headers[resp.header_count].value = malloc(val_len + 1);
            strncpy(resp.headers[resp.header_count].value, val, val_len);
            resp.headers[resp.header_count].value[val_len] = '\0';
            
            resp.header_count++;
        }
        line = strtok(NULL, "\r\n");
    }
    
    // Extract body
    if (body_start) {
        int body_offset = (strstr(buffer, "\r\n\r\n") ? 4 : 2);
        resp.body = malloc(total - (body_start - buffer) - body_offset + 1);
        strcpy(resp.body, body_start + body_offset);
    } else {
        resp.body = malloc(1);
        resp.body[0] = '\0';
    }
    
    free(headers_copy);
    free(buffer);
    return resp;
}

HttpResponse http_get(const char *url, HttpHeader *headers, int header_count) {
    char host[256], path[1024];
    parse_url(url, host, path);
    
    int sock = connect_socket(host);
    if (sock < 0) return (HttpResponse){0, NULL, 0, NULL};
    
    char request[4096];
    sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n", path, host);
    
    for (int i = 0; i < header_count; i++) {
        sprintf(request + strlen(request), "%s: %s\r\n", headers[i].key, headers[i].value);
    }
    strcat(request, "\r\n");
    
    return send_request(sock, request);
}

HttpResponse http_post(const char *url, const char *body, HttpHeader *headers, int header_count) {
    char host[256], path[1024];
    parse_url(url, host, path);
    
    int sock = connect_socket(host);
    if (sock < 0) return (HttpResponse){0, NULL, 0, NULL};
    
    char request[8192];
    sprintf(request, "POST %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nContent-Length: %zu\r\n", 
            path, host, strlen(body));
    
    for (int i = 0; i < header_count; i++) {
        sprintf(request + strlen(request), "%s: %s\r\n", headers[i].key, headers[i].value);
    }
    sprintf(request + strlen(request), "\r\n%s", body);
    
    return send_request(sock, request);
}

void http_response_free(HttpResponse response) {
    for (int i = 0; i < response.header_count; i++) {
        free(response.headers[i].key);
        free(response.headers[i].value);
    }
    free(response.headers);
    free(response.body);
}

Value* native_http_init(Env* e, int argc, Value** argv) {
    http_init();
    return vnull();
}

Value* native_http_cleanup(Env* e, int argc, Value** argv) {
    http_cleanup();
    return vnull();
}

Value* native_http_post(Env* e, int argc, Value** argv) {
    HttpResponse resp = http_post(GET_STRING(argv[0]), GET_STRING(argv[1]), NULL, 0);
    Value* d = call_native_with(NULL, native_new_dict, NULL);
    val_release(call_native_with(NULL, native_set_dict,
        val_retain(d), vstring_dup("body"), vstring_dup(resp.body), NULL));
    val_release(call_native_with(NULL, native_set_dict,
        val_retain(d), vstring_dup("status"), vint(resp.status_code), NULL));
    Value* header = call_native_with(NULL, native_new_dict, NULL);
    for (int i=0; i<resp.header_count; ++i)
        val_release(call_native_with(NULL, native_set_dict,
        val_retain(header), vstring_dup(resp.headers[i].key), vstring_dup(resp.headers[i].value), NULL));
    val_release(call_native_with(NULL, native_set_dict,
        val_retain(d), vstring_dup("headers"), val_retain(header), NULL));
    http_response_free(resp);
    return d;
}

void env_register_http_ext(Env* g) {
    env_register_native(g, "http.init", native_http_init);
    env_register_native(g, "http.post", native_http_post);
    env_register_native(g, "http.cleanup", native_http_cleanup);
}