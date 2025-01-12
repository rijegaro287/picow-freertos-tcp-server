# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)
include(./lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(picow-tcp-server-freertos C CXX ASM)

pico_sdk_init()

set(SOURCES src/main.c
            src/tcp_server.c
            src/processing.c
)

add_executable(picow-tcp-server-freertos ${SOURCES})

pico_enable_stdio_uart(picow-tcp-server-freertos 1)
pico_enable_stdio_usb(picow-tcp-server-freertos 0)

target_include_directories(picow-tcp-server-freertos PRIVATE
                           ${CMAKE_CURRENT_LIST_DIR}
                           include
)

target_link_libraries(picow-tcp-server-freertos
                      pico_cyw43_arch_lwip_sys_freertos
                      pico_stdlib
                      pico_multicore
                      FreeRTOS-Kernel-Heap4
)

pico_add_extra_outputs(picow-tcp-server-freertos)
