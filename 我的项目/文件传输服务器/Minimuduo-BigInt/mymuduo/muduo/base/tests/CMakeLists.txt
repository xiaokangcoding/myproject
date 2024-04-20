add_executable(timestamp_unittest Timestamp_unittest.cc)
target_link_libraries(timestamp_unittest muduo_base)

add_executable(atomic_unittest Atomic_unittest.cc)
target_link_libraries(atomic_unittest muduo_base)

add_executable(exception_unittest Exception_test.cc)
target_link_libraries(exception_unittest muduo_base)

add_executable(Thread_unittest Thread_test.cc)
target_link_libraries(Thread_unittest muduo_base)

add_executable(mutex_unittest Mutex_test.cc)
target_link_libraries(mutex_unittest muduo_base)

add_executable(boundedblockingqueue_unittest BoundedBlockingQueue_test.cc)
target_link_libraries(boundedblockingqueue_unittest muduo_base)

add_executable(threadpool_unittest ThreadPool_test.cc)
target_link_libraries(threadpool_unittest muduo_base)
