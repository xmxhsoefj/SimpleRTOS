cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_QUIET OFF)

include(FetchContent)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/stm32f7-toolchain.cmake
    CACHE STRING "Toolchain file")
project(firmware C ASM)

set(CPU_FLAGS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)

# FreeRTOS is not yet integrated
# add_library(freertos STATIC
#     lib/FreeRTOS/tasks.c
#     lib/FreeRTOS/queue.c
#     lib/FreeRTOS/list.c
#     lib/FreeRTOS/timers.c
#     lib/FreeRTOS/event_groups.c
#     lib/FreeRTOS/stream_buffer.c
#     lib/FreeRTOS/portable/GCC/ARM_CM7/r0p1/port.c
#     lib/FreeRTOS/portable/MemMang/heap_4.c
# )

# target_include_directories(freertos PUBLIC
#     lib/FreeRTOS/include
#     lib/FreeRTOS/portable/GCC/ARM_CM7/r0p1
#     include
# )

# target_compile_options(freertos PRIVATE ${CPU_FLAGS})

FetchContent_Declare(
    cmsis6
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS_6.git
    GIT_TAG        v6.3.0 # latest non-dev non-main tag atm

    GIT_PROGRESS   TRUE
    USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(cmsis6)

FetchContent_Declare(
    cmsis6_f7
    GIT_REPOSITORY https://github.com/STMicroelectronics/cmsis-device-f7.git
    GIT_TAG        v1.2.10

    GIT_PROGRESS   TRUE
    USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(cmsis6_f7)

FetchContent_Declare(
    hal_f7
    GIT_REPOSITORY https://github.com/STMicroelectronics/stm32f7xx-hal-driver.git
    GIT_TAG        v1.3.3

    GIT_PROGRESS   TRUE
    USES_TERMINAL_DOWNLOAD TRUE
)
# FetchContent_MakeAvailable(hal_f7)

add_library(cmsis STATIC
    ${cmsis6_f7_SOURCE_DIR}/Source/Templates/system_stm32f7xx.c
)

# There's an option of using HAL, but I don't want to
# add_library(hal STATIC
#     ${hal_f7_SOURCE_DIR}/Src/stm32f7xx_hal.c
# )

target_include_directories(cmsis PUBLIC
target_include_directories(cmsis PUBLIC
    ${cmsis6_SOURCE_DIR}/CMSIS/Core/Include
    ${cmsis6_f7_SOURCE_DIR}/Include # with system_stm32f7xx.h and stm32f769xx.h
    # ${hal_f7_SOURCE_DIR}/Inc
)

target_compile_options(cmsis PRIVATE ${CPU_FLAGS})
target_compile_definitions(cmsis PUBLIC STM32F769xx)

add_executable(firmware
    ${cmsis6_f7_SOURCE_DIR}/Source/Templates/gcc/startup_stm32f769xx.s
    src/main.c
    # src/startup.c
)

set_target_properties(firmware PROPERTIES
    SUFFIX ".elf"
)

target_include_directories(firmware PRIVATE include)

target_compile_options(firmware PRIVATE ${CPU_FLAGS})

target_link_libraries(firmware PRIVATE cmsis) # hal) # freertos)

target_link_options(firmware PRIVATE
    ${CPU_FLAGS}
    -T${CMAKE_SOURCE_DIR}/linker.ld
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/firmware.map
    --specs=nosys.specs
    --specs=nano.specs
)

add_custom_command(TARGET firmware POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:firmware> ${CMAKE_BINARY_DIR}/firmware.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:firmware>
    COMMENT "Generating firmware.bin"
)

add_custom_target(flash
    COMMAND openocd
        -f interface/stlink.cfg
        -f target/stm32f7x.cfg
        -c "program $<TARGET_FILE:firmware> verify reset exit"
    DEPENDS firmware
    COMMENT "Flashing firmware"
)
