#include "win_gui_visualizer.h"
#include <iostream>

#ifdef _WIN32

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // 创建可视化对象
    WinGUIVisualizer visualizer;
    
    // 初始化窗口
    if (!visualizer.initializeWindow(hInstance, nCmdShow)) {
        MessageBoxW(nullptr, L"窗口初始化失败！", L"错误", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // 消息循环
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    return 0;
}

#else

int main() {
    std::cout << "此GUI版本仅支持Windows平台。" << std::endl;
    return 1;
}

#endif