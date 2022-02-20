#include <malloc.h>
#include <string.h>

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

static void my_init_hook(void);
static void *my_malloc_hook(size_t, const void *);
static void my_free_hook(void *, const void *);

static void *(*old_malloc_hook)(size_t, const void *);
static void (*old_free_hook)(void *, const void *);

struct MallocInfo
{
    char name[64];
    uint64_t malloc_count = 0;
    uint64_t malloc_sum_t = 0;
    uint64_t free_count = 0;
    uint64_t free_sum_t = 0;
};

static MallocInfo malloc_info_array[1000];
thread_local MallocInfo *malloc_info_ptr = nullptr;

static std::atomic<int> thread_count(0);

static void my_init_hook(void)
{
    old_malloc_hook = __malloc_hook;
    old_free_hook = __free_hook;
    __malloc_hook = my_malloc_hook;
    __free_hook = my_free_hook;
}

static void *my_malloc_hook(size_t size, const void *caller)
{
    void *result;
    /* Restore all old hooks */
    __malloc_hook = old_malloc_hook;
    __free_hook = old_free_hook;
    /* Call recursively */
    result = malloc(size);
    /* Save underlying hooks */
    old_malloc_hook = __malloc_hook;
    old_free_hook = __free_hook;
    /* printf might call malloc, so protect it too. */
    // printf("malloc (%u) returns %p\n", (unsigned int)size, result);
    // std::thread::id id = std::this_thread::get_id();
    // struct thread_info_t thread_info;

    if (malloc_info_ptr == nullptr)
    {
        int c = thread_count++;
        malloc_info_ptr = &malloc_info_array[c];
    }
    else
    {
        char thread_name[64];
        pthread_getname_np(pthread_self(), thread_name, 64);
        strncpy(malloc_info_ptr->name, thread_name, 64);
        malloc_info_ptr->malloc_count++;
    }

    /* Restore our own hooks */
    __malloc_hook = my_malloc_hook;
    __free_hook = my_free_hook;
    return result;
}

static void my_free_hook(void *ptr, const void *caller)
{
    /* Restore all old hooks */
    __malloc_hook = old_malloc_hook;
    __free_hook = old_free_hook;
    /* Call recursively */
    free(ptr);
    /* Save underlying hooks */
    old_malloc_hook = __malloc_hook;
    old_free_hook = __free_hook;
    /* printf might call free, so protect it too. */
    // printf("freed pointer %p\n", ptr);
    // std::thread::id id = std::this_thread::get_id();
    if (malloc_info_ptr == nullptr)
    {
        int c = thread_count++;
        malloc_info_ptr = &malloc_info_array[c];
    }
    else
    {
        char thread_name[64];
        pthread_getname_np(pthread_self(), thread_name, 64);
        strncpy(malloc_info_ptr->name, thread_name, 64);
        malloc_info_ptr->free_count++;
    }

    /* Restore our own hooks */
    __malloc_hook = my_malloc_hook;
    __free_hook = my_free_hook;
}