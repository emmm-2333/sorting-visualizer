#include "win_gui_visualizer.h"
#include <iostream>
#include <commctrl.h>

#ifdef _WIN32

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 初始化公共控件库
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);
    
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