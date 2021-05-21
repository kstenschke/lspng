cmake_minimum_required(VERSION 3.10)
project(lspng)

include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(Threads)

find_package(PNG REQUIRED)
if (PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIR})
    include_directories(${ZLIB_INCLUDE_DIR})
    add_definitions(-DUSE_LIBPNG)
else()
    message(WARNING "libpng or one of its dependencies couldn't be found."
            + "Transparency may not be correctly detected.")
endif()

add_executable(lspng CImg/CImg.h lspng/main.cc)

target_link_libraries(lspng
        X11
        pthread
        m
        png)

install(TARGETS
        ${PROJECT_NAME}
        RUNTIME DESTINATION /usr/bin/)
