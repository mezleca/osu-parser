cmake_minimum_required(VERSION 3.16)

if(WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_CXX_COMPILER "C:/Program Files/LLVM/bin/clang-cl.exe")
    set(CMAKE_LINKER "C:/Program Files/LLVM/bin/lld-link.exe")
    set(CMAKE_MT "C:/Program Files/LLVM/bin/llvm-mt.exe")
    set(CMAKE_RC_COMPILER "C:/Program Files/LLVM/bin/llvm-rc.exe")
endif()

project(osu_parser VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("src/core")

add_library(osu_parser SHARED "src/native/addon.cpp" ${CMAKE_JS_SRC})
target_sources(osu_parser PRIVATE
    "src/native/bindings/beatmap.cpp"
    "src/native/bindings/osu_db.cpp"
    "src/native/bindings/collection_db.cpp"
    "src/native/bindings/scores_db.cpp"
    "src/native/bindings/replay.cpp"
    "src/native/bindings/osdb.cpp"
)
set_target_properties(osu_parser PROPERTIES PREFIX "" SUFFIX ".node" OUTPUT_NAME "osu-parser")

execute_process(
    COMMAND node -p "require('node-addon-api').include"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE NODE_ADDON_API_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

target_include_directories(osu_parser PRIVATE ${NODE_ADDON_API_DIR} ${CMAKE_JS_INC} "src")
target_link_libraries(osu_parser osu_parser_core ${CMAKE_JS_LIB})
target_compile_definitions(osu_parser PRIVATE NODE_ADDON_API_CPP_EXCEPTIONS)

if(WIN32)
    target_link_libraries(osu_parser delayimp.lib)
endif()

if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(osu_parser PRIVATE /EHsc)
endif()

add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
)
