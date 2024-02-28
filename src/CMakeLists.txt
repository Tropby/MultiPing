
add_executable(MultiPing main.cpp MultiPing.cpp)
target_link_libraries(MultiPing -lws2_32 -liphlpapi)

