#include "management_console.h"
#include "console.h"   // For block_url() and unblock_url()
#include "cache.h"     // Added to use remove_cache_by_url()
#include "logging.h"
#include "proxy.h"     // For the global shutdown_requested flag.
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static pthread_t admin_thread;
static volatile int admin_thread_running = 0;

static void *admin_console_thread_func(void *arg) {
    (void)arg;
    while (admin_thread_running) {
        FILE *fp = fopen("admin_cmd.txt", "r");
        if (fp) {
            char command[1024];
            if (fgets(command, sizeof(command), fp) != NULL) {
                // Remove newline character.
                command[strcspn(command, "\n")] = '\0';
                if (strncmp(command, "block ", 6) == 0) {
                    const char *url = command + 6;
                    block_url(url);
                    // Remove any cache entries for this newly blocked URL.
                    remove_cache_by_url(url);
                    log_message(LOG_LEVEL_INFO, "Admin command executed: block %s", url);
                } else if (strncmp(command, "unblock ", 8) == 0) {
                    const char *url = command + 8;
                    unblock_url(url);
                    log_message(LOG_LEVEL_INFO, "Admin command executed: unblock %s", url);
                } else if (strcmp(command, "list") == 0) {
                    log_message(LOG_LEVEL_INFO, "Admin command executed: list");
                    /* Listing currently is a placeholder.
                       In a more advanced implementation you might expose the current block list via an API. */
                } else if (strcmp(command, "quit") == 0) {
                    log_message(LOG_LEVEL_INFO, "Admin command executed: quit");
                    shutdown_requested = 1;
                } else {
                    log_message(LOG_LEVEL_WARN, "Unknown admin command: %s", command);
                }
            }
            fclose(fp);
            remove("admin_cmd.txt");
        }
        sleep(1);
    }
    return NULL;
}

void start_admin_console_thread(void) {
    admin_thread_running = 1;
    if (pthread_create(&admin_thread, NULL, admin_console_thread_func, NULL) != 0) {
        log_message(LOG_LEVEL_ERROR, "Failed to start admin console thread");
    } else {
        log_message(LOG_LEVEL_INFO, "Admin console thread started");
    }
}

void stop_admin_console_thread(void) {
    admin_thread_running = 0;
    pthread_join(admin_thread, NULL);
    log_message(LOG_LEVEL_INFO, "Admin console thread stopped");
}
