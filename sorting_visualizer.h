#ifndef SORTING_VISUALIZER_H
#define SORTING_VISUALIZER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>

// 数据生成类型枚举
enum class DataPattern {
    Random,
    Ascending,
    Descending,
    PartiallySorted
};

class SortingVisualizer {
private:
    sf::RenderWindow window;
    std::vector<int> data;
    std::vector<sf::RectangleShape> bars;
    
    // 窗口尺寸
    const unsigned int WINDOW_WIDTH = 1200;
    const unsigned int WINDOW_HEIGHT = 800;
    
    // 排序过程控制
    bool isSorting;
    bool isPaused;
    int currentStep;
    int comparisonCount;
    int swapCount;
    
    // 动画速度控制（毫秒）
    int animationSpeed;

public:
    SortingVisualizer();
    
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
    void initializeBars();
    void updateBars();
    void draw();
    void renderBars();
    void highlightBars(int index1, int index2, sf::Color color);
    void highlightSingleBar(int index, sf::Color color);
    
    // 动画控制
    void animate();
    void pauseResume();
    void stepForward();
    void stepBackward();
    void setAnimationSpeed(int speed); // 1-100, 100最快
    
    // 工具函数
    static std::vector<int> generateTestData(size_t size, DataPattern pattern);
    
private:
    void swap(int i, int j);
    void processEvents();
    void clearHighlight();
};

#endif // SORTING_VISUALIZER_H