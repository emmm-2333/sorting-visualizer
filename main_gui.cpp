#include "sorting_visualizer.h"
#include <iostream>

int main() {
    std::cout << "Sorting Algorithm Visualizer" << std::endl;
    std::cout << "===========================" << std::endl;
    std::cout << "Starting GUI version..." << std::endl;
    
    // 创建可视化对象并生成初始数据
    SortingVisualizer visualizer;
    visualizer.generateData(50, DataPattern::Random); // 生成50个随机数据
    
    // 开始动画循环
    visualizer.animate();
    
    return 0;
}