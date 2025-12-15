#ifndef SORTING_SYSTEM_H
#define SORTING_SYSTEM_H

#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <random>
#include <iostream>

// 排序算法性能比较结果结构体
struct SortPerformance {
    std::string algorithmName;
    double timeTaken; // 毫秒
    size_t comparisons;
    size_t swaps;
    bool stable;

    SortPerformance(const std::string& name, double time, size_t comp, size_t sw, bool st) 
        : algorithmName(name), timeTaken(time), comparisons(comp), swaps(sw), stable(st) {}
};

// 数据生成类型枚举
enum class DataPattern {
    Random,
    Ascending,
    Descending,
    PartiallySorted
};

class SortingSystem {
private:
    std::vector<int> data;
    std::vector<int> originalData;
    
    // 性能统计变量
    mutable size_t comparisonCount;
    mutable size_t swapCount;

    // 辅助函数用于性能统计
    void resetCounters() const;
    void incrementComparisons() const { comparisonCount++; }
    void incrementSwaps() const { swapCount++; }

public:
    // 构造和数据管理
    SortingSystem();
    void generateData(size_t size, DataPattern pattern);
    void setData(const std::vector<int>& newData);
    const std::vector<int>& getData() const { return data; }
    const std::vector<int>& getOriginalData() const { return originalData; }
    void resetData();

    // 排序算法实现
    void bubbleSort();
    void quickSort();
    void mergeSort();
    void heapSort();
    void insertionSort();
    void selectionSort();

    // 排序算法辅助函数
    void quickSortHelper(int low, int high);
    int partition(int low, int high);
    void merge(int left, int mid, int right);
    void mergeSortHelper(int left, int right);
    void heapify(int n, int i);
    
    // 性能测试
    SortPerformance testAlgorithm(const std::string& algorithmName, void (SortingSystem::*sortFunc)());

    // 工具函数
    bool isSorted() const;
    void printData() const;
    static std::vector<int> generateTestData(size_t size, DataPattern pattern);

private:
    // 排序过程中的交换操作（用于统计）
    void swap(int& a, int& b);
};

#endif // SORTING_SYSTEM_H