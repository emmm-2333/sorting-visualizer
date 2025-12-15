#include <iostream>
#include <vector>
#include <iomanip>
#include <io.h>
#include <fcntl.h>
#include "sorting_system.h"

void setShowChinese() {
    // 设置控制台输出编码为UTF-8
    system("chcp 65001 > nul");
}

void showMenu() {
    std::cout << "\n========== 排序算法性能比较系统 ==========" << std::endl;
    std::cout << "1. 生成随机数据" << std::endl;
    std::cout << "2. 生成有序数据" << std::endl;
    std::cout << "3. 生成逆序数据" << std::endl;
    std::cout << "4. 生成部分有序数据" << std::endl;
    std::cout << "5. 手动输入数据" << std::endl;
    std::cout << "6. 显示当前数据" << std::endl;
    std::cout << "7. 运行性能比较" << std::endl;
    std::cout << "8. 动画演示排序过程" << std::endl;
    std::cout << "0. 退出程序" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "请选择操作: ";
}

void showSortingAlgorithms() {
    std::cout << "\n======= 可用排序算法 =======" << std::endl;
    std::cout << "1. 冒泡排序" << std::endl;
    std::cout << "2. 快速排序" << std::endl;
    std::cout << "3. 归并排序" << std::endl;
    std::cout << "4. 堆排序" << std::endl;
    std::cout << "5. 插入排序" << std::endl;
    std::cout << "6. 选择排序" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << "请选择算法: ";
}

void demonstrateSorting(SortingSystem& system, int choice) {
    std::cout << "\n原始数据:" << std::endl;
    system.printData();
    
    switch (choice) {
        case 1:
            std::cout << "\n冒泡排序过程演示:" << std::endl;
            system.bubbleSort();
            break;
        case 2:
            std::cout << "\n快速排序过程演示:" << std::endl;
            system.quickSort();
            break;
        case 3:
            std::cout << "\n归并排序过程演示:" << std::endl;
            system.mergeSort();
            break;
        case 4:
            std::cout << "\n堆排序过程演示:" << std::endl;
            system.heapSort();
            break;
        case 5:
            std::cout << "\n插入排序过程演示:" << std::endl;
            system.insertionSort();
            break;
        case 6:
            std::cout << "\n选择排序过程演示:" << std::endl;
            system.selectionSort();
            break;
    }
    
    std::cout << "\n排序后数据:" << std::endl;
    system.printData();
    std::cout << "排序结果验证: " << (system.isSorted() ? "正确" : "错误") << std::endl;
}

void runPerformanceTest(SortingSystem& system) {
    std::cout << "\n======= 性能比较测试 =======\n" << std::endl;
    std::cout << std::left << std::setw(15) << "算法名称" 
              << std::setw(12) << "耗时(ms)" 
              << std::setw(15) << "比较次数" 
              << std::setw(10) << "交换次数" 
              << std::setw(8) << "稳定性" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    // 测试各种排序算法
    std::vector<std::pair<std::string, void (SortingSystem::*)()>> algorithms = {
        {"Bubble Sort", &SortingSystem::bubbleSort},
        {"Insertion Sort", &SortingSystem::insertionSort},
        {"Selection Sort", &SortingSystem::selectionSort},
        {"Quick Sort", &SortingSystem::quickSort},
        {"Merge Sort", &SortingSystem::mergeSort},
        {"Heap Sort", &SortingSystem::heapSort}
    };

    for (const auto& alg : algorithms) {
        SortPerformance perf = system.testAlgorithm(alg.first, alg.second);
        std::cout << std::left << std::setw(15) << perf.algorithmName
                  << std::setw(12) << std::fixed << std::setprecision(3) << perf.timeTaken
                  << std::setw(15) << perf.comparisons
                  << std::setw(10) << perf.swaps
                  << std::setw(8) << (perf.stable ? "稳定" : "不稳定") << std::endl;
    }
}

int main() {
    // 设置控制台支持中文显示
    setShowChinese();
    
    SortingSystem system;
    int choice;
    size_t dataSize;

    std::cout << "欢迎使用排序算法性能比较与动画演示系统！" << std::endl;

    while (true) {
        showMenu();
        std::cin >> choice;

        switch (choice) {
            case 1: // 生成随机数据
                std::cout << "请输入数据量大小: ";
                std::cin >> dataSize;
                system.generateData(dataSize, DataPattern::Random);
                std::cout << "已生成包含 " << dataSize << " 个元素的随机数据集。" << std::endl;
                break;

            case 2: // 生成有序数据
                std::cout << "请输入数据量大小: ";
                std::cin >> dataSize;
                system.generateData(dataSize, DataPattern::Ascending);
                std::cout << "已生成包含 " << dataSize << " 个元素的有序数据集。" << std::endl;
                break;

            case 3: // 生成逆序数据
                std::cout << "请输入数据量大小: ";
                std::cin >> dataSize;
                system.generateData(dataSize, DataPattern::Descending);
                std::cout << "已生成包含 " << dataSize << " 个元素的逆序数据集。" << std::endl;
                break;

            case 4: // 生成部分有序数据
                std::cout << "请输入数据量大小: ";
                std::cin >> dataSize;
                system.generateData(dataSize, DataPattern::PartiallySorted);
                std::cout << "已生成包含 " << dataSize << " 个元素的部分有序数据集。" << std::endl;
                break;

            case 5: // 手动输入数据
            {
                std::cout << "请输入数据个数: ";
                size_t count;
                std::cin >> count;
                std::vector<int> manualData(count);
                std::cout << "请输入 " << count << " 个整数: ";
                for (size_t i = 0; i < count; i++) {
                    std::cin >> manualData[i];
                }
                system.setData(manualData);
                std::cout << "数据输入完成。" << std::endl;
                break;
            }

            case 6: // 显示当前数据
                std::cout << "\n当前数据集:" << std::endl;
                system.printData();
                break;

            case 7: // 运行性能比较
                if (system.getData().empty()) {
                    std::cout << "请先生成或输入数据！" << std::endl;
                    break;
                }
                runPerformanceTest(system);
                break;

            case 8: // 动画演示排序过程
                if (system.getData().empty()) {
                    std::cout << "请先生成或输入数据！" << std::endl;
                    break;
                }
                showSortingAlgorithms();
                int algChoice;
                std::cin >> algChoice;
                if (algChoice >= 1 && algChoice <= 6) {
                    demonstrateSorting(system, algChoice);
                } else {
                    std::cout << "无效的选择！" << std::endl;
                }
                break;

            case 0: // 退出程序
                std::cout << "感谢使用排序算法性能比较与动画演示系统！" << std::endl;
                return 0;

            default:
                std::cout << "无效的选择，请重新输入！" << std::endl;
                break;
        }
    }

    return 0;
}