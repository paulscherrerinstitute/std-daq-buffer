file(GLOB SOURCES
        src/*.cpp)

add_library(std-buffer-lib STATIC ${SOURCES})
target_include_directories(std-buffer-lib PUBLIC include/)
target_link_libraries(std-buffer-lib
        external)

add_subdirectory(test/)