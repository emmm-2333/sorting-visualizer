#include "sorting_system.h"
#include <iostream>
#include <iomanip>

SortingSystem::SortingSystem() : comparisonCount(0), swapCount(0) {}

void SortingSystem::resetCounters() const {
    comparisonCount = 0;
    swapCount = 0;
}

void SortingSystem::generateData(size_t size, DataPattern pattern) {
    data = generateTestData(size, pattern);
    originalData = data;
}

void SortingSystem::setData(const std::vector<int>& newData) {
    data = newData;
    originalData = newData;
}

void SortingSystem::resetData() {
    data = originalData;
}

void SortingSystem::swap(int& a, int& b) {
    if (&a != &b) {
        int temp = a;
        a = b;
        b = temp;
        swapCount++;
    }
}

// 冒泡排序实现
void SortingSystem::bubbleSort() {
    resetCounters();
    int n = data.size();
    bool swapped;
    
    for (int i = 0; i < n - 1; i++) {
        swapped = false;
        for (int j = 0; j < n - i - 1; j++) {
            incrementComparisons();
            if (data[j] > data[j + 1]) {
                swap(data[j], data[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

// 快速排序实现
void SortingSystem::quickSort() {
    resetCounters();
    quickSortHelper(0, data.size() - 1);
}

void SortingSystem::quickSortHelper(int low, int high) {
    if (low < high) {
        int pi = partition(low, high);
        quickSortHelper(low, pi - 1);
        quickSortHelper(pi + 1, high);
    }
}

int SortingSystem::partition(int low, int high) {
    int pivot = data[high];
    int i = low - 1;

    for (int j = low; j <= high - 1; j++) {
        incrementComparisons();
        if (data[j] < pivot) {
            i++;
            swap(data[i], data[j]);
        }
    }
    swap(data[i + 1], data[high]);
    return i + 1;
}

// 归并排序实现
void SortingSystem::mergeSort() {
    resetCounters();
    mergeSortHelper(0, data.size() - 1);
}

void SortingSystem::mergeSortHelper(int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortHelper(left, mid);
        mergeSortHelper(mid + 1, right);
        merge(left, mid, right);
    }
}

void SortingSystem::merge(int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<int> leftArray(n1);
    std::vector<int> rightArray(n2);

    for (int i = 0; i < n1; i++)
        leftArray[i] = data[left + i];
    for (int j = 0; j < n2; j++)
        rightArray[j] = data[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        incrementComparisons();
        if (leftArray[i] <= rightArray[j]) {
            data[k] = leftArray[i];
            i++;
        } else {
            data[k] = rightArray[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        data[k] = leftArray[i];
        i++;
        k++;
    }

    while (j < n2) {
        data[k] = rightArray[j];
        j++;
        k++;
    }
}

// 堆排序实现
void SortingSystem::heapSort() {
    resetCounters();
    int n = data.size();

    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(n, i);

    for (int i = n - 1; i > 0; i--) {
        swap(data[0], data[i]);
        heapify(i, 0);
    }
}

void SortingSystem::heapify(int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n) {
        incrementComparisons();
        if (data[left] > data[largest])
            largest = left;
    }

    if (right < n) {
        incrementComparisons();
        if (data[right] > data[largest])
            largest = right;
    }

    if (largest != i) {
        swap(data[i], data[largest]);
        heapify(n, largest);
    }
}

// 插入排序实现
void SortingSystem::insertionSort() {
    resetCounters();
    int n = data.size();
    
    for (int i = 1; i < n; i++) {
        int key = data[i];
        int j = i - 1;
        
        while (j >= 0) {
            incrementComparisons();
            if (data[j] > key) {
                data[j + 1] = data[j];
                j--;
                swapCount++; // 记录移动次数
            } else {
                break;
            }
        }
        data[j + 1] = key;
    }
}

// 选择排序实现
void SortingSystem::selectionSort() {
    resetCounters();
    int n = data.size();
    
    for (int i = 0; i < n - 1; i++) {
        int minIndex = i;
        for (int j = i + 1; j < n; j++) {
            incrementComparisons();
            if (data[j] < data[minIndex]) {
                minIndex = j;
            }
        }
        if (minIndex != i) {
            swap(data[i], data[minIndex]);
        }
    }
}

SortPerformance SortingSystem::testAlgorithm(const std::string& algorithmName, void (SortingSystem::*sortFunc)()) {
    resetData();
    auto start = std::chrono::high_resolution_clock::now();
    (this->*sortFunc)();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double timeInMs = duration.count() / 1000.0;
    
    // 简单判断稳定性（对于整数来说较难体现，这里仅作示例）
    bool isStable = (algorithmName == "Merge Sort" || algorithmName == "Bubble Sort" || algorithmName == "Insertion Sort");
    
    return SortPerformance(algorithmName, timeInMs, comparisonCount, swapCount, isStable);
}

bool SortingSystem::isSorted() const {
    for (size_t i = 1; i < data.size(); i++) {
        if (data[i] < data[i-1]) {
            return false;
        }
    }
    return true;
}

void SortingSystem::printData() const {
    for (size_t i = 0; i < data.size(); i++) {
        std::cout << std::setw(4) << data[i] << " ";
        if ((i + 1) % 10 == 0) std::cout << std::endl;
    }
    std::cout << std::endl;
}

std::vector<int> SortingSystem::generateTestData(size_t size, DataPattern pattern) {
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