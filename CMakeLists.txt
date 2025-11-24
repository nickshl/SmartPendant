cmake_minimum_required(VERSION 4.0.0)

project(SmartPendant
  VERSION 1.0.0
  LANGUAGES ASM C CXX)

file(GLOB_RECURSE SRC
  DevCore/*.cpp
  Drivers/*.c Middlewares/*.c Src/*.c Startup/*.s
  Application/*.cpp
)
add_executable(SmartPendant ${SRC})

target_compile_definitions(SmartPendant PRIVATE $<$<CONFIG:DEBUG>:DEBUG>
  USE_HAL_DRIVER STM32F411xE INCLUDE_xQueueGetMutexHolder)

target_include_directories(
  SmartPendant PRIVATE
  Inc
  Application
  DevCore
  DevCore/Display
  DevCore/Display/Fonts
  DevCore/Drivers
  DevCore/Framework
  DevCore/FreeRtosWrapper
  DevCore/Interfaces
  DevCore/Libraries
  DevCore/Math
  DevCore/Tasks
  DevCore/UiEngine
  Drivers/CMSIS/Device/ST/STM32F4xx/Include
  Drivers/CMSIS/Include
  Drivers/STM32F4xx_HAL_Driver/Inc
  Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
  Middlewares/ST/STM32_USB_Device_Library/Core/Inc
  Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
  Middlewares/Third_Party/FreeRTOS/Source/include
  Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
  Middlewares/Third_Party/FatFs/src
)

target_link_libraries(
  SmartPendant
  PRIVATE --specs=nano.specs -Wl,--gc-sections,-Map=${PROJECT_NAME}.map
  -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F411CEUX_FLASH.ld
)

target_compile_definitions(SmartPendant
  PRIVATE SWAP_BUTTONS=1)
        
add_custom_command(
  TARGET SmartPendant
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME} ${PROJECT_NAME}.elf
  COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}
          ${PROJECT_NAME}_${PROJECT_VERSION}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}
          ${PROJECT_NAME}_${PROJECT_VERSION}.bin
  COMMAND ${CMAKE_OBJDUMP} --source --all-headers --demangle --line-numbers
          --wide ${PROJECT_NAME} > ${PROJECT_NAME}.lst
  COMMAND ${CMAKE_SIZE} --format=berkeley ${PROJECT_NAME}
  COMMENT "Generate .hex, .bin and .lst from .elf file")
