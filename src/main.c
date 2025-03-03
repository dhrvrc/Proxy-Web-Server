#include "logging.h"
#include "proxy.h"
#include "cache.h"
#include "management_console.h"  
#include "thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define PORT 8080
#define NUM_THREADS 4

// Global shutdown flag.
volatile sig_atomic_t shutdown_requested = 0;
// Global server socket variable.
int server_sock = -1;

void handle_signal(int sig) {
    (void)sig; // Unused parameter.
    shutdown_requested = 1;
    // Close the server socket to break out of the blocking accept().
    if (server_sock != -1) {
        close(server_sock);
        server_sock = -1;
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    // Register signal handlers for graceful shutdown.
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Initialize logging (logs go to "proxy.log", with DEBUG and above).
    init_logging("proxy.log", LOG_LEVEL_DEBUG);

    // Initialize cache.
    init_cache();

    // Start the admin (management) console thread.
    start_admin_console_thread();

    // Initialize the thread pool with a fixed number of worker threads.
    ThreadPool *pool = thread_pool_init(NUM_THREADS);
    if (!pool) {
        log_message(LOG_LEVEL_ERROR, "Failed to initialize thread pool");
        exit(EXIT_FAILURE);
    }

    // Create the server socket.
    server_sock = create_server_socket(PORT);
    if (server_sock < 0) {
        log_message(LOG_LEVEL_ERROR, "Failed to create server socket");
        exit(EXIT_FAILURE);
    }
    log_message(LOG_LEVEL_INFO, "Proxy server listening on port %d", PORT);

    // Accept incoming client connections and enqueue them to the thread pool.
    while (!shutdown_requested) {
        int client_sock = accept_client(server_sock);
        if (client_sock < 0) {
            // If shutdown_requested is set, break the loop.
            if (shutdown_requested) break;
            log_message(LOG_LEVEL_ERROR, "Error accepting client connection");
            continue;
        }
        thread_pool_enqueue(pool, client_sock);
    }

    log_message(LOG_LEVEL_INFO, "Shutdown signal received. Cleaning up...");

    // Cleanup resources.
    thread_pool_destroy(pool);
    free_cache();
    stop_admin_console_thread();
    close_logging();

    return 0;
}
