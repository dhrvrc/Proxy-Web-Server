#ifndef THREAD_POOL_H
#define THREAD_POOL_H

// Opaque structure representing the thread pool.
typedef struct thread_pool ThreadPool;

/**
 * Initializes a thread pool with a specified number of threads.
 *
 * @param num_threads The number of worker threads in the pool.
 * @return A pointer to the newly created ThreadPool, or NULL on failure.
 */
ThreadPool *thread_pool_init(int num_threads);

/**
 * Enqueues a client socket to be processed by the thread pool.
 *
 * @param pool Pointer to the thread pool.
 * @param client_sock The client socket file descriptor.
 * @return 0 on success, -1 on failure.
 */
int thread_pool_enqueue(ThreadPool *pool, int client_sock);

/**
 * Destroys the thread pool and frees all allocated resources.
 *
 * @param pool Pointer to the thread pool.
 */
void thread_pool_destroy(ThreadPool *pool);

#endif // THREAD_POOL_H
