# Do not try to compile a full blown executable as this would depend on standard
# C and syscalls
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_SYSTEM_NAME Generic)

# Find arm-none-eabi
find_program(C_COMPILER arm-none-eabi-gcc)
find_program(CXX_COMPILER arm-none-eabi-g++)
find_program(AR arm-none-eabi-ar)
find_program(OBJCOPY arm-none-eabi-objcopy)
find_program(OBJDUMP arm-none-eabi-objdump)
find_program(SIZE arm-none-eabi-size)

set(CMAKE_ASM_COMPILER ${C_COMPILER})
set(CMAKE_C_COMPILER ${C_COMPILER})
set(CMAKE_CXX_COMPILER ${CXX_COMPILER})
set(CMAKE_AR ${AR})
set(CMAKE_OBJCOPY ${OBJCOPY})
set(CMAKE_OBJDUMP ${OBJDUMP})
set(CMAKE_SIZE ${SIZE})

# Architecture flags
include(${CMAKE_CURRENT_LIST_DIR}/arm_arch.cmake)

set(CMAKE_ASM_FLAGS "${ARCH}")
set(CMAKE_C_FLAGS "${ARCH}")
set(CMAKE_CXX_FLAGS "${ARCH}")
set(CMAKE_C_FLAGS_DEBUG "-Og -g")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -Os -g")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Os -g")
