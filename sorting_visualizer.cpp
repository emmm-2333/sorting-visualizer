#include "sorting_visualizer.h"
#include <iostream>
#include <algorithm>
#include <random>

SortingVisualizer::SortingVisualizer() 
    : window(sf::VideoMode(1200, 800), "Sorting Algorithm Visualizer"),
      isSorting(false),
      isPaused(false),
      currentStep(0),
      comparisonCount(0),
      swapCount(0),
      animationSpeed(50) {
    window.setFramerateLimit(60);
}

void SortingVisualizer::generateData(size_t size, DataPattern pattern) {
    data = generateTestData(size, pattern);
    initializeBars();
}

std::vector<int> SortingVisualizer::generateTestData(size_t size, DataPattern pattern) {
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

void SortingVisualizer::initializeBars() {
    bars.clear();
    bars.reserve(data.size());
    
    float barWidth = (float)WINDOW_WIDTH / data.size();
    float maxHeight = WINDOW_HEIGHT - 100; // 为顶部留出空间显示信息
    
    for (size_t i = 0; i < data.size(); i++) {
        float barHeight = (float)data[i] / data.size() * maxHeight;
        sf::RectangleShape bar(sf::Vector2f(barWidth - 1, barHeight));
        
        // 设置位置（左下角为基准）
        bar.setPosition(i * barWidth, WINDOW_HEIGHT - barHeight);
        bar.setFillColor(sf::Color::White);
        
        bars.push_back(bar);
    }
}

void SortingVisualizer::updateBars() {
    float barWidth = (float)WINDOW_WIDTH / data.size();
    float maxHeight = WINDOW_HEIGHT - 100;
    
    for (size_t i = 0; i < data.size(); i++) {
        float barHeight = (float)data[i] / data.size() * maxHeight;
        bars[i].setSize(sf::Vector2f(barWidth - 1, barHeight));
        bars[i].setPosition(i * barWidth, WINDOW_HEIGHT - barHeight);
    }
}

void SortingVisualizer::draw() {
    window.clear(sf::Color::Black);
    
    // 绘制柱状图
    renderBars();
    
    // 绘制信息文本
    // 注意：为了简化，这里没有实际绘制文本，但在完整实现中会添加
    
    window.display();
}

void SortingVisualizer::renderBars() {
    for (const auto& bar : bars) {
        window.draw(bar);
    }
}

void SortingVisualizer::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Space:
                    pauseResume();
                    break;
                case sf::Keyboard::Right:
                    stepForward();
                    break;
                case sf::Keyboard::Left:
                    stepBackward();
                    break;
                case sf::Keyboard::Up:
                    setAnimationSpeed(std::min(100, animationSpeed + 10));
                    break;
                case sf::Keyboard::Down:
                    setAnimationSpeed(std::max(1, animationSpeed - 10));
                    break;
                case sf::Keyboard::R:
                    resetData();
                    break;
            }
        }
    }
}

void SortingVisualizer::animate() {
    while (window.isOpen()) {
        processEvents();
        draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
    }
}

void SortingVisualizer::pauseResume() {
    isPaused = !isPaused;
}

void SortingVisualizer::stepForward() {
    // 在手动模式下执行下一步
}

void SortingVisualizer::stepBackward() {
    // 回退一步（需要保存历史状态）
}

void SortingVisualizer::setAnimationSpeed(int speed) {
    animationSpeed = std::max(1, std::min(100, speed));
}

void SortingVisualizer::resetData() {
    // 重新随机排列数据
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(data.begin(), data.end(), gen);
    updateBars();
}

void SortingVisualizer::highlightBars(int index1, int index2, sf::Color color) {
    if (index1 >= 0 && index1 < (int)bars.size()) {
        bars[index1].setFillColor(color);
    }
    if (index2 >= 0 && index2 < (int)bars.size()) {
        bars[index2].setFillColor(color);
    }
}

void SortingVisualizer::highlightSingleBar(int index, sf::Color color) {
    if (index >= 0 && index < (int)bars.size()) {
        bars[index].setFillColor(color);
    }
}

void SortingVisualizer::clearHighlight() {
    for (auto& bar : bars) {
        bar.setFillColor(sf::Color::White);
    }
}

void SortingVisualizer::swap(int i, int j) {
    if (i != j) {
        std::swap(data[i], data[j]);
        // 更新对应的柱状图
        updateBars();
    }
}

// 排序算法实现
void SortingVisualizer::bubbleSort() {
    isSorting = true;
    int n = data.size();
    
    for (int i = 0; i < n - 1 && isSorting && !isPaused; i++) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1 && isSorting && !isPaused; j++) {
            // 高亮正在比较的元素
            highlightBars(j, j + 1, sf::Color::Red);
            draw();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
            
            if (data[j] > data[j + 1]) {
                swap(j, j + 1);
                swapped = true;
                
                // 显示交换后的结果
                highlightBars(j, j + 1, sf::Color::Green);
                draw();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
            }
            
            clearHighlight();
        }
        if (!swapped) break;
    }
    isSorting = false;
}

void SortingVisualizer::quickSort() {
    isSorting = true;
    quickSortHelper(0, data.size() - 1);
    isSorting = false;
}

void SortingVisualizer::quickSortHelper(int low, int high) {
    if (low < high && isSorting && !isPaused) {
        int pi = partition(low, high);
        quickSortHelper(low, pi - 1);
        quickSortHelper(pi + 1, high);
    }
}

int SortingVisualizer::partition(int low, int high) {
    int pivot = data[high];
    int i = low - 1;
    
    for (int j = low; j <= high - 1 && isSorting && !isPaused; j++) {
        // 高亮正在比较的元素
        highlightBars(j, high, sf::Color::Red);
        draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
        
        if (data[j] < pivot) {
            i++;
            swap(i, j);
            
            // 显示交换后的结果
            highlightBars(i, j, sf::Color::Green);
            draw();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
        }
        clearHighlight();
    }
    swap(i + 1, high);
    return i + 1;
}

void SortingVisualizer::mergeSort() {
    isSorting = true;
    mergeSortHelper(0, data.size() - 1);
    isSorting = false;
}

void SortingVisualizer::mergeSortHelper(int left, int right) {
    if (left < right && isSorting && !isPaused) {
        int mid = left + (right - left) / 2;
        mergeSortHelper(left, mid);
        mergeSortHelper(mid + 1, right);
        merge(left, mid, right);
    }
}

void SortingVisualizer::merge(int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    std::vector<int> leftArray(n1);
    std::vector<int> rightArray(n2);
    
    for (int i = 0; i < n1; i++)
        leftArray[i] = data[left + i];
    for (int j = 0; j < n2; j++)
        rightArray[j] = data[mid + 1 + j];
    
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2 && isSorting && !isPaused) {
        // 高亮正在比较的元素
        highlightBars(left + i, mid + 1 + j, sf::Color::Red);
        draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
        
        if (leftArray[i] <= rightArray[j]) {
            data[k] = leftArray[i];
            i++;
        } else {
            data[k] = rightArray[j];
            j++;
        }
        updateBars();
        k++;
        
        // 显示合并后的结果
        highlightSingleBar(k - 1, sf::Color::Green);
        draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
        clearHighlight();
    }
    
    while (i < n1 && isSorting && !isPaused) {
        data[k] = leftArray[i];
        updateBars();
        i++;
        k++;
    }
    
    while (j < n2 && isSorting && !isPaused) {
        data[k] = rightArray[j];
        updateBars();
        j++;
        k++;
    }
}

void SortingVisualizer::heapSort() {
    isSorting = true;
    int n = data.size();
    
    for (int i = n / 2 - 1; i >= 0 && isSorting && !isPaused; i--) {
        heapify(n, i);
    }
    
    for (int i = n - 1; i > 0 && isSorting && !isPaused; i--) {
        swap(0, i);
        heapify(i, 0);
    }
    isSorting = false;
}

void SortingVisualizer::heapify(int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    if (left < n && isSorting && !isPaused) {
        if (data[left] > data[largest]) {
            largest = left;
        }
    }
    
    if (right < n && isSorting && !isPaused) {
        if (data[right] > data[largest]) {
            largest = right;
        }
    }
    
    if (largest != i && isSorting && !isPaused) {
        swap(i, largest);
        heapify(n, largest);
    }
}

void SortingVisualizer::insertionSort() {
    isSorting = true;
    int n = data.size();
    
    for (int i = 1; i < n && isSorting && !isPaused; i++) {
        int key = data[i];
        int j = i - 1;
        
        // 高亮当前元素
        highlightSingleBar(i, sf::Color::Yellow);
        draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
        
        while (j >= 0 && data[j] > key && isSorting && !isPaused) {
            // 高亮正在比较的元素
            highlightBars(j, j + 1, sf::Color::Red);
            draw();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
            
            data[j + 1] = data[j];
            j--;
            updateBars();
            
            // 显示移动后的结果
            clearHighlight();
            highlightSingleBar(j + 1, sf::Color::Green);
            draw();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
            clearHighlight();
        }
        data[j + 1] = key;
        updateBars();
    }
    isSorting = false;
}

void SortingVisualizer::selectionSort() {
    isSorting = true;
    int n = data.size();
    
    for (int i = 0; i < n - 1 && isSorting && !isPaused; i++) {
        int minIndex = i;
        
        // 高亮当前元素
        highlightSingleBar(i, sf::Color::Yellow);
        draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
        
        for (int j = i + 1; j < n && isSorting && !isPaused; j++) {
            // 高亮正在比较的元素
            highlightBars(minIndex, j, sf::Color::Red);
            draw();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
            
            if (data[j] < data[minIndex]) {
                clearHighlight();
                minIndex = j;
                // 高亮新的最小值
                highlightSingleBar(minIndex, sf::Color::Blue);
                draw();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
            }
            clearHighlight();
        }
        
        if (minIndex != i) {
            swap(i, minIndex);
            
            // 显示交换后的结果
            highlightBars(i, minIndex, sf::Color::Green);
            draw();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 - (animationSpeed * 9)));
        }
        clearHighlight();
    }
    isSorting = false;
}