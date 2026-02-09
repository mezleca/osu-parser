cmake_minimum_required(VERSION 3.16)

if(WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_CXX_COMPILER "C:/Program Files/LLVM/bin/clang-cl.exe")
    set(CMAKE_LINKER "C:/Program Files/LLVM/bin/lld-link.exe")
    set(CMAKE_MT "C:/Program Files/LLVM/bin/llvm-mt.exe")
    set(CMAKE_RC_COMPILER "C:/Program Files/LLVM/bin/llvm-rc.exe")
endif()

project(osu-beatmap-parser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS "src/native/osu/*.cpp")
file(GLOB_RECURSE COMMON_HEADERS CONFIGURE_DEPENDS "src/native/*.hpp")

if(EMSCRIPTEN)
    add_executable(${PROJECT_NAME} "src/native/wasm.cpp" ${COMMON_SOURCES} ${COMMON_HEADERS})
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".js")

    target_link_options(${PROJECT_NAME} PRIVATE
        --bind
        "-sEXPORTED_FUNCTIONS=['_malloc','_free']"
        "-sEXPORTED_RUNTIME_METHODS=['HEAPU8']"
        -sALLOW_MEMORY_GROWTH=1
        -sMODULARIZE=1
        -sEXPORT_NAME='create_osu_parser'
        -sSINGLE_FILE=1
        "-sENVIRONMENT=['web','worker','node']"
        -sSTRICT=1
        -sFILESYSTEM=0
        -Oz
    )

    target_compile_options(${PROJECT_NAME} PRIVATE -Oz -flto)
else()
    add_library(${PROJECT_NAME} SHARED "src/native/addon.cpp" ${COMMON_SOURCES} ${COMMON_HEADERS} ${CMAKE_JS_SRC})
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

    execute_process(
        COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

    target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR} ${CMAKE_JS_INC})
    target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})
    target_compile_definitions(${PROJECT_NAME} PRIVATE NODE_ADDON_API_CPP_EXCEPTIONS)

    # windows-specific linker flag
    if(WIN32)
        target_link_libraries(${PROJECT_NAME} delayimp.lib)
    endif()

    # clang-cl needs /EHsc for exception handling
    if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE /EHsc)
    endif()
endif()

# copy compile_commands.json
add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
)
