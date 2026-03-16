file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/**/*.cpp"
)

file(GLOB_RECURSE CORE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/**/*.hpp"
)

add_library(osu_parser_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(osu_parser_core PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
set_target_properties(osu_parser_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
