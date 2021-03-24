cmake_minimum_required (VERSION 3.4.3)

# cd build
# "C:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 16 2019" ../

project (d3d11-sample)

file(GLOB SOURCES_ *.cpp)
file(GLOB HEADERS_ *.h)
file(GLOB SHADERS_ *.hlsl)

source_group("shaders" FILES ${SHADERS_})

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
if (_cpp_latest_flag_supported)
    add_compile_options("/std:c++latest")
else()
	add_compile_options("/std:c++17")
endif()

set(DX11_LIBS dxguid dxgi d3d11 d3dcompiler windowscodecs)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
add_executable(d3d11-sample ${SOURCES_} ${HEADERS_} ${SHADERS_}
3rdparty/imgui/imgui.cpp
3rdparty/imgui/imgui_demo.cpp
3rdparty/imgui/imgui_draw.cpp
3rdparty/imgui/imgui_tables.cpp
3rdparty/imgui/imgui_widgets.cpp
3rdparty/imgui/imgui_impl_dx11.cpp
3rdparty/imgui/imgui_impl_win32.cpp
)
set_source_files_properties(${SHADERS_} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(d3d11-sample ${DX11_LIBS})
target_precompile_headers(d3d11-sample PRIVATE pch.h)

target_include_directories(d3d11-sample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(d3d11-sample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui)
set_target_properties(d3d11-sample PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:d3d11-sample>")

