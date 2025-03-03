#include "console.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Structure for a blocked URL in a singly linked list.
typedef struct BlockedURL {
    char *url;
    struct BlockedURL *next;
} BlockedURL;

static BlockedURL *blocked_list = NULL;
static pthread_mutex_t block_mutex = PTHREAD_MUTEX_INITIALIZER;

int block_url(const char *url) {
    pthread_mutex_lock(&block_mutex);
    // Check if URL is already blocked.
    BlockedURL *curr = blocked_list;
    while (curr) {
        if (strcmp(curr->url, url) == 0) {
            pthread_mutex_unlock(&block_mutex);
            return 0; // Already blocked.
        }
        curr = curr->next;
    }
    // Create a new node.
    BlockedURL *new_node = (BlockedURL *)malloc(sizeof(BlockedURL));
    if (!new_node) {
        pthread_mutex_unlock(&block_mutex);
        return -1;
    }
    new_node->url = strdup(url);
    new_node->next = blocked_list;
    blocked_list = new_node;
    pthread_mutex_unlock(&block_mutex);
    log_message(LOG_LEVEL_INFO, "Blocked URL: %s", url);
    return 0;
}

int unblock_url(const char *url) {
    pthread_mutex_lock(&block_mutex);
    BlockedURL *prev = NULL;
    BlockedURL *curr = blocked_list;
    while (curr) {
        if (strcmp(curr->url, url) == 0) {
            if (prev)
                prev->next = curr->next;
            else
                blocked_list = curr->next;
            free(curr->url);
            free(curr);
            pthread_mutex_unlock(&block_mutex);
            log_message(LOG_LEVEL_INFO, "Unblocked URL: %s", url);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    pthread_mutex_unlock(&block_mutex);
    log_message(LOG_LEVEL_WARN, "Attempted to unblock URL not in block list: %s", url);
    return -1;
}

int is_url_blocked(const char *host) {
    FILE *fp = fopen("block_list.txt", "r");
    if (!fp) {
        // If the file doesn't exist, assume no URLs are blocked.
        return 0;
    }
    char blocked_url[256];
    int blocked = 0;
    while (fgets(blocked_url, sizeof(blocked_url), fp)) {
        // Remove any newline characters.
        blocked_url[strcspn(blocked_url, "\n")] = '\0';
        if (strlen(blocked_url) > 0 && strstr(host, blocked_url) != NULL) {
            blocked = 1;
            break;
        }
    }
    fclose(fp);
    return blocked;
}