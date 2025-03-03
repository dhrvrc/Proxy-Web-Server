#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#define MAX_METHOD_SIZE 16
#define MAX_URL_SIZE 1024
#define MAX_HOST_SIZE 256

typedef struct {
    char method[MAX_METHOD_SIZE];
    char url[MAX_URL_SIZE];
    char host[MAX_HOST_SIZE];
    int port;
} HttpRequest;

/**
 * Parses the HTTP request from the given client socket and populates the HttpRequest structure.
 *
 * @param client_sock The client socket file descriptor.
 * @param request Pointer to an HttpRequest structure to populate.
 * @return 0 on success, -1 on failure.
 */
int parse_http_request(int client_sock, HttpRequest *request);

/**
 * Handles an HTTP request (non-CONNECT).
 *
 * @param client_sock The client socket file descriptor.
 * @param request Pointer to the parsed HttpRequest.
 * @return 0 on success, -1 on failure.
 */
int handle_http(int client_sock, const HttpRequest *request);

/**
 * Handles an HTTPS CONNECT tunnel.
 *
 * @param client_sock The client socket file descriptor.
 * @param request Pointer to the parsed HttpRequest (should contain host and port).
 * @return 0 on success, -1 on failure.
 */
int handle_https(int client_sock, const HttpRequest *request);

int connect_to_server(const char *host, int port);

#endif // HTTP_HANDLER_H
