#include "http.c"
#include <stdio.h>

int main() {
    http_init();
    
    // GET request
    HttpResponse get_resp = http_get("http://example.com", NULL, 0);
    printf("GET Status: %d\n", get_resp.status_code);
    printf("Headers: %d\n", get_resp.header_count);
    for (int i = 0; i < get_resp.header_count; i++) {
        printf("  %s: %s\n", get_resp.headers[i].key, get_resp.headers[i].value);
    }
    printf("Body: %s...\n\n", get_resp.body);
    http_response_free(get_resp);
    
    // POST request
    HttpHeader hdrs[] = {{"Content-Type", "application/json"}};
    HttpResponse post_resp = http_post("http://example.com/api", 
                                       "{\"test\": \"data\"}", hdrs, 1);
    printf("POST Status: %d\n", post_resp.status_code);
    printf("Headers: %d\n", post_resp.header_count);
    for (int i = 0; i < post_resp.header_count; i++) {
        printf("  %s: %s\n", post_resp.headers[i].key, post_resp.headers[i].value);
    }
    printf("Body: %s...\n\n", post_resp.body);
    http_response_free(post_resp);
    
    http_cleanup();
    return 0;
}