cmake_minimum_required(VERSION 3.10)
project(embedded_sensor_hub C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Windows specific settings
if(WIN32)
    add_compile_definitions(_WIN32 _CRT_SECURE_NO_WARNINGS)
    add_compile_options(-Wno-deprecated-declarations)
endif()

# Compiler options
add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Os
    -fdata-sections
    -ffunction-sections
)

add_link_options(
    -Wl,--gc-sections
    -static
)

# Main executable
add_executable(sensor_hub
    main.c
    src/kernel/scheduler.c
    src/kernel/queue.c
    src/kernel/semaphore.c
    src/hal/hal.c
    src/hal/virt_periph.c
    src/algorithms/kalman_filter.c
    src/protocols/comm_protocol.c
    src/ota/bootloader.c
    src/ota/ota_manager.c
    src/app/sensor_task.c
    src/app/comm_task.c
    src/app/monitor_task.c
    src/utils/logger.c
    src/utils/circular_buffer.c
    simulator/virt_board.c
    simulator/visualization.c
)

target_include_directories(sensor_hub PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/src/algorithms
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocols
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ota
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/simulator
)

# Test executable
add_executable(sensor_hub_test
    tests/test_runner.c
    tests/test_utils.c
    src/kernel/scheduler.c
    src/kernel/queue.c
    src/kernel/semaphore.c
    src/algorithms/kalman_filter.c
    src/utils/logger.c
    src/hal/hal.c
)

target_include_directories(sensor_hub_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/src/algorithms
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)