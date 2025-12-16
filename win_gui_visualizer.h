#ifndef WIN_GUI_VISUALIZER_H
#define WIN_GUI_VISUALIZER_H

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <cstdlib>
#include <cstring>
#include <tchar.h>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <mutex>
#include <atomic>

// 数据生成类型枚举
enum class DataPattern {
    Random,
    Ascending,
    Descending,
    PartiallySorted
};

// 排序算法枚举
enum class SortingAlgorithm {
    BubbleSort,
    QuickSort,
    MergeSort,
    HeapSort,
    InsertionSort,
    SelectionSort
};

// 性能测试结果结构
struct PerformanceResult {
    SortingAlgorithm algorithm;
    std::wstring name;
    double timeMs;
    size_t comparisons;
    size_t swaps;
    bool isStable;
};

class WinGUIVisualizer {
private:
    HWND hwnd;
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;
    
    // 控件句柄
    HWND hwndBtnGenerateRandom;
    HWND hwndBtnGenerateAsc;
    HWND hwndBtnGenerateDesc;
    HWND hwndBtnGeneratePartial;
    HWND hwndBtnReset;
    HWND hwndBtnBubbleSort;
    HWND hwndBtnQuickSort;
    HWND hwndBtnMergeSort;
    HWND hwndBtnHeapSort;
    HWND hwndBtnInsertionSort;
    HWND hwndBtnSelectionSort;
    HWND hwndBtnCompareAll;
    HWND hwndBtnPauseResume;
    HWND hwndTrackbarSpeed;
    HWND hwndEditDataSize;
    HWND hwndStaticStatus;
    HWND hwndCanvas;

    // 窗口尺寸
    static const int WINDOW_WIDTH = 1200;
    static const int WINDOW_HEIGHT = 800;
    
    // 数据和可视化相关
    std::vector<int> data;
    std::vector<int> originalData;
    std::vector<PerformanceResult> performanceResults;
    int barWidth;
    int maxDataValue;
    std::mutex dataMutex; // 保护 data / originalData / related members

    // 排序状态
    bool isSorting;
    bool isPaused;
    SortingAlgorithm currentAlgorithm;
    int animationDelay; // 毫秒
    std::wstring statusText;
    
    // 当前高亮的元素 (使用原子以便在线程间读取)
    std::atomic<int> highlightIndex1;
    std::atomic<int> highlightIndex2;

    // 性能统计
    size_t comparisonCount;
    size_t swapCount;
    std::chrono::high_resolution_clock::time_point startTime;

    // 控件字体
    HFONT controlFont;
    HFONT titleFont;

public:
    WinGUIVisualizer();
    
    // 窗口初始化
    bool initializeWindow(HINSTANCE hInstance, int nCmdShow);
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK CanvasProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void createControls();
    void layoutControls();
    
    // 数据管理
    void generateData(size_t size, DataPattern pattern);
    void resetData();
    void setData(const std::vector<int>& newData);
    
    // 排序算法（带可视化）
    void bubbleSort();
    void quickSort();
    void mergeSort();
    void heapSort();
    void insertionSort();
    void selectionSort();
    
    // 排序辅助函数
    void quickSortHelper(int low, int high);
    int partition(int low, int high);
    void merge(int left, int mid, int right);
    void mergeSortHelper(int left, int right);
    void heapify(int n, int i);
    
    // 可视化相关
    void drawVisualization();
    void drawBars();
    void drawUI();
    void highlightBars(int index1, int index2);
    void clearHighlights();
    void updateStatus(const std::wstring& text);
    void refreshAll(); // 声明全局刷新方法，解决无法解析符号

    // 控制函数
    void startSorting(SortingAlgorithm algorithm);
    void pauseResume();
    void stopSorting();
    void setAnimationSpeed(int speed); // 1-100, 100最快
    void runPerformanceComparison();
    
    // 工具函数
    static std::vector<int> generateTestData(size_t size, DataPattern pattern);
    
private:
    void swap(int i, int j);
    void sleepAnimation();
    void resetCounters();
    void startTimer();
    double stopTimer();
    void incrementComparisons();
    void incrementSwaps();
};

#endif // _WIN32

#endif // WIN_GUI_VISUALIZER_H







