#include "thread_pool.h"
#include "logging.h"
#include <stdlib.h>
#include <pthread.h>

// Structure for a task in the work queue.
typedef struct task_t {
    int client_sock;
    struct task_t *next;
} task_t;

// Definition of the thread pool structure.
struct thread_pool {
    pthread_t *threads;       // Array of worker threads.
    int num_threads;          // Number of worker threads.
    task_t *task_queue_head;  // Head of the task queue.
    task_t *task_queue_tail;  // Tail of the task queue.
    pthread_mutex_t queue_mutex;  // Mutex to protect access to the queue.
    pthread_cond_t queue_cond;      // Condition variable for task availability.
    int shutdown;             // Flag indicating whether the pool is shutting down.
};

// Forward declaration of the worker thread function.
static void *thread_worker(void *arg);

// External function that handles a client connection. You must implement this function in your proxy module.
extern void handle_client_connection(int client_sock);

ThreadPool *thread_pool_init(int num_threads) {
    ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
    if (!pool) {
        log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for thread pool");
        return NULL;
    }
    pool->num_threads = num_threads;
    pool->shutdown = 0;
    pool->task_queue_head = NULL;
    pool->task_queue_tail = NULL;
    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_cond, NULL);
    
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    if (!pool->threads) {
        log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for thread pool threads");
        free(pool);
        return NULL;
    }
    
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, thread_worker, pool) != 0) {
            log_message(LOG_LEVEL_ERROR, "Failed to create worker thread %d", i);
            pool->shutdown = 1;
            pthread_cond_broadcast(&pool->queue_cond);
            for (int j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }
    log_message(LOG_LEVEL_INFO, "Thread pool initialized with %d threads", num_threads);
    return pool;
}

int thread_pool_enqueue(ThreadPool *pool, int client_sock) {
    if (pool == NULL) return -1;
    
    task_t *new_task = (task_t *)malloc(sizeof(task_t));
    if (!new_task) {
        log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for new task");
        return -1;
    }
    new_task->client_sock = client_sock;
    new_task->next = NULL;
    
    pthread_mutex_lock(&pool->queue_mutex);
    if (pool->task_queue_tail == NULL) {
        pool->task_queue_head = new_task;
        pool->task_queue_tail = new_task;
    } else {
        pool->task_queue_tail->next = new_task;
        pool->task_queue_tail = new_task;
    }
    pthread_cond_signal(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
    
    return 0;
}

void thread_pool_destroy(ThreadPool *pool) {
    if (pool == NULL) return;
    
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
    
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    // Free remaining tasks.
    while (pool->task_queue_head) {
        task_t *task = pool->task_queue_head;
        pool->task_queue_head = task->next;
        free(task);
    }
    
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_cond);
    free(pool->threads);
    free(pool);
    
    log_message(LOG_LEVEL_INFO, "Thread pool destroyed");
}

static void *thread_worker(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;
    while (1) {
        pthread_mutex_lock(&pool->queue_mutex);
        // Wait until there is a task or the pool is shutting down.
        while (pool->task_queue_head == NULL && !pool->shutdown) {
            pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
        }
        if (pool->shutdown && pool->task_queue_head == NULL) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }
        // Dequeue a task.
        task_t *task = pool->task_queue_head;
        if (task) {
            pool->task_queue_head = task->next;
            if (pool->task_queue_head == NULL) {
                pool->task_queue_tail = NULL;
            }
        }
        pthread_mutex_unlock(&pool->queue_mutex);
        
        if (task) {
            // Process the task by handling the client connection.
            handle_client_connection(task->client_sock);
            free(task);
        }
    }
    return NULL;
}
