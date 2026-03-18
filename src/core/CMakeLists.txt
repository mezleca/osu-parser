file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/**/*.cpp"
)

file(GLOB_RECURSE CORE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/**/*.hpp"
)

add_library(osu_parser_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(osu_parser_core PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
set_target_properties(osu_parser_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

find_package(ZLIB QUIET)
if (NOT ZLIB_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        zlib
        URL https://github.com/madler/zlib/archive/refs/tags/v1.3.1.tar.gz
        URL_HASH SHA256=17e88863f3600672ab49182f217281b6fc4d3c762bde361935e436a95214d05c
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(zlib)
    if (NOT TARGET ZLIB::ZLIB)
        if (TARGET zlibstatic)
            add_library(ZLIB::ZLIB ALIAS zlibstatic)
        elseif (TARGET zlib)
            add_library(ZLIB::ZLIB ALIAS zlib)
        endif()
    endif()
endif()

target_link_libraries(osu_parser_core PUBLIC ZLIB::ZLIB)
