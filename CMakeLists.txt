cmake_minimum_required(VERSION 4.1.0)

project("SuperMosaic")


set(SDL3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sdl_cmake)
find_package(SDL3)

add_executable(SuperMosaic
	SuperMosaic/main.cpp
	SuperMosaic/SNES.cpp
	SuperMosaic/CPU.cpp
	SuperMosaic/CPU_AddrModes.cpp
	SuperMosaic/CPU_Instructions.cpp
	SuperMosaic/Bus.cpp
	SuperMosaic/DMA.cpp
	SuperMosaic/Joypad.cpp
	SuperMosaic/Cartridge.cpp
	SuperMosaic/PPU.cpp
	SuperMosaic/PPU_Rendering.cpp
	SuperMosaic/PPU_Rendering.cpp
)

target_include_directories(SuperMosaic PUBLIC ${SDL3_INCLUDE_DIRS})
target_compile_definitions(SuperMosaic PRIVATE SDL_MAIN_USE_CALLBACKS)

target_link_libraries(SuperMosaic PUBLIC SDL3)