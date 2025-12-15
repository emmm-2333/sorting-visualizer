cmake_minimum_required(VERSION 4.0)
project(12_15)

set(CMAKE_CXX_STANDARD 20)

# 添加编译选项以支持Unicode
if(WIN32)
    # 对于MinGW编译器，使用-finput-charset=utf-8选项
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-finput-charset=utf-8>)
    
    # Windows特定的链接库
    set(WINDOWS_LIBRARIES gdi32)
endif()

# 主控制台版本
add_executable(12_15 main.cpp sorting_system.cpp)

# Windows GUI版本
if(WIN32)
    add_executable(win_gui_visualizer WIN32 main_win_gui.cpp win_gui_visualizer.cpp)
    target_link_libraries(win_gui_visualizer PRIVATE ${WINDOWS_LIBRARIES})
endif()