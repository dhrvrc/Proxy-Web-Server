#include "cache.h"
#include "logging.h"
#include "console.h"  // To use is_url_blocked()
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_CACHE_ENTRIES 100

// Global array to hold cache entries and a count of entries.
static CacheEntry *cache_entries[MAX_CACHE_ENTRIES];
static int cache_count = 0;
static pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_cache() {
    pthread_mutex_lock(&cache_mutex);
    cache_count = 0;
    for (int i = 0; i < MAX_CACHE_ENTRIES; i++) {
        cache_entries[i] = NULL;
    }
    pthread_mutex_unlock(&cache_mutex);
    log_message(LOG_LEVEL_INFO, "Cache initialized");
}

int lookup_cache(const char *url, CacheEntry *entry) {
    pthread_mutex_lock(&cache_mutex);
    for (int i = 0; i < cache_count; i++) {
        if (strcmp(cache_entries[i]->url, url) == 0) {
            // Increment frequency on cache hit.
            cache_entries[i]->frequency++;
            // Copy data into provided entry.
            entry->url = strdup(cache_entries[i]->url);
            entry->response = (char *)malloc(cache_entries[i]->response_length);
            memcpy(entry->response, cache_entries[i]->response, cache_entries[i]->response_length);
            entry->response_length = cache_entries[i]->response_length;
            entry->time_taken = cache_entries[i]->time_taken;
            entry->frequency = cache_entries[i]->frequency;
            pthread_mutex_unlock(&cache_mutex);
            log_message(LOG_LEVEL_DEBUG, "Cache hit for URL: %s (frequency now %d)", url, entry->frequency);
            return 1;
        }
    }
    pthread_mutex_unlock(&cache_mutex);
    log_message(LOG_LEVEL_DEBUG, "Cache miss for URL: %s", url);
    return 0;
}

void insert_cache(const char *url, const char *response, int response_length, double time_taken) {
    // Check if the URL is blocked. If so, do not cache it.
    if (is_url_blocked(url)) {
        log_message(LOG_LEVEL_INFO, "Not caching blocked URL: %s", url);
        return;
    }
    
    pthread_mutex_lock(&cache_mutex);
    // If the cache is full, remove the LFU entry.
    if (cache_count >= MAX_CACHE_ENTRIES) {
        int lfu_index = 0;
        for (int i = 1; i < cache_count; i++) {
            if (cache_entries[i]->frequency < cache_entries[lfu_index]->frequency) {
                lfu_index = i;
            }
        }
        log_message(LOG_LEVEL_INFO, "Cache full. Removing LFU entry for URL: %s (frequency %d)",
                    cache_entries[lfu_index]->url, cache_entries[lfu_index]->frequency);
        free(cache_entries[lfu_index]->url);
        free(cache_entries[lfu_index]->response);
        free(cache_entries[lfu_index]);
        // Shift entries left.
        for (int i = lfu_index + 1; i < cache_count; i++) {
            cache_entries[i - 1] = cache_entries[i];
        }
        cache_count--;
    }
    // Create a new cache entry.
    CacheEntry *new_entry = (CacheEntry *)malloc(sizeof(CacheEntry));
    new_entry->url = strdup(url);
    new_entry->response = (char *)malloc(response_length);
    memcpy(new_entry->response, response, response_length);
    new_entry->response_length = response_length;
    new_entry->time_taken = time_taken;
    new_entry->frequency = 1;  // Initialize frequency to 1.
    cache_entries[cache_count++] = new_entry;
    pthread_mutex_unlock(&cache_mutex);
    log_message(LOG_LEVEL_INFO, "Inserted cache entry for URL: %s", url);
}

void remove_cache_by_url(const char *url) {
    pthread_mutex_lock(&cache_mutex);
    int i = 0;
    while (i < cache_count) {
        if (strcmp(cache_entries[i]->url, url) == 0) {
            free(cache_entries[i]->url);
            free(cache_entries[i]->response);
            free(cache_entries[i]);
            // Shift remaining entries left.
            for (int j = i; j < cache_count - 1; j++) {
                cache_entries[j] = cache_entries[j + 1];
            }
            cache_count--;
            // Do not increment i, new entry is now at index i.
        } else {
            i++;
        }
    }
    pthread_mutex_unlock(&cache_mutex);
    log_message(LOG_LEVEL_INFO, "Removed cache entries for URL: %s", url);
}

void free_cache() {
    pthread_mutex_lock(&cache_mutex);
    for (int i = 0; i < cache_count; i++) {
        if (cache_entries[i]) {
            free(cache_entries[i]->url);
            free(cache_entries[i]->response);
            free(cache_entries[i]);
            cache_entries[i] = NULL;
        }
    }
    cache_count = 0;
    pthread_mutex_unlock(&cache_mutex);
    log_message(LOG_LEVEL_INFO, "Cache cleared");
}
