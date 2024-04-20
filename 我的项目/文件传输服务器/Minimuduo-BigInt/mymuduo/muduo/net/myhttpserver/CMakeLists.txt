set(http_SRCS
  HttpContext.cpp
  HttpRequest.cpp
  HttpResponse.cpp
  HttpServer.cpp
  Util.cpp
  )

add_library(muduo_myhttp ${http_SRCS})
target_link_libraries(muduo_myhttp muduo_net)

install(TARGETS muduo_myhttp DESTINATION lib)
set(HEADERS
  HttpContext.h
  HttpRequest.h
  HttpResponse.h
  HttpServer.h
  Util.h
  ParseType.h
  )
install(FILES ${HEADERS} DESTINATION include/muduo/net/myhttp)

if(NOT CMAKE_BUILD_NO_EXAMPLES)
add_executable(myhttpd tests/myhttpd.cpp)
target_link_libraries(myhttpd muduo_myhttp)


endif()
add_definitions(-std=c++11)
# add_subdirectory(tests)
