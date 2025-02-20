# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)  # Mantendo a versão correta do SDK
set(toolchainVersion 13_3_Rel1)  # Mantendo apenas a versão mais recente
set(picotoolVersion 2.0.0)  # Mantendo a versão 2.0.0 conforme o SDK 1.5.1
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)

if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

# ====================================================================================
# Generated CMake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Definir a placa Pico antes de importar o SDK
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(blink C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(blink blink.c)

# Set program name and version
pico_set_program_name(blink "blink")
pico_set_program_version(blink "0.1")

# Mantendo PIO (necessário para WS2812)
pico_generate_pio_header(blink ${CMAKE_CURRENT_LIST_DIR}/ws2818b.pio)

# Habilita a saída USB para printf
pico_enable_stdio_usb(blink 1)
pico_enable_stdio_uart(blink 0)

# Vincula as bibliotecas necessárias
target_link_libraries(blink 
    pico_stdlib 
    hardware_adc       # Biblioteca para o ADC
    hardware_pwm       # Biblioteca para o PWM
    hardware_pio       # Biblioteca para PIO (necessária para NeoPixel/WS2812)
    hardware_clocks    # Biblioteca para controle de clocks
)

# Verifica se o suporte ao CYW43 está habilitado
if (PICO_CYW43_SUPPORTED)
    target_link_libraries(blink pico_cyw43_arch_none)
endif()

# Cria arquivos adicionais (map/bin/hex)
pico_add_extra_outputs(blink)


