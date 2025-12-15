#include "win_gui_visualizer.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <thread>
#include <chrono>

#ifdef _WIN32

WinGUIVisualizer::WinGUIVisualizer() 
    : hwnd(nullptr),
      isSorting(false),
      isPaused(false),
      currentAlgorithm(0),
      animationDelay(50),
      highlightIndex1(-1),
      highlightIndex2(-1),
      maxDataValue(0) {
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
    
    // 创建窗口
    hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"排序算法可视化演示系统",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr,
        nullptr,
        hInstance,
        this // 将this指针传递给窗口
    );
    
    if (hwnd == nullptr) {
        return false;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // 生成默认数据
    generateData(50, DataPattern::Random);
    
    return true;
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
            case WM_PAINT:
                visualizer->drawVisualization();
                break;
                
            case WM_KEYDOWN:
                switch (wParam) {
                    case VK_SPACE:
                        visualizer->pauseResume();
                        break;
                    case VK_UP:
                        visualizer->setAnimationSpeed(std::min(100, visualizer->animationDelay + 10));
                        break;
                    case VK_DOWN:
                        visualizer->setAnimationSpeed(std::max(1, visualizer->animationDelay - 10));
                        break;
                    case '1':
                        visualizer->startSorting(SortingAlgorithm::BubbleSort);
                        break;
                    case '2':
                        visualizer->startSorting(SortingAlgorithm::QuickSort);
                        break;
                    case '3':
                        visualizer->startSorting(SortingAlgorithm::MergeSort);
                        break;
                    case '4':
                        visualizer->startSorting(SortingAlgorithm::HeapSort);
                        break;
                    case '5':
                        visualizer->startSorting(SortingAlgorithm::InsertionSort);
                        break;
                    case '6':
                        visualizer->startSorting(SortingAlgorithm::SelectionSort);
                        break;
                    case 'R':
                    case 'r':
                        visualizer->resetData();
                        InvalidateRect(hwnd, nullptr, TRUE);
                        break;
                }
                break;
                
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
    }
    
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void WinGUIVisualizer::generateData(size_t size, DataPattern pattern) {
    data = generateTestData(size, pattern);
    
    // 计算最大值用于缩放
    maxDataValue = *std::max_element(data.begin(), data.end());
    
    // 计算柱形宽度
    barWidth = (WINDOW_WIDTH - 40) / size;
    if (barWidth < 1) barWidth = 1;
    
    // 重绘窗口
    if (hwnd) {
        InvalidateRect(hwnd, nullptr, TRUE);
    }
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

void WinGUIVisualizer::drawVisualization() {
    hdc = BeginPaint(hwnd, &ps);
    
    // 设置字体支持中文
    HFONT hFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                              CHINESEBIG5_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"微软雅黑");
    SelectObject(hdc, hFont);
    
    // 填充背景
    RECT background = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &background, blackBrush);
    DeleteObject(blackBrush);
    
    // 绘制柱形图
    drawBars();
    
    // 绘制说明文字
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    
    wchar_t infoText[512];
    swprintf_s(infoText, L"排序算法可视化演示系统 - 按键: 1-6选择算法, 空格暂停, R重置, ↑/↓调速");
    TextOutW(hdc, 10, 10, infoText, (int)wcslen(infoText));
    
    DeleteObject(hFont);
    EndPaint(hwnd, &ps);
}

void WinGUIVisualizer::drawBars() {
    if (data.empty()) return;
    
    int baseY = WINDOW_HEIGHT - 50; // 底部边距
    int maxHeight = WINDOW_HEIGHT - 100; // 最大高度
    
    for (size_t i = 0; i < data.size(); i++) {
        int barHeight = (static_cast<long long>(data[i]) * maxHeight) / maxDataValue;
        int x = 20 + static_cast<int>(i) * barWidth;
        int y = baseY - barHeight;
        
        RECT barRect = {x, y, x + barWidth - 1, baseY};
        
        // 根据状态设置颜色
        HBRUSH brush;
        if (static_cast<int>(i) == highlightIndex1 || static_cast<int>(i) == highlightIndex2) {
            brush = CreateSolidBrush(RGB(255, 0, 0)); // 红色高亮
        } else {
            brush = CreateSolidBrush(RGB(0, 200, 255)); // 蓝色普通
        }
        
        FillRect(hdc, &barRect, brush);
        DeleteObject(brush);
    }
}

void WinGUIVisualizer::highlightBars(int index1, int index2) {
    highlightIndex1 = index1;
    highlightIndex2 = index2;
    InvalidateRect(hwnd, nullptr, TRUE);
    UpdateWindow(hwnd);
    sleepAnimation();
}

void WinGUIVisualizer::clearHighlights() {
    highlightIndex1 = -1;
    highlightIndex2 = -1;
}

void WinGUIVisualizer::swap(int i, int j) {
    if (i != j) {
        std::swap(data[i], data[j]);
        InvalidateRect(hwnd, nullptr, TRUE);
        UpdateWindow(hwnd);
        sleepAnimation();
    }
}

void WinGUIVisualizer::sleepAnimation() {
    Sleep(animationDelay);
}

void WinGUIVisualizer::resetData() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(data.begin(), data.end(), gen);
    clearHighlights();
}

void WinGUIVisualizer::startSorting(SortingAlgorithm algorithm) {
    // 在单独的线程中运行排序算法，避免阻塞UI
    std::thread sortingThread([this, algorithm]() {
        isSorting = true;
        isPaused = false;
        
        switch (algorithm) {
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
        
        isSorting = false;
        clearHighlights();
        InvalidateRect(hwnd, nullptr, TRUE);
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

// 排序算法实现
void WinGUIVisualizer::bubbleSort() {
    int n = data.size();
    
    for (int i = 0; i < n - 1 && isSorting; i++) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1 && isSorting; j++) {
            while (isPaused) Sleep(100); // 暂停时等待
            
            highlightBars(j, j + 1);
            
            if (data[j] > data[j + 1]) {
                swap(j, j + 1);
                swapped = true;
            }
            
            clearHighlights();
        }
        if (!swapped) break;
    }
}

void WinGUIVisualizer::quickSort() {
    quickSortHelper(0, data.size() - 1);
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
    int pivot = data[high];
    int i = low - 1;
    
    for (int j = low; j <= high - 1 && isSorting; j++) {
        while (isPaused) Sleep(100);
        
        highlightBars(j, high);
        
        if (data[j] < pivot) {
            i++;
            swap(i, j);
        }
        
        clearHighlights();
    }
    swap(i + 1, high);
    return i + 1;
}

void WinGUIVisualizer::mergeSort() {
    mergeSortHelper(0, data.size() - 1);
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
    
    for (int i = 0; i < n1; i++)
        leftArray[i] = data[left + i];
    for (int j = 0; j < n2; j++)
        rightArray[j] = data[mid + 1 + j];
    
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2 && isSorting) {
        while (isPaused) Sleep(100);
        
        highlightBars(left + i, mid + 1 + j);
        
        if (leftArray[i] <= rightArray[j]) {
            data[k] = leftArray[i];
            i++;
        } else {
            data[k] = rightArray[j];
            j++;
        }
        k++;
        
        InvalidateRect(hwnd, nullptr, TRUE);
        UpdateWindow(hwnd);
        sleepAnimation();
        clearHighlights();
    }
    
    while (i < n1 && isSorting) {
        while (isPaused) Sleep(100);
        data[k] = leftArray[i];
        i++;
        k++;
        InvalidateRect(hwnd, nullptr, TRUE);
        UpdateWindow(hwnd);
        sleepAnimation();
    }
    
    while (j < n2 && isSorting) {
        while (isPaused) Sleep(100);
        data[k] = rightArray[j];
        j++;
        k++;
        InvalidateRect(hwnd, nullptr, TRUE);
        UpdateWindow(hwnd);
        sleepAnimation();
    }
}

void WinGUIVisualizer::heapSort() {
    int n = data.size();
    
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
        if (data[left] > data[largest]) {
            largest = left;
        }
    }
    
    if (right < n && isSorting) {
        while (isPaused) Sleep(100);
        if (data[right] > data[largest]) {
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
    int n = data.size();
    
    for (int i = 1; i < n && isSorting; i++) {
        while (isPaused) Sleep(100);
        
        int key = data[i];
        int j = i - 1;
        
        highlightBars(i, -1); // 只高亮当前元素
        
        while (j >= 0 && data[j] > key && isSorting) {
            while (isPaused) Sleep(100);
            
            highlightBars(j, j + 1);
            data[j + 1] = data[j];
            j--;
            
            InvalidateRect(hwnd, nullptr, TRUE);
            UpdateWindow(hwnd);
            sleepAnimation();
            clearHighlights();
        }
        data[j + 1] = key;
        
        InvalidateRect(hwnd, nullptr, TRUE);
        UpdateWindow(hwnd);
        sleepAnimation();
    }
}

void WinGUIVisualizer::selectionSort() {
    int n = data.size();
    
    for (int i = 0; i < n - 1 && isSorting; i++) {
        while (isPaused) Sleep(100);
        
        int minIndex = i;
        highlightBars(i, -1); // 高亮当前位置
        
        for (int j = i + 1; j < n && isSorting; j++) {
            while (isPaused) Sleep(100);
            
            highlightBars(minIndex, j);
            
            if (data[j] < data[minIndex]) {
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

#endif // _WIN32