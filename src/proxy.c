#include "proxy.h"
#include "logging.h"
#include "http_handler.h"
#include "cache.h"
#include "console.h"  // For is_url_blocked() and remove_cache_by_url()
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// Forward declaration for connect_to_server (implemented in http_handler.c).
int connect_to_server(const char *host, int port);

// Local helper function: Write all bytes.
static ssize_t write_all(int sock, const void *buffer, size_t length) {
    size_t total_written = 0;
    const char *buf = (const char *)buffer;
    while (total_written < length) {
        ssize_t written = write(sock, buf + total_written, length - total_written);
        if (written <= 0) {
            return -1;
        }
        total_written += written;
    }
    return total_written;
}

void handle_client_connection(int client_sock) {
    log_message(LOG_LEVEL_INFO, "Handling client on socket %d", client_sock);

    HttpRequest req;
    if (parse_http_request(client_sock, &req) < 0) {
        log_message(LOG_LEVEL_ERROR, "Failed to parse HTTP request on socket %d", client_sock);
        close(client_sock);
        return;
    }

    // Check if the requested host is blocked.
    if (is_url_blocked(req.host)) {
        // Remove any cached entry for this host.
        remove_cache_by_url(req.host);
        const char *block_response = "HTTP/1.1 403 Forbidden\r\nContent-Length: 13\r\n\r\nAccess Denied";
        if (write_all(client_sock, block_response, strlen(block_response)) < 0) {
            log_message(LOG_LEVEL_ERROR, "Failed to send blocked response to client");
        }
        log_message(LOG_LEVEL_INFO, "Blocked URL: %s", req.host);
        close(client_sock);
        return;
    }

    // For GET requests (non-CONNECT), attempt to serve from cache.
    if (strcmp(req.method, "CONNECT") != 0 && strcmp(req.method, "GET") == 0) {
        CacheEntry cached;
        if (lookup_cache(req.url, &cached)) {
            log_message(LOG_LEVEL_INFO, "Serving cached content for %s", req.url);
            if (write_all(client_sock, cached.response, cached.response_length) < 0) {
                log_message(LOG_LEVEL_ERROR, "Failed to send cached response to client");
            }
            free(cached.url);
            free(cached.response);
            close(client_sock);
            return;
        }
    }

    // Dispatch based on the request method.
    if (strcmp(req.method, "CONNECT") == 0) {
        // HTTPS: establish a tunnel.
        handle_https(client_sock, &req);
    } else {
        // HTTP: forward the request and capture the response.
        struct timeval start, end;
        gettimeofday(&start, NULL);

        int server_sock = connect_to_server(req.host, req.port);
        if (server_sock < 0) {
            log_message(LOG_LEVEL_ERROR, "Unable to connect to server %s:%d", req.host, req.port);
            close(client_sock);
            return;
        }

        // Forward the HTTP request to the destination server.
        char forward_buffer[4096];
        snprintf(forward_buffer, sizeof(forward_buffer),
                 "%s %s HTTP/1.0\r\nHost: %s\r\n\r\n",
                 req.method, req.url, req.host);
        if (write_all(server_sock, forward_buffer, strlen(forward_buffer)) < 0) {
            log_message(LOG_LEVEL_ERROR, "Failed to send request to server");
            close(server_sock);
            close(client_sock);
            return;
        }

        // Relay the response from the server back to the client while accumulating for caching.
        char buffer[4096];
        int bytes;
        int total_length = 0;
        int capacity = 4096;
        char *response_buffer = (char *)malloc(capacity);
        if (!response_buffer) {
            log_message(LOG_LEVEL_ERROR, "Memory allocation failed for response buffer");
            close(server_sock);
            close(client_sock);
            return;
        }

        while ((bytes = read(server_sock, buffer, sizeof(buffer))) > 0) {
            if (write_all(client_sock, buffer, bytes) < 0) {
                log_message(LOG_LEVEL_ERROR, "Failed to relay data to client");
                break;
            }
            if (total_length + bytes > capacity) {
                capacity = (total_length + bytes) * 2;
                char *new_buffer = realloc(response_buffer, capacity);
                if (!new_buffer) {
                    log_message(LOG_LEVEL_ERROR, "Memory allocation failed during response accumulation");
                    free(response_buffer);
                    close(server_sock);
                    close(client_sock);
                    return;
                }
                response_buffer = new_buffer;
            }
            memcpy(response_buffer + total_length, buffer, bytes);
            total_length += bytes;
        }
        close(server_sock);

        gettimeofday(&end, NULL);
        double time_taken = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);
        log_message(LOG_LEVEL_INFO, "HTTP request to %s completed in %.3f seconds", req.url, time_taken);

        // Cache the response if this is a GET request.
        if (strcmp(req.method, "GET") == 0) {
            insert_cache(req.url, response_buffer, total_length, time_taken);
        }
        free(response_buffer);
    }
    close(client_sock);
    log_message(LOG_LEVEL_INFO, "Closed connection on socket %d", client_sock);
}

int create_server_socket(int port) {
    int server_sock;
    struct sockaddr_in server_addr;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        log_message(LOG_LEVEL_ERROR, "Failed to create socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        log_message(LOG_LEVEL_WARN, "Failed to set socket options");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        log_message(LOG_LEVEL_ERROR, "Failed to bind socket to port %d", port);
        close(server_sock);
        return -1;
    }

    if (listen(server_sock, 10) < 0) {
        log_message(LOG_LEVEL_ERROR, "Failed to listen on socket");
        close(server_sock);
        return -1;
    }

    log_message(LOG_LEVEL_INFO, "Server socket created, listening on port %d", port);
    return server_sock;
}

int accept_client(int server_sock) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
    if (client_sock < 0) {
        log_message(LOG_LEVEL_ERROR, "Failed to accept client connection");
        return -1;
    }
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    log_message(LOG_LEVEL_INFO, "Accepted connection from %s:%d", client_ip, ntohs(client_addr.sin_port));
    return client_sock;
}
