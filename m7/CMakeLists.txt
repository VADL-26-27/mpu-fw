cmake_minimum_required(VERSION 3.22.0)

# Locate MCUXpresso SDK.
get_filename_component(
    SdkRootDirPath
    "${CMAKE_CURRENT_LIST_DIR}/../../mcuxsdk"
    ABSOLUTE
)

include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(mpu_m7
    LANGUAGES C CXX ASM
    PROJECT_BOARD_PORT_PATH ${board_root}/${board}/demo_apps/hello_world
)

include(${SdkRootDirPath}/CMakeLists.txt)

# Collect application and board source files.
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/*.c"
)

mcux_add_source(
    BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
    SOURCES ${APP_SOURCES}
)

mcux_add_include(
    BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}
    INCLUDES
        include
        board
        freertos
)

# M7 resource table.
mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES middleware/multicore/remoteproc/empty_rsc_table.c
)

# Board-wide binary conversion settings.
include(
    ${SdkRootDirPath}/examples/_boards/${board}/project_common.cmake
    OPTIONAL
)

mcux_convert_binary(
    BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin
)