#ifndef HTTP_H
#define HTTP_H

#include <stdlib.h>

typedef struct {
    char *key;
    char *value;
} HttpHeader;

typedef struct {
    int status_code;
    HttpHeader *headers;
    int header_count;
    char *body;
} HttpResponse;

// Initialize HTTP library (required on Windows, safe to call on Unix)
void http_init(void);

// Cleanup HTTP library (required on Windows, safe to call on Unix)
void http_cleanup(void);

// Make GET request to URL. Return response struct.
HttpResponse http_get(const char *url, HttpHeader *headers, int header_count);

// Make POST request to URL with body. Return response struct.
HttpResponse http_post(const char *url, const char *body, HttpHeader *headers, int header_count);

// Free response memory
void http_response_free(HttpResponse response);

// Ext Binding
void env_register_http_ext(Env* g);

#endif