#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#define MAX_METHOD_LEN    8
#define MAX_URL_LEN       2048
#define MAX_VERSION_LEN   16
#define MAX_HEADER_LINES  100
#define MAX_HEADER_KEY    128
#define MAX_HEADER_VALUE  1024

typedef struct {
    char method[MAX_METHOD_LEN];
    char url[MAX_URL_LEN];
    char version[MAX_VERSION_LEN];
} RequestLine;

typedef struct {
    char key[MAX_HEADER_KEY];
    char value[MAX_HEADER_VALUE];
} Header;

typedef struct {
    RequestLine request_line;
    Header headers[MAX_HEADER_LINES];
    int header_count;
} HttpRequest;

/**
 * Parses a raw HTTP request into an HttpRequest structure.
 *
 * @param raw_request The null-terminated raw HTTP request string.
 * @param request Pointer to an HttpRequest structure to populate.
 * @return 0 on success, -1 on failure.
 */
int parse_http_request(const char *raw_request, HttpRequest *request);

#endif
