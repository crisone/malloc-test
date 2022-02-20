#include <iostream>
#include <thread>
#include <pthread.h>

#include "malloc_trace.h"

void f1(void)
{
    pthread_setname_np(pthread_self(), "F1");
    for (int i = 0; i < 100; ++i)
    {
        int *a = new int[10];
        std::this_thread::sleep_for(std::chrono::seconds(1));
        delete[] a;
    }
}

void f2(void)
{
    pthread_setname_np(pthread_self(), "F2");
    for (int i = 0; i < 50; ++i)
    {
        int *a = new int[10];
        std::this_thread::sleep_for(std::chrono::seconds(2));
        delete[] a;
    }
}

int main(void)
{
    printf("Hello World!\n");

    my_init_hook();

    std::thread t1(f1);
    std::thread t2(f2);

    // do query
    while (true)
    {
        int th_count = thread_count;
        for (int i = 0; i < th_count; ++i)
        {
            MallocInfo *info = &malloc_info_array[i];
            std::cout << "[";
            std::cout << info->name << ": " << info->malloc_count << ", " << info->free_count;
            std::cout << "]";
        }
        std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    t1.join();
    t2.join();
}