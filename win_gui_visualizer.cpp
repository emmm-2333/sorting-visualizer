#include "win_gui_visualizer.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <thread>
#include <chrono>
#include <string>

#include <uxtheme.h>
#include <wchar.h>

#ifndef SET_WINDOW_THEME_DYNAMIC
#define SET_WINDOW_THEME_DYNAMIC
static void applyExplorerTheme(HWND h) {
    HMODULE hUx = LoadLibraryW(L"uxtheme.dll");
    if (!hUx) return;
    using PFNSetWindowTheme = HRESULT (WINAPI*)(HWND, LPCWSTR, LPCWSTR);
    auto pSetTheme = reinterpret_cast<PFNSetWindowTheme>(GetProcAddress(hUx, "SetWindowTheme"));
    if (pSetTheme) {
        pSetTheme(h, L"Explorer", nullptr);
    }
    FreeLibrary(hUx);
}
#endif

#ifdef _WIN32

#define WM_APP_UPDATE_STATUS (WM_APP + 1)
#define WM_APP_SET_PAUSE_TEXT (WM_APP + 2)

// Forward declaration for CanvasProc
LRESULT CALLBACK WinGUIVisualizer::CanvasProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WinGUIVisualizer* visualizer = nullptr;
    if (uMsg == WM_CREATE) {
        CREATESTRUCT* pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
        visualizer = reinterpret_cast<WinGUIVisualizer*>(pcs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(visualizer));
    } else {
        visualizer = reinterpret_cast<WinGUIVisualizer*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (visualizer) {
        switch (uMsg) {
            case WM_PAINT:
                visualizer->drawVisualization();
                return 0;
            case WM_ERASEBKGND:
                return 1;
        }
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

WinGUIVisualizer::WinGUIVisualizer()
    : hwnd(nullptr),
      hdc(nullptr),
      // ps left default-initialized
      ps(),
      rect(),
      isSorting(false),
      isPaused(false),
      currentAlgorithm(SortingAlgorithm::BubbleSort),
      animationDelay(50),
      highlightIndex1(-1),
      highlightIndex2(-1),
      barWidth(1),
      maxDataValue(0),
      comparisonCount(0),
      swapCount(0),
      statusText(L"就绪"),
      hwndBtnGenerateRandom(nullptr),
      hwndBtnGenerateAsc(nullptr),
      hwndBtnGenerateDesc(nullptr),
      hwndBtnGeneratePartial(nullptr),
      hwndBtnReset(nullptr),
      hwndBtnBubbleSort(nullptr),
      hwndBtnQuickSort(nullptr),
      hwndBtnMergeSort(nullptr),
      hwndBtnHeapSort(nullptr),
      hwndBtnInsertionSort(nullptr),
      hwndBtnSelectionSort(nullptr),
      hwndBtnCompareAll(nullptr),
      hwndBtnPauseResume(nullptr),
      hwndTrackbarSpeed(nullptr),
      hwndEditDataSize(nullptr),
      hwndStaticStatus(nullptr),
      hwndCanvas(nullptr),
      controlFont(nullptr),
      titleFont(nullptr) {
}

bool WinGUIVisualizer::initializeWindow(HINSTANCE hInstance, int nCmdShow) {
    // 注册窗口类
    const wchar_t CLASS_NAME[] = L"Sorting Visualizer Window";
    
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
    RegisterClassW(&wc);

    // 注册 Canvas 类
    WNDCLASSW wcCanvas = {};
    wcCanvas.lpfnWndProc = CanvasProc;
    wcCanvas.hInstance = hInstance;
    wcCanvas.lpszClassName = L"VisualizerCanvas";
    wcCanvas.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wcCanvas);

    // 创建窗口
    hwnd = CreateWindowExW(
        WS_EX_COMPOSITED, // 尝试启用组合绘制以减少子控件导致的闪烁
        CLASS_NAME,
        L"排序算法性能比较与动画演示系统",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr,
        nullptr,
        hInstance,
        this // 将this指针传递给窗口
    );
    
    if (hwnd == nullptr) {
        return false;
    }
    
    // 创建控件
    createControls();
    layoutControls();
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 生成默认数据
    generateData(30, DataPattern::Random);

    // 只在窗口可见时刷新，canvas 优先
    if (IsWindow(hwnd) && IsWindowVisible(hwnd)) {
        if (hwndCanvas) {
            InvalidateRect(hwndCanvas, nullptr, TRUE);
            UpdateWindow(hwndCanvas);
        } else {
            InvalidateRect(hwnd, nullptr, TRUE);
            UpdateWindow(hwnd);
        }
    }
    return true;
}

void WinGUIVisualizer::createControls() {
    // Create a shared UI font for controls to improve look & reduce flicker
    if (!controlFont) {
        controlFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    }

    // 数据生成按钮
    hwndBtnGenerateRandom = CreateWindowW(L"BUTTON", L"生成随机数据", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 150, 30, hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnGenerateRandom, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnGenerateRandom);

    hwndBtnGenerateAsc = CreateWindowW(L"BUTTON", L"生成有序数据", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 150, 30, hwnd, (HMENU)2, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnGenerateAsc, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnGenerateAsc);

    hwndBtnGenerateDesc = CreateWindowW(L"BUTTON", L"生成逆序数据", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 150, 30, hwnd, (HMENU)3, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnGenerateDesc, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnGenerateDesc);

    hwndBtnGeneratePartial = CreateWindowW(L"BUTTON", L"生成部分有序数据", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 150, 30, hwnd, (HMENU)4, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnGeneratePartial, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnGeneratePartial);

    // 排序算法按钮
    hwndBtnBubbleSort = CreateWindowW(L"BUTTON", L"冒泡排序", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 120, 30, hwnd, (HMENU)11, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnBubbleSort, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnBubbleSort);

    hwndBtnQuickSort = CreateWindowW(L"BUTTON", L"快速排序", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 120, 30, hwnd, (HMENU)12, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnQuickSort, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnQuickSort);

    hwndBtnMergeSort = CreateWindowW(L"BUTTON", L"归并排序", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 120, 30, hwnd, (HMENU)13, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnMergeSort, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnMergeSort);

    hwndBtnHeapSort = CreateWindowW(L"BUTTON", L"堆排序", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 120, 30, hwnd, (HMENU)14, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnHeapSort, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnHeapSort);

    hwndBtnInsertionSort = CreateWindowW(L"BUTTON", L"插入排序", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 120, 30, hwnd, (HMENU)15, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnInsertionSort, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnInsertionSort);

    hwndBtnSelectionSort = CreateWindowW(L"BUTTON", L"选择排序", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 120, 30, hwnd, (HMENU)16, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnSelectionSort, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnSelectionSort);

    // 功能按钮
    hwndBtnCompareAll = CreateWindowW(L"BUTTON", L"性能比较", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 120, 30, hwnd, (HMENU)21, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnCompareAll, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnCompareAll);

    hwndBtnReset = CreateWindowW(L"BUTTON", L"重置数据", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 120, 30, hwnd, (HMENU)22, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnReset, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnReset);

    hwndBtnPauseResume = CreateWindowW(L"BUTTON", L"暂停", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        0, 0, 120, 30, hwnd, (HMENU)23, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndBtnPauseResume, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndBtnPauseResume);

    // 速度滑块
    hwndTrackbarSpeed = CreateWindowW(TRACKBAR_CLASS, L"速度", WS_VISIBLE | WS_CHILD | TBS_AUTOTICKS | TBS_HORZ,
        0, 0, 150, 30, hwnd, (HMENU)31, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndTrackbarSpeed, TBM_SETRANGE, TRUE, MAKELONG(1, 100));
    SendMessageW(hwndTrackbarSpeed, TBM_SETPOS, TRUE, 50);
    SendMessageW(hwndTrackbarSpeed, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndTrackbarSpeed);

    // 数据大小输入框
    hwndEditDataSize = CreateWindowW(L"EDIT", L"30", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
        0, 0, 100, 25, hwnd, (HMENU)41, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndEditDataSize, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndEditDataSize);

    // 状态标签
    hwndStaticStatus = CreateWindowW(L"STATIC", statusText.c_str(), WS_VISIBLE | WS_CHILD,
        0, 0, 400, 25, hwnd, (HMENU)51, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), nullptr);
    SendMessageW(hwndStaticStatus, WM_SETFONT, (WPARAM)controlFont, TRUE);
    applyExplorerTheme(hwndStaticStatus);

    // 创建绘图区 (canvas) 用于双缓冲绘制，避免与控件冲突
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;
    // 初始放置在控件下方，layoutControls 会移动大小
    hwndCanvas = CreateWindowExW(0, L"VisualizerCanvas", nullptr, WS_CHILD | WS_VISIBLE,
                                  10, 150, std::max(100, clientWidth - 20), std::max(100, clientHeight - 160),
                                   hwnd, nullptr, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), this);
}

void WinGUIVisualizer::layoutControls() {
    // 获取客户区宽度用于适配控件位置
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;

    // 防止在布局期间频繁重绘子控件导致闪烁
    SendMessageW(hwnd, WM_SETREDRAW, FALSE, 0);

    // 布局控件（保持原位）
    MoveWindow(hwndBtnGenerateRandom, 10, 30, 150, 30, TRUE);
    MoveWindow(hwndBtnGenerateAsc, 170, 30, 150, 30, TRUE);
    MoveWindow(hwndBtnGenerateDesc, 330, 30, 150, 30, TRUE);
    MoveWindow(hwndBtnGeneratePartial, 490, 30, 150, 30, TRUE);
    MoveWindow(hwndEditDataSize, 650, 30, 100, 30, TRUE);
    MoveWindow(hwndBtnReset, 760, 30, 120, 30, TRUE);
    
    // 第二行控件 (Y = 70)
    MoveWindow(hwndBtnBubbleSort, 10, 70, 120, 30, TRUE);
    MoveWindow(hwndBtnQuickSort, 140, 70, 120, 30, TRUE);
    MoveWindow(hwndBtnMergeSort, 270, 70, 120, 30, TRUE);
    MoveWindow(hwndBtnHeapSort, 400, 70, 120, 30, TRUE);
    MoveWindow(hwndBtnInsertionSort, 530, 70, 120, 30, TRUE);
    MoveWindow(hwndBtnSelectionSort, 660, 70, 120, 30, TRUE);
    MoveWindow(hwndBtnCompareAll, 790, 70, 120, 30, TRUE);
    
    // 第三行控件 (Y = 110)
    MoveWindow(hwndBtnPauseResume, 10, 110, 120, 30, TRUE);
    MoveWindow(hwndTrackbarSpeed, 140, 110, 150, 30, TRUE);
    MoveWindow(hwndStaticStatus, 300, 115, std::min(600, clientWidth - 320), 25, TRUE);

    // 布局绘图区（canvas）位于控件下方，留出顶部 150 像素用于按钮
    int canvasX = 10;
    int canvasY = 150;
    int canvasW = std::max(100, clientWidth - 20);
    int canvasH = std::max(100, clientHeight - canvasY - 20);
    if (hwndCanvas) {
        MoveWindow(hwndCanvas, canvasX, canvasY, canvasW, canvasH, TRUE);
    }

    // 允许重绘并刷新一次（优先重绘 canvas）
    SendMessageW(hwnd, WM_SETREDRAW, TRUE, 0);
    if (hwndCanvas) InvalidateRect(hwndCanvas, nullptr, FALSE);
    else InvalidateRect(hwnd, nullptr, FALSE);
}

LRESULT CALLBACK WinGUIVisualizer::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WinGUIVisualizer* visualizer = nullptr;
    
    if (uMsg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        visualizer = reinterpret_cast<WinGUIVisualizer*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(visualizer));
    } else {
        visualizer = reinterpret_cast<WinGUIVisualizer*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    
    if (visualizer) {
        switch (uMsg) {
            case WM_COMMAND: {
                int wmId = LOWORD(wParam);
                switch (wmId) {
                    // 数据生成按钮
                    case 1:
                        {
                            wchar_t buf[256];
                            GetWindowTextW(visualizer->hwndEditDataSize, buf, 256);
                            int size = _wtoi(buf);
                            if (size <= 0) size = 30;
                            if (size > 200) size = 200; // 限制最大数量以保证可视化效果
                            visualizer->generateData(size, DataPattern::Random);
                            visualizer->updateStatus(L"已生成随机数据");
                        }
                        break;
                    case 2:
                        {
                            wchar_t buf[256];
                            GetWindowTextW(visualizer->hwndEditDataSize, buf, 256);
                            int size = _wtoi(buf);
                            if (size <= 0) size = 30;
                            if (size > 200) size = 200;
                            visualizer->generateData(size, DataPattern::Ascending);
                            visualizer->updateStatus(L"已生成有序数据");
                        }
                        break;
                    case 3:
                        {
                            wchar_t buf[256];
                            GetWindowTextW(visualizer->hwndEditDataSize, buf, 256);
                            int size = _wtoi(buf);
                            if (size <= 0) size = 30;
                            if (size > 200) size = 200;
                            visualizer->generateData(size, DataPattern::Descending);
                            visualizer->updateStatus(L"已生成逆序数据");
                        }
                        break;
                    case 4:
                        {
                            wchar_t buf[256];
                            GetWindowTextW(visualizer->hwndEditDataSize, buf, 256);
                            int size = _wtoi(buf);
                            if (size <= 0) size = 30;
                            if (size > 200) size = 200;
                            visualizer->generateData(size, DataPattern::PartiallySorted);
                            visualizer->updateStatus(L"已生成部分有序数据");
                        }
                        break;
                        
                    // 排序算法按钮
                    case 11:
                        visualizer->startSorting(SortingAlgorithm::BubbleSort);
                        break;
                    case 12:
                        visualizer->startSorting(SortingAlgorithm::QuickSort);
                        break;
                    case 13:
                        visualizer->startSorting(SortingAlgorithm::MergeSort);
                        break;
                    case 14:
                        visualizer->startSorting(SortingAlgorithm::HeapSort);
                        break;
                    case 15:
                        visualizer->startSorting(SortingAlgorithm::InsertionSort);
                        break;
                    case 16:
                        visualizer->startSorting(SortingAlgorithm::SelectionSort);
                        break;
                        
                    // 功能按钮
                    case 21:
                        visualizer->runPerformanceComparison();
                        break;
                    case 22:
                        visualizer->resetData();
                        visualizer->updateStatus(L"数据已重置");
                        break;
                    case 23:
                        visualizer->pauseResume();
                        SetWindowTextW(visualizer->hwndBtnPauseResume, visualizer->isPaused ? L"继续" : L"暂停");
                        break;
                }
                break;
            }
            
            case WM_HSCROLL:
                if ((HWND)lParam == visualizer->hwndTrackbarSpeed) {
                    int pos = (int)SendMessageW(visualizer->hwndTrackbarSpeed, TBM_GETPOS, 0, 0);
                    visualizer->setAnimationSpeed(pos);
                }
                break;
                
            case WM_PAINT:
                // 主窗口无需绘制可视化，canvas 处理绘制
                // 仅验证消息并返回
                ValidateRect(hwnd, NULL);
                break;
                
            case WM_SIZE:
                // 重新布局控件
                visualizer->layoutControls();
                // 重绘 canvas
                if (visualizer->hwndCanvas) InvalidateRect(visualizer->hwndCanvas, NULL, FALSE);
                break;
                
            case WM_ERASEBKGND:
                // 避免背景擦除造成的闪烁
                return 1;

            case WM_DESTROY:
                // 清理共享字体
                if (visualizer->controlFont) {
                    DeleteObject(visualizer->controlFont);
                    visualizer->controlFont = nullptr;
                }
                if (visualizer->titleFont) {
                    DeleteObject(visualizer->titleFont);
                    visualizer->titleFont = nullptr;
                }
                PostQuitMessage(0);
                return 0;

            case WM_APP_UPDATE_STATUS: {
                // lParam is a heap-allocated wide string (wchar_t*) from worker thread
                wchar_t* s = reinterpret_cast<wchar_t*>(lParam);
                if (s) {
                    visualizer->statusText = std::wstring(s);
                    if (visualizer->hwndStaticStatus) {
                        SetWindowTextW(visualizer->hwndStaticStatus, s);
                    }
                    free(s);
                }
                return 0;
            }

            case WM_APP_SET_PAUSE_TEXT: {
                // wParam == 1 -> set to "暂停", 0 -> "继续"
                if (visualizer->hwndBtnPauseResume) {
                    if (wParam) SetWindowTextW(visualizer->hwndBtnPauseResume, L"暂停");
                    else SetWindowTextW(visualizer->hwndBtnPauseResume, L"继续");
                }
                return 0;
            }

            case WM_DRAWITEM: {
                LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
                if (pdis->CtlType == ODT_BUTTON) {
                    HDC hdc = pdis->hDC;
                    RECT rc = pdis->rcItem;
                    // 双缓冲绘制到内存 DC
                    HDC memDC = CreateCompatibleDC(hdc);
                    int width = rc.right - rc.left;
                    int height = rc.bottom - rc.top;
                    HBITMAP bmp = CreateCompatibleBitmap(hdc, width, height);
                    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, bmp);

                    // 在内存 DC 上使用以 (0,0) 为原点的局部矩形，避免首次显示出现黑边/未填充区域
                    RECT localRect = {0, 0, width, height};

                    // 背景颜色
                    COLORREF bg = RGB(40, 120, 200);
                    if (pdis->itemState & ODS_SELECTED) bg = RGB(30, 100, 170);
                    if (pdis->itemState & ODS_DISABLED) bg = RGB(160, 160, 160);

                    HBRUSH br = CreateSolidBrush(bg);
                    FillRect(memDC, &localRect, br);
                    DeleteObject(br);

                    // 边框
                    HPEN pen = CreatePen(PS_SOLID, 1, RGB(20, 20, 20));
                    HGDIOBJ oldPen = SelectObject(memDC, pen);
                    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
                    Rectangle(memDC, localRect.left, localRect.top, localRect.right, localRect.bottom);
                    SelectObject(memDC, oldBrush);
                    SelectObject(memDC, oldPen);
                    DeleteObject(pen);

                    // 文本
                    SetBkMode(memDC, TRANSPARENT);
                    SetTextColor(memDC, RGB(255,255,255));
                    HFONT useFont = visualizer->controlFont ? visualizer->controlFont : (HFONT)GetStockObject(DEFAULT_GUI_FONT);
                    HFONT oldF = (HFONT)SelectObject(memDC, (HGDIOBJ)useFont);
                    wchar_t buf[256];
                    GetWindowTextW(pdis->hwndItem, buf, _countof(buf));
                    DrawTextW(memDC, buf, -1, &localRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    SelectObject(memDC, oldF);

                    // Blit（将内存位图复制到控件位置）
                    BitBlt(hdc, rc.left, rc.top, width, height, memDC, 0, 0, SRCCOPY);

                    // cleanup
                    SelectObject(memDC, oldBmp);
                    DeleteObject(bmp);
                    DeleteDC(memDC);
                    return 0;
                }
                break;
            }
        }
    }
    
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void WinGUIVisualizer::refreshAll() {
    if (!hwnd || !IsWindow(hwnd) || !IsWindowVisible(hwnd)) return;
    RedrawWindow(hwnd, nullptr, nullptr,
                 RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}

void WinGUIVisualizer::generateData(size_t size, DataPattern pattern) {
    std::lock_guard<std::mutex> lg(dataMutex);
    data = generateTestData(size, pattern);
    originalData = data;
    
    // 计算最大值用于缩放
    if (!data.empty()) {
        maxDataValue = *std::max_element(data.begin(), data.end());
    } else {
        maxDataValue = 1;
    }
    
    // 计算柱形宽度 使用客户区宽度以支持缩放
    RECT clientRect;
    GetClientRect(hwndCanvas ? hwndCanvas : hwnd, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    if (size > 0) {
        barWidth = std::max(1, (clientWidth - 40) / static_cast<int>(size));
    } else {
        barWidth = 1;
    }
    if (barWidth > 40) barWidth = 40; // 限制最大宽度

    // 重绘窗口（优先只重绘 canvas，避免重绘所有控件导致闪烁）
    if (hwndCanvas) InvalidateRect(hwndCanvas, nullptr, FALSE);
    else if (hwnd) InvalidateRect(hwnd, nullptr, FALSE);
    // 不再调用 refreshAll
}

std::vector<int> WinGUIVisualizer::generateTestData(size_t size, DataPattern pattern) {
    std::vector<int> testData(size);
    
    // 初始化顺序数据
    for (size_t i = 0; i < size; i++) {
        testData[i] = static_cast<int>(i + 1);
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    switch (pattern) {
        case DataPattern::Random:
            std::shuffle(testData.begin(), testData.end(), gen);
            break;
            
        case DataPattern::Descending:
            std::reverse(testData.begin(), testData.end());
            break;
            
        case DataPattern::PartiallySorted:
            // 将大约70%的数据保持有序，其余打乱
            if (size > 10) {
                size_t unsortedSize = size * 0.3;
                std::shuffle(testData.end() - unsortedSize, testData.end(), gen);
            }
            break;
            
        case DataPattern::Ascending:
            // 默认就是升序，无需处理
            break;
    }
    
    return testData;
}

void WinGUIVisualizer::setData(const std::vector<int>& newData) {
    std::lock_guard<std::mutex> lg(dataMutex);
    data = newData;
    originalData = newData;
    
    // 计算最大值用于缩放
    if (!data.empty()) {
        maxDataValue = *std::max_element(data.begin(), data.end());
    } else {
        maxDataValue = 1;
    }
    
    // 计算柱形宽度
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    size_t size = data.size();
    if (size > 0) {
        barWidth = std::max(1, (clientWidth - 40) / static_cast<int>(size));
    } else {
        barWidth = 1;
    }
    if (barWidth > 40) barWidth = 40;

    // 重绘窗口
    if (hwndCanvas) InvalidateRect(hwndCanvas, nullptr, FALSE);
    else InvalidateRect(hwnd, nullptr, FALSE);
}

void WinGUIVisualizer::drawVisualization() {
    HWND paintTarget = hwndCanvas ? hwndCanvas : hwnd;
    PAINTSTRUCT localPs;
    HDC localHdc = BeginPaint(paintTarget, &localPs);

    // 直接绘制到窗口 HDC（移除双缓冲）
    RECT clientRect;
    GetClientRect(paintTarget, &clientRect);

    // 设置字体支持中文 (局部字体对象)
    HFONT hFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"微软雅黑");
    HFONT oldFont = (HFONT)SelectObject(localHdc, hFont);

    // 填充背景（黑色）
    HBRUSH backgroundBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(localHdc, &clientRect, backgroundBrush);
    DeleteObject(backgroundBrush);

    // 将当前绘制目标设置为窗口 HDC
    HDC prevHdc = hdc;
    hdc = localHdc;

    // 绘制柱形图与 UI
    drawBars();
    drawUI();

    // 恢复 HDC 并清理资源
    hdc = prevHdc;
    SelectObject(localHdc, oldFont);
    DeleteObject(hFont);

    EndPaint(paintTarget, &localPs);
}

void WinGUIVisualizer::drawBars() {
    std::lock_guard<std::mutex> lg(dataMutex);
    if (data.empty()) return;
    
    // 获取客户区大小
    RECT clientRect;
    HWND paintTarget = hwndCanvas ? hwndCanvas : hwnd;
    GetClientRect(paintTarget, &clientRect);
    int clientHeight = clientRect.bottom - clientRect.top;
    int clientWidth = clientRect.right - clientRect.left;

    int baseY = clientHeight - 50; // 底部边距
    int maxHeight = std::max(10, clientHeight - 200); // 最大高度（为控件留出空间）
    if (maxDataValue <= 0) maxDataValue = 1;

    // 如果 barWidth 太小或数据量变化，按客户区重新计算
    if ((int)data.size() > 0) {
        barWidth = std::max(1, (clientWidth - 40) / static_cast<int>(data.size()));
    }

    for (size_t i = 0; i < data.size(); i++) {
        int barHeight = (static_cast<long long>(data[i]) * maxHeight) / maxDataValue;
        int x = 20 + static_cast<int>(i) * barWidth;
        int y = baseY - barHeight;
        
        RECT barRect = {x, y, x + barWidth - 1, baseY};
        
        // 根据状态设置颜色
        HBRUSH brush;
        int hi1 = highlightIndex1.load();
        int hi2 = highlightIndex2.load();
        if (static_cast<int>(i) == hi1 || static_cast<int>(i) == hi2) {
            brush = CreateSolidBrush(RGB(255, 0, 0)); // 红色高亮
        } else {
            brush = CreateSolidBrush(RGB(0, 200, 255)); // 蓝色普通
        }
        
        FillRect(hdc, &barRect, brush);
        DeleteObject(brush);
    }
}

void WinGUIVisualizer::drawUI() {
    // 绘制说明文字
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    
    wchar_t infoText[512];
    swprintf_s(infoText, L"排序算法性能比较与动画演示系统");
    TextOutW(hdc, 10, 5, infoText, (int)wcslen(infoText));
}

void WinGUIVisualizer::highlightBars(int index1, int index2) {
     highlightIndex1.store(index1);
     highlightIndex2.store(index2);
     // 只使窗口无背景擦除刷新来减少闪烁
    if (hwndCanvas) InvalidateRect(hwndCanvas, nullptr, FALSE);
    else InvalidateRect(hwnd, nullptr, FALSE);
     sleepAnimation();
 }

void WinGUIVisualizer::clearHighlights() {
    highlightIndex1.store(-1);
    highlightIndex2.store(-1);
}

void WinGUIVisualizer::swap(int i, int j) {
     if (i != j) {
         {
             std::lock_guard<std::mutex> lg(dataMutex);
             std::swap(data[i], data[j]);
             incrementSwaps();
         }
         // 请求重绘但保留背景（仅重绘 canvas）
         if (hwndCanvas) InvalidateRect(hwndCanvas, nullptr, FALSE);
         else InvalidateRect(hwnd, nullptr, FALSE);
         sleepAnimation();
     }
 }

void WinGUIVisualizer::sleepAnimation() {
    Sleep(animationDelay);
}

void WinGUIVisualizer::resetData() {
     std::lock_guard<std::mutex> lg(dataMutex);
     data = originalData;
     clearHighlights();
    if (hwndCanvas) InvalidateRect(hwndCanvas, nullptr, FALSE);
    else InvalidateRect(hwnd, nullptr, FALSE);
 }

void WinGUIVisualizer::startSorting(SortingAlgorithm algorithm) {
    // 在单独的线程中运行排序算法，避免阻塞UI
    std::thread sortingThread([this, algorithm]() {
        isSorting = true;
        isPaused = false;
        // Update pause button text on UI thread
        if (hwnd) PostMessageW(hwnd, WM_APP_SET_PAUSE_TEXT, 1, 0);
         resetCounters();
         startTimer();

         switch (algorithm) {
             case SortingAlgorithm::BubbleSort:
                updateStatus(L"正在执行冒泡排序...");
                 bubbleSort();
                 break;
             case SortingAlgorithm::QuickSort:
                updateStatus(L"正在执行快速排序...");
                 quickSort();
                 break;
             case SortingAlgorithm::MergeSort:
                updateStatus(L"正在执行归并排序...");
                 mergeSort();
                 break;
             case SortingAlgorithm::HeapSort:
                updateStatus(L"正在执行堆排序...");
                 heapSort();
                 break;
             case SortingAlgorithm::InsertionSort:
                updateStatus(L"正在执行插入排序...");
                 insertionSort();
                 break;
             case SortingAlgorithm::SelectionSort:
                updateStatus(L"正在执行选择排序...");
                 selectionSort();
                 break;
         }

         double timeElapsed = stopTimer();
         isSorting = false;
         clearHighlights();
         if (hwndCanvas) InvalidateRect(hwndCanvas, nullptr, FALSE);
         else InvalidateRect(hwnd, nullptr, FALSE);

         // 显示排序完成信息
         wchar_t msg[256];
         swprintf_s(msg, L"排序完成，耗时: %.2f ms，比较: %zu 次，交换: %zu 次",
                    timeElapsed, comparisonCount, swapCount);
        updateStatus(msg);

         // 恢复暂停按钮文本
        if (hwnd) PostMessageW(hwnd, WM_APP_SET_PAUSE_TEXT, 1, 0);
     });

     sortingThread.detach();
}

void WinGUIVisualizer::pauseResume() {
    isPaused = !isPaused;
}

void WinGUIVisualizer::stopSorting() {
    isSorting = false;
}

void WinGUIVisualizer::setAnimationSpeed(int speed) {
    animationDelay = 101 - speed; // 反向映射，速度越高延迟越低
    if (animationDelay < 1) animationDelay = 1;
    if (animationDelay > 100) animationDelay = 100;
}

void WinGUIVisualizer::updateStatus(const std::wstring& text) {
    statusText = text;
    if (!hwnd) return;
    // If caller is UI thread for this window, set directly, otherwise post to UI thread
    DWORD uiThreadId = GetWindowThreadProcessId(hwnd, nullptr);
    if (uiThreadId == GetCurrentThreadId()) {
        if (hwndStaticStatus) SetWindowTextW(hwndStaticStatus, text.c_str());
    } else {
        // duplicate string and post to window to update on UI thread
        size_t len = text.size() + 1;
        wchar_t* dup = (wchar_t*)_wcsdup(text.c_str());
        if (dup) {
            PostMessageW(hwnd, WM_APP_UPDATE_STATUS, 0, (LPARAM)dup);
        }
    }
}

void WinGUIVisualizer::resetCounters() {
    comparisonCount = 0;
    swapCount = 0;
}

void WinGUIVisualizer::startTimer() {
    startTime = std::chrono::high_resolution_clock::now();
}

double WinGUIVisualizer::stopTimer() {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    return duration.count() / 1000.0; // 转换为毫秒
}

void WinGUIVisualizer::incrementComparisons() {
    comparisonCount++;
}

void WinGUIVisualizer::incrementSwaps() {
    swapCount++;
}

// 排序算法实现
void WinGUIVisualizer::bubbleSort() {
    int n;
    {
        std::lock_guard<std::mutex> lg(dataMutex);
        n = (int)data.size();
    }

    for (int i = 0; i < n - 1 && isSorting; i++) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1 && isSorting; j++) {
            while (isPaused) Sleep(100); // 暂停时等待
            
            incrementComparisons();
            highlightBars(j, j + 1);
            
            int a, b;
            {
                std::lock_guard<std::mutex> lg(dataMutex);
                a = data[j];
                b = data[j+1];
            }
            if (a > b) {
                swap(j, j + 1);
                swapped = true;
            }
            
            clearHighlights();
        }
        if (!swapped) break;
    }
}

void WinGUIVisualizer::quickSort() {
    int n;
    {
        std::lock_guard<std::mutex> lg(dataMutex);
        n = (int)data.size();
    }
    if (n > 0) quickSortHelper(0, n - 1);
}

void WinGUIVisualizer::quickSortHelper(int low, int high) {
    if (low < high && isSorting) {
        while (isPaused) Sleep(100);
        
        int pi = partition(low, high);
        quickSortHelper(low, pi - 1);
        quickSortHelper(pi + 1, high);
    }
}

int WinGUIVisualizer::partition(int low, int high) {
    int pivot;
    {
        std::lock_guard<std::mutex> lg(dataMutex);
        pivot = data[high];
    }
    int i = low - 1;
    
    for (int j = low; j <= high - 1 && isSorting; j++) {
        while (isPaused) Sleep(100);
        
        incrementComparisons();
        highlightBars(j, high);
        
        int valj;
        {
            std::lock_guard<std::mutex> lg(dataMutex);
            valj = data[j];
        }

        if (valj < pivot) {
            i++;
            swap(i, j);
        }
        
        clearHighlights();
    }
    swap(i + 1, high);
    return i + 1;
}

void WinGUIVisualizer::mergeSort() {
    int n;
    {
        std::lock_guard<std::mutex> lg(dataMutex);
        n = (int)data.size();
    }
    if (n > 0) mergeSortHelper(0, n - 1);
}

void WinGUIVisualizer::mergeSortHelper(int left, int right) {
    if (left < right && isSorting) {
        while (isPaused) Sleep(100);
        
        int mid = left + (right - left) / 2;
        mergeSortHelper(left, mid);
        mergeSortHelper(mid + 1, right);
        merge(left, mid, right);
    }
}

void WinGUIVisualizer::merge(int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    std::vector<int> leftArray(n1);
    std::vector<int> rightArray(n2);
    
    {
        std::lock_guard<std::mutex> lg(dataMutex);
        for (int i = 0; i < n1; i++)
            leftArray[i] = data[left + i];
        for (int j = 0; j < n2; j++)
            rightArray[j] = data[mid + 1 + j];
    }

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2 && isSorting) {
        while (isPaused) Sleep(100);
        
        incrementComparisons();
        highlightBars(left + i, mid + 1 + j);
        
        int writeVal;
        if (leftArray[i] <= rightArray[j]) {
            writeVal = leftArray[i];
            i++;
        } else {
            writeVal = rightArray[j];
            j++;
        }
        {
            std::lock_guard<std::mutex> lg(dataMutex);
            data[k] = writeVal;
        }
        k++;
        
        if (hwndCanvas) InvalidateRect(hwndCanvas, nullptr, FALSE); // 不清除背景以减少闪烁
        else InvalidateRect(hwnd, nullptr, FALSE);
        sleepAnimation();
        clearHighlights();
    }
    
    while (i < n1 && isSorting) {
        while (isPaused) Sleep(100);
        {
            std::lock_guard<std::mutex> lg(dataMutex);
            data[k] = leftArray[i];
        }
        i++; k++;
        if (hwndCanvas) InvalidateRect(hwndCanvas, nullptr, FALSE);
        else InvalidateRect(hwnd, nullptr, FALSE);
        sleepAnimation();
    }
    
    while (j < n2 && isSorting) {
        while (isPaused) Sleep(100);
        {
            std::lock_guard<std::mutex> lg(dataMutex);
            data[k] = rightArray[j];
        }
        j++; k++;
        if (hwndCanvas) InvalidateRect(hwndCanvas, nullptr, FALSE);
        else InvalidateRect(hwnd, nullptr, FALSE);
        sleepAnimation();
    }
}

void WinGUIVisualizer::heapSort() {
    int n;
    {
        std::lock_guard<std::mutex> lg(dataMutex);
        n = (int)data.size();
    }

    for (int i = n / 2 - 1; i >= 0 && isSorting; i--) {
        while (isPaused) Sleep(100);
        heapify(n, i);
    }
    
    for (int i = n - 1; i > 0 && isSorting; i--) {
        while (isPaused) Sleep(100);
        swap(0, i);
        heapify(i, 0);
    }
}

void WinGUIVisualizer::heapify(int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    if (left < n && isSorting) {
        while (isPaused) Sleep(100);
        incrementComparisons();
        int leftVal;
        int largestVal;
        {
            std::lock_guard<std::mutex> lg(dataMutex);
            leftVal = data[left];
            largestVal = data[largest];
        }
        if (leftVal > largestVal) {
            largest = left;
        }
    }
    
    if (right < n && isSorting) {
        while (isPaused) Sleep(100);
        incrementComparisons();
        int rightVal;
        int largestVal;
        {
            std::lock_guard<std::mutex> lg(dataMutex);
            rightVal = data[right];
            largestVal = data[largest];
        }
        if (rightVal > largestVal) {
            largest = right;
        }
    }
    
    if (largest != i && isSorting) {
        while (isPaused) Sleep(100);
        swap(i, largest);
        heapify(n, largest);
    }
}

void WinGUIVisualizer::insertionSort() {
    int n;
    {
        std::lock_guard<std::mutex> lg(dataMutex);
        n = (int)data.size();
    }

    for (int i = 1; i < n && isSorting; i++) {
        while (isPaused) Sleep(100);
        
        int key;
        {
            std::lock_guard<std::mutex> lg(dataMutex);
            key = data[i];
        }
        int j = i - 1;
        
        highlightBars(i, -1); // 只高亮当前元素
        
        while (j >= 0 && isSorting) {
            while (isPaused) Sleep(100);
            int dj;
            {
                std::lock_guard<std::mutex> lg(dataMutex);
                dj = data[j];
            }
            incrementComparisons();
            if (dj > key) {
                highlightBars(j, j + 1);
                {
                    std::lock_guard<std::mutex> lg(dataMutex);
                    data[j + 1] = data[j];
                }
                j--;
                if (hwndCanvas) InvalidateRect(hwndCanvas, nullptr, FALSE);
                else InvalidateRect(hwnd, nullptr, FALSE);
                sleepAnimation();
                clearHighlights();
            } else {
                break;
            }
        }
        {
            std::lock_guard<std::mutex> lg(dataMutex);
            data[j + 1] = key;
        }
        if (hwndCanvas) InvalidateRect(hwndCanvas, nullptr, FALSE);
        else InvalidateRect(hwnd, nullptr, FALSE);
        sleepAnimation();
    }
}

void WinGUIVisualizer::selectionSort() {
    int n;
    {
        std::lock_guard<std::mutex> lg(dataMutex);
        n = (int)data.size();
    }

    for (int i = 0; i < n - 1 && isSorting; i++) {
        while (isPaused) Sleep(100);
        
        int minIndex = i;
        highlightBars(i, -1); // 高亮当前位置
        
        for (int j = i + 1; j < n && isSorting; j++) {
            while (isPaused) Sleep(100);
            
            incrementComparisons();
            highlightBars(minIndex, j);
            
            int valj, valmin;
            {
                std::lock_guard<std::mutex> lg(dataMutex);
                valj = data[j];
                valmin = data[minIndex];
            }

            if (valj < valmin) {
                clearHighlights();
                minIndex = j;
                highlightBars(minIndex, -1);
            }
            
            sleepAnimation();
            clearHighlights();
        }
        
        if (minIndex != i) {
            swap(i, minIndex);
        }
    }
}

void WinGUIVisualizer::runPerformanceComparison() {
    // 在单独的线程中运行性能比较，避免阻塞UI
    std::thread compareThread([this]() {
        updateStatus(L"正在进行性能比较测试...");
        
        performanceResults.clear();
        
        // 保存当前数据
        std::vector<int> savedData;
        {
            std::lock_guard<std::mutex> lg(dataMutex);
            savedData = data;
        }

        // 测试各种排序算法
        std::vector<std::pair<SortingAlgorithm, std::wstring>> algorithms = {
            {SortingAlgorithm::BubbleSort, L"冒泡排序"},
            {SortingAlgorithm::InsertionSort, L"插入排序"},
            {SortingAlgorithm::SelectionSort, L"选择排序"},
            {SortingAlgorithm::QuickSort, L"快速排序"},
            {SortingAlgorithm::MergeSort, L"归并排序"},
            {SortingAlgorithm::HeapSort, L"堆排序"}
        };
        
        for (const auto& alg : algorithms) {
            // 恢复原始数据
            {
                std::lock_guard<std::mutex> lg(dataMutex);
                data = originalData;
            }

            // 创建性能结果对象
            PerformanceResult result;
            result.algorithm = alg.first;
            result.name = alg.second;
            
            // 执行排序并测量性能
            resetCounters();
            startTimer();
            
            switch (alg.first) {
                case SortingAlgorithm::BubbleSort:
                    bubbleSort();
                    break;
                case SortingAlgorithm::QuickSort:
                    quickSort();
                    break;
                case SortingAlgorithm::MergeSort:
                    mergeSort();
                    break;
                case SortingAlgorithm::HeapSort:
                    heapSort();
                    break;
                case SortingAlgorithm::InsertionSort:
                    insertionSort();
                    break;
                case SortingAlgorithm::SelectionSort:
                    selectionSort();
                    break;
            }
            
            result.timeMs = stopTimer();
            result.comparisons = comparisonCount;
            result.swaps = swapCount;
            result.isStable = (alg.first == SortingAlgorithm::BubbleSort || 
                              alg.first == SortingAlgorithm::MergeSort || 
                              alg.first == SortingAlgorithm::InsertionSort);
            
            performanceResults.push_back(result);
        }
        
        // 恢复数据
        {
            std::lock_guard<std::mutex> lg(dataMutex);
            data = savedData;
        }

        // 显示结果在新的窗口中
        std::wstring resultText = L"性能比较结果:\n\n";
        resultText += L"算法名称\t\t耗时(ms)\t比较次数\t交换次数\t稳定性\n";
        resultText += L"----------------------------------------------------------------\n";
        
        for (const auto& result : performanceResults) {
            wchar_t line[256];
            swprintf_s(line, L"%-12s\t\t%8.2f\t%8zu\t%8zu\t%s\n",
                      result.name.c_str(),
                      result.timeMs,
                      result.comparisons,
                      result.swaps,
                      result.isStable ? L"稳定" : L"不稳定");
            resultText += line;
        }
        
        // 显示消息框
        MessageBoxW(hwnd, resultText.c_str(), L"性能比较结果", MB_OK | MB_ICONINFORMATION);
        updateStatus(L"性能比较测试完成");
    });
    
    compareThread.detach();
}

#endif // _WIN32

