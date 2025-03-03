#ifndef CACHE_H
#define CACHE_H

/**
 * Represents a cached HTTP response.
 */
typedef struct {
    char *url;           // Dynamically allocated URL key.
    char *response;      // Dynamically allocated response data.
    int response_length; // Length of the response data in bytes.
    double time_taken;   // Time taken (in seconds) to fetch the response.
    int frequency;       // Frequency count for LFU caching.
} CacheEntry;

/**
 * Initializes the cache system.
 */
void init_cache();

/**
 * Looks up a cache entry by URL.
 *
 * @param url The URL to search for.
 * @param entry Pointer to a CacheEntry structure to be filled with the cached data if found.
 *              The caller is responsible for freeing 'entry->url' and 'entry->response' if a cache hit occurs.
 * @return 1 if the entry is found, 0 otherwise.
 */
int lookup_cache(const char *url, CacheEntry *entry);

/**
 * Inserts a new cache entry.
 *
 * @param url The URL to cache.
 * @param response The response data to cache.
 * @param response_length The length of the response data.
 * @param time_taken The time taken (in seconds) to fetch the response.
 */
void insert_cache(const char *url, const char *response, int response_length, double time_taken);

/**
 * Removes any cache entries corresponding to the given URL.
 */
void remove_cache_by_url(const char *url);

/**
 * Frees all allocated cache entries and cleans up resources.
 */
void free_cache();

#endif // CACHE_H
