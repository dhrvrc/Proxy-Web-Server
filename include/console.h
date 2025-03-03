#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

// Adds a URL to the block list.
// Returns 0 on success or if already blocked, or a negative value on error.
int block_url(const char *url);

// Removes a URL from the block list.
// Returns 0 on success, or a negative value if the URL was not found.
int unblock_url(const char *url);

// Checks if a URL is blocked.
// Returns 1 if blocked, 0 if not.
int is_url_blocked(const char *url);

#ifdef __cplusplus
}
#endif

#endif // CONSOLE_H
