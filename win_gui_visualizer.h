#ifndef WIN_GUI_VISUALIZER_H
#define WIN_GUI_VISUALIZER_H

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <vector>
#include <algorithm>
#include <random>

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

class WinGUIVisualizer {
private:
    HWND hwnd;
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;
    
    // 窗口尺寸
    static const int WINDOW_WIDTH = 1000;
    static const int WINDOW_HEIGHT = 700;
    
    // 数据和可视化相关
    std::vector<int> data;
    int barWidth;
    int maxDataValue;
    
    // 排序状态
    bool isSorting;
    bool isPaused;
    int currentAlgorithm;
    int animationDelay; // 毫秒
    
    // 当前高亮的元素
    int highlightIndex1;
    int highlightIndex2;
    
public:
    WinGUIVisualizer();
    
    // 窗口初始化
    bool initializeWindow(HINSTANCE hInstance, int nCmdShow);
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    // 数据管理
    void generateData(size_t size, DataPattern pattern);
    void resetData();
    
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
    void highlightBars(int index1, int index2);
    void clearHighlights();
    
    // 控制函数
    void startSorting(SortingAlgorithm algorithm);
    void pauseResume();
    void stopSorting();
    void setAnimationSpeed(int speed); // 1-100, 100最快
    
    // 工具函数
    static std::vector<int> generateTestData(size_t size, DataPattern pattern);
    
private:
    void swap(int i, int j);
    void sleepAnimation();
};

#endif // _WIN32

#endif // WIN_GUI_VISUALIZER_H