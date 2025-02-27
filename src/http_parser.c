#include "http_parser.h"
#include <stdio.h>
#include <string.h>

int parse_http_request(const char *raw_request, HttpRequest *request) {
    if (!raw_request || !request) return -1;
    
    // Find the end of the request line.
    const char *line_end = strstr(raw_request, "\r\n");
    if (!line_end) return -1;
    int line_len = line_end - raw_request;
    
    char request_line_str[4096];
    if (line_len >= sizeof(request_line_str)) return -1;
    strncpy(request_line_str, raw_request, line_len);
    request_line_str[line_len] = '\0';

    // Parse the method, URL, and version from the request line.
    if (sscanf(request_line_str, "%s %s %s",
               request->request_line.method,
               request->request_line.url,
               request->request_line.version) != 3) {
        return -1;
    }
    
    // Parse the headers.
    const char *headers_start = line_end + 2; // Skip "\r\n"
    request->header_count = 0;
    const char *current = headers_start;
    char line[4096];
    
    while (1) {
        const char *next_line = strstr(current, "\r\n");
        if (!next_line) break;
        int len = next_line - current;
        if (len == 0) { // Empty line indicates the end of headers.
            break;
        }
        if (len >= sizeof(line)) len = sizeof(line) - 1;
        strncpy(line, current, len);
        line[len] = '\0';
        
        // Find the colon separator between key and value.
        char *colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char *key = line;
            char *value = colon + 1;
            // Trim leading spaces in the value.
            while (*value == ' ') value++;
            strncpy(request->headers[request->header_count].key, key, MAX_HEADER_KEY - 1);
            request->headers[request->header_count].key[MAX_HEADER_KEY - 1] = '\0';
            strncpy(request->headers[request->header_count].value, value, MAX_HEADER_VALUE - 1);
            request->headers[request->header_count].value[MAX_HEADER_VALUE - 1] = '\0';
            request->header_count++;
            if (request->header_count >= MAX_HEADER_LINES) break;
        }
        
        current = next_line + 2; // Move past "\r\n"
    }
    
    return 0;
}
