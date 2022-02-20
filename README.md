# MALLOC-TEST

使用glibc提供的malloc_hook，统计不同线程内 malloc, free 的次数和累计时间

# 备注
- 使用atomic<int>避免加锁操作
- 使用thread_local variable 记录结构体地址
- 主线程进行查询