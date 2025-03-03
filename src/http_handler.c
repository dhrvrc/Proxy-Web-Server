#include "http_handler.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

// Helper function: Write all bytes from buffer to sock.
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


int connect_to_server(const char *host, int port) {
    struct addrinfo hints, *res, *p;
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;        // IPv4 (or use AF_UNSPEC for IPv4 & IPv6)
    hints.ai_socktype = SOCK_STREAM;    // TCP

    int status = getaddrinfo(host, port_str, &hints, &res);
    if (status != 0) {
        log_message(LOG_LEVEL_ERROR, "getaddrinfo error for host %s: %s", host, gai_strerror(status));
        return -1;
    }

    int sock = -1;
    for (p = res; p != NULL; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock < 0)
            continue;
        if (connect(sock, p->ai_addr, p->ai_addrlen) == 0)
            break;
        close(sock);
        sock = -1;
    }
    freeaddrinfo(res);

    if (sock < 0) {
        log_message(LOG_LEVEL_ERROR, "Failed to connect to %s:%d", host, port);
    }
    return sock;
}

/**
 * Reads data from the client socket and parses the HTTP request.
 * For CONNECT methods, it expects the URL to be in the form "host:port".
 * For other methods, it extracts the host from an absolute URL.
 */
int parse_http_request(int client_sock, HttpRequest *request) {
    char buffer[4096];
    int bytes_read = read(client_sock, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        log_message(LOG_LEVEL_ERROR, "Failed to read from client socket");
        return -1;
    }
    buffer[bytes_read] = '\0';

    // Log the raw request at DEBUG level.
    log_message(LOG_LEVEL_DEBUG, "Raw request: %s", buffer);

    // Parse the request line: METHOD URL HTTP/1.x
    if (sscanf(buffer, "%15s %1023s", request->method, request->url) != 2) {
        log_message(LOG_LEVEL_ERROR, "Failed to parse request line");
        return -1;
    }

    // If the request is a CONNECT (HTTPS), the URL is "host:port".
    if (strcmp(request->method, "CONNECT") == 0) {
        char *colon = strchr(request->url, ':');
        if (colon) {
            *colon = '\0';
            strncpy(request->host, request->url, MAX_HOST_SIZE);
            request->port = atoi(colon + 1);
        } else {
            strncpy(request->host, request->url, MAX_HOST_SIZE);
            request->port = 443; // default for HTTPS
        }
    } else {
        // For other methods, assume the URL is absolute (e.g., http://host/path).
        char *host_start = strstr(request->url, "://");
        if (host_start) {
            host_start += 3;
        } else {
            host_start = request->url;
        }
        char *host_end = strchr(host_start, '/');
        int host_len = host_end ? (host_end - host_start) : strlen(host_start);
        if (host_len >= MAX_HOST_SIZE) {
            host_len = MAX_HOST_SIZE - 1;
        }
        strncpy(request->host, host_start, host_len);
        request->host[host_len] = '\0';
        request->port = 80; // default for HTTP
    }

    log_message(LOG_LEVEL_DEBUG, "Parsed Request - Method: %s, URL: %s, Host: %s, Port: %d",
                request->method, request->url, request->host, request->port);
    return 0;
}

/**
 * Forwards a non-CONNECT (HTTP) request to the destination server and relays the response.
 */
int handle_http(int client_sock, const HttpRequest *request) {
    int server_sock = connect_to_server(request->host, request->port);
    if (server_sock < 0) {
        log_message(LOG_LEVEL_ERROR, "Unable to connect to server %s:%d", request->host, request->port);
        return -1;
    }

    // Forward a minimal HTTP/1.0 request.
    char forward_buffer[4096];
    snprintf(forward_buffer, sizeof(forward_buffer),
             "%s %s HTTP/1.0\r\nHost: %s\r\n\r\n",
             request->method, request->url, request->host);
    if (write_all(server_sock, forward_buffer, strlen(forward_buffer)) < 0) {
        log_message(LOG_LEVEL_ERROR, "Failed to send request to server");
        close(server_sock);
        return -1;
    }

    // Relay the response from the server back to the client.
    char buffer[4096];
    int bytes;
    while ((bytes = read(server_sock, buffer, sizeof(buffer))) > 0) {
        if (write_all(client_sock, buffer, bytes) < 0) {
            log_message(LOG_LEVEL_ERROR, "Failed to send response to client");
            break;
        }
    }

    close(server_sock);
    return 0;
}

/**
 * Handles an HTTPS CONNECT request by establishing a tunnel between the client and the destination server.
 */
int handle_https(int client_sock, const HttpRequest *request) {
    int server_sock = connect_to_server(request->host, request->port);
    if (server_sock < 0) {
        log_message(LOG_LEVEL_ERROR, "Unable to connect to HTTPS server %s:%d", request->host, request->port);
        return -1;
    }

    // Inform the client that the connection is established.
    const char *conn_established = "HTTP/1.1 200 Connection Established\r\n\r\n";
    if (write_all(client_sock, conn_established, strlen(conn_established)) < 0) {
        log_message(LOG_LEVEL_ERROR, "Failed to send connection established message to client");
        close(server_sock);
        return -1;
    }

    // Set up a loop to tunnel data between the client and server.
    fd_set readfds;
    int maxfd = (client_sock > server_sock) ? client_sock : server_sock;
    char buffer[4096];
    int n;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(client_sock, &readfds);
        FD_SET(server_sock, &readfds);

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            log_message(LOG_LEVEL_ERROR, "Select error in HTTPS tunnel");
            break;
        }
        if (FD_ISSET(client_sock, &readfds)) {
            n = read(client_sock, buffer, sizeof(buffer));
            if (n <= 0)
                break;
            if (write_all(server_sock, buffer, n) < 0)
                break;
        }
        if (FD_ISSET(server_sock, &readfds)) {
            n = read(server_sock, buffer, sizeof(buffer));
            if (n <= 0)
                break;
            if (write_all(client_sock, buffer, n) < 0)
                break;
        }
    }
    close(server_sock);
    return 0;
}
