add_executable(std-buffer-tests main.cpp)

target_link_libraries(std-buffer-tests
        std-buffer-lib
        external
        rt
        zmq
        gtest)
