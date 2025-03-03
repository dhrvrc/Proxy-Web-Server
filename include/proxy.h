#ifndef PROXY_H
#define PROXY_H
#include <signal.h>
extern volatile sig_atomic_t shutdown_requested;

/**
 * Creates a server socket listening on the specified port.
 * @param port The port number to listen on.
 * @return The server socket file descriptor, or -1 on error.
 */
int create_server_socket(int port);

/**
 * Accepts an incoming client connection on the server socket.
 * @param server_sock The server socket file descriptor.
 * @return The client socket file descriptor, or -1 on error.
 */
int accept_client(int server_sock);

/**
 * Spawns a new thread to handle the client connection.
 * @param client_sock The client socket file descriptor.
 */
void spawn_client_handler(int client_sock);

/**
 * The function that handles a client connection.
 * @param arg A pointer to the client socket file descriptor.
 * @return NULL.
 */
void *client_handler(void *arg);

#endif // PROXY_H
