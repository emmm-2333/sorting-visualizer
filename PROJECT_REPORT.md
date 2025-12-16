# 项目综合报告（2025-12-16）

本项目实现了“排序算法性能比较与动画演示系统”，同时提供控制台版本与 Windows GUI 可视化版本。以下对项目的技术路线、库使用、关键算法与实现要点进行系统性整理与说明，并附改进建议与使用指南。

---

## 1. 项目结构与构建

- 根目录文件：
  - `CMakeLists.txt`：统一的 CMake 构建配置，生成两类可执行文件：
    - 控制台版本：`12_15`（源文件：`main.cpp`、`sorting_system.cpp`）
    - Windows GUI 版本：`win_gui_visualizer.exe`（源文件：`main_win_gui.cpp`、`win_gui_visualizer.cpp`）
  - 源码文件：
    - 控制台核心：`sorting_system.h`、`sorting_system.cpp`、`main.cpp`
    - GUI 核心：`win_gui_visualizer.h`、`win_gui_visualizer.cpp`、`main_win_gui.cpp`
  - 构建输出目录：`build/`、`cmake-build-debug/`

- 构建要点：
  - C++ 标准：`CMAKE_CXX_STANDARD 20`
  - Windows 上定义 Unicode：`add_compile_definitions(UNICODE _UNICODE)`
  - Windows 需要链接的系统库：`gdi32`、`comctl32`、`uxtheme`
  - 已验证 MinGW g++ 编译链（`C:\msys64\ucrt64\bin\g++.exe`）

---

## 2. 技术路线与模块划分

项目采用“核心算法库 + 两种前端（控制台/UI）”的分层设计：

- 核心算法与数据模块（跨前端复用）：
  - `SortingSystem`（`sorting_system.*`）：封装数据生成、数据管理、6 种排序算法实现、性能计数器、稳定性标识、算法测试函数等。

- 控制台前端：
  - `main.cpp`：命令交互菜单，驱动 `SortingSystem`，进行性能测试与简单输出（非动画）。

- Windows GUI 前端：
  - `WinGUIVisualizer`（`win_gui_visualizer.*`）：
    - 负责窗口、控件创建与布局、双缓冲绘制柱状图、动画渲染、暂停/恢复控制、性能统计展示。
    - GUI 的排序实现直接在该类中编写，以便细粒度控制动画（高亮、交换、重绘、sleep 等），与控制台版的 `SortingSystem` 相互独立，实现思路相同但为了可视化做了适配。

---

## 3. 依赖库与 API 使用

- 标准库（跨平台）：
  - `<vector>`, `<string>`, `<chrono>`, `<algorithm>`, `<random>`, `<iostream>`, `<iomanip>`, `<mutex>`, `<atomic>`, `<thread>` 等
  - 用途：数据容器、计时统计、随机打乱、排序逻辑、并发控制（GUI）、打印与格式化输出。

- Windows 专属：
  - `<windows.h>`, `<windowsx.h>`, `<commctrl.h>`：创建窗口与控件（按钮、滑条）、消息循环、消息处理。
  - GDI 绘图：`HDC`, `BitBlt`, `FillRect`, `CreateCompatibleDC/Bitmap`, `TextOutW` 等，用于双缓冲绘制。
  - `uxtheme.dll`：动态调用 `SetWindowTheme`（通过 `LoadLibraryW` + `GetProcAddress`）以应用 Explorer 风格，改善控件观感。
  - Unicode 支持：项目全程使用宽字符 API（`W` 结尾函数），并在 CMake 中定义 `UNICODE/_UNICODE`。

---

## 4. 数据生成策略

两套实现基本一致（GUI/控制台）：

- 数据模式枚举 `DataPattern`：`Random`、`Ascending`、`Descending`、`PartiallySorted`
- 生成逻辑：
  - 初始按 1..N 填充；
  - `Random`：`std::shuffle`
  - `Descending`：`std::reverse`
  - `PartiallySorted`：约 30% 尾段元素打乱（保证 70% 有序）
  - `Ascending`：维持升序
- GUI 侧会根据客户区重新计算 `barWidth` 与缩放高度，限制最大条宽（≤ 40）以保证观感。

---

## 5. 关键算法与可视化逻辑

本项目实现了 6 种经典排序算法，控制台与 GUI 均提供。下面分别总结控制台实现（`SortingSystem`）与 GUI 实现（`WinGUIVisualizer`）的要点。

- 冒泡排序（Bubble Sort）
  - 控制台：外层 n-1、内层 n-i-1，比较后交换；若某轮无交换提前结束。
  - GUI：每次比较高亮两元素，交换使用 `swap(i,j)` 包装（自增 `swapCount`、触发重绘、延时）。支持暂停 `isPaused` 与线程安全访问 `dataMutex`。

- 快速排序（Quick Sort）
  - 控制台：Lomuto 分区，递归调用；`incrementComparisons` 计次。
  - GUI：同思路，`partition(low, high)` 中高亮 `j` 与 `pivot`，交换后重绘并动画；递归时检查 `isSorting` 与 `isPaused`。

- 归并排序（Merge Sort）
  - 控制台：临时左右数组、合并时 `<=` 保持稳定性计数比较。
  - GUI：在 `merge(left, mid, right)` 中逐步写回 `data[k]`，每次写入重绘与 sleep，实现平滑动画；剩余段也逐元素写回并重绘。

- 堆排序（Heap Sort）
  - 控制台：建堆（`heapify`）、逐步交换堆顶与末尾并下滤。
  - GUI：在 `heapify(n, i)` 中逐步比较与下滤，访问元素时持锁，比较与交换后重绘与 sleep。

- 插入排序（Insertion Sort）
  - 控制台：向左移动元素并插入 `key`，移动也记入 `swapCount`（作为“移动次数”统计）。
  - GUI：高亮当前 `key` 位置，向左扫描时每次移动重绘与 sleep，最终插入写回并重绘。

- 选择排序（Selection Sort）
  - 控制台：寻找最小值下标后一次交换。
  - GUI：内层扫描动态高亮当前最小与候选元素，更新最小下标时高亮刷新；最终交换并动画。

- 稳定性判定（控制台与 GUI 的性能报告均采用约定）：
  - 稳定：冒泡、归并、插入
  - 不稳定：快速、选择、堆

---

## 6. GUI 架构与并发控制

- 窗口与控件：
  - 自绘按钮（`BS_OWNERDRAW` + `WM_DRAWITEM`）提升视觉效果；
  - 速度滑条（`TRACKBAR_CLASS`）控制动画延时（`animationDelay = 101 - speed`）；
  - 状态文本与数据大小输入框；
  - 独立 Canvas 子窗口（`VisualizerCanvas`）用于绘制，避免与控件互相覆盖导致闪烁。

- 双缓冲绘制：
  - 在 `drawVisualization()` 中创建内存 DC 与位图，先绘制到内存，再 `BitBlt` 到屏幕；背景填充为黑色，柱状采用蓝色，当前高亮为红色。

- 线程模型：
  - 排序与性能比较在后台线程中运行（`std::thread` + `detach`），避免阻塞 UI；
  - 排序过程中通过 `PostMessage(WM_APP_UPDATE_STATUS)` 与 `WM_APP_SET_PAUSE_TEXT` 让 UI 线程安全更新控件文本；
  - 访问与变更 `data` 使用 `std::mutex dataMutex` 保护；高亮索引使用 `std::atomic<int>` 以跨线程安全读取；
  - 暂停/恢复通过共享标志 `isPaused` 控制，循环中以 `Sleep(100)` 被动等待。

- 防闪烁策略：
  - 使用 `WS_EX_COMPOSITED`、子窗口 Canvas、`WM_ERASEBKGND` 返回 1（避免擦除）、局部 `InvalidateRect(..., FALSE)`（不清背景）等手段减轻闪烁。

---

## 7. 性能统计与报告

- 计数器：`comparisonCount`、`swapCount`，在每次比较与交换/移动时递增。
- 计时：`std::chrono::high_resolution_clock`，以毫秒输出。
- 控制台版：`runPerformanceTest()`（在 `main.cpp`）打印表格，列出各算法耗时、比较次数、交换次数、稳定性。
- GUI 版：`runPerformanceComparison()` 弹窗展示同等信息，排序前重置计数器、计时，结束后汇总数据并恢复原数据集。

---

## 8. 使用说明

- 控制台版本（示例步骤）：
  1. 运行 `12_15.exe`；
  2. 通过菜单生成数据（随机/有序/逆序/部分有序），或手动输入数据；
  3. 选择“运行性能比较”查看各算法表现，或选择“动画演示排序过程”（此为文字输出演示）。

- Windows GUI 版本：
  1. 运行 `win_gui_visualizer.exe`；
  2. 顶部按钮生成不同模式数据；可在输入框设置数据大小（1~200，默认 30）；
  3. 选择排序算法按钮开始动画；
  4. 使用“暂停/继续”控制播放；右侧滑条调节速度；
  5. 点击“性能比较”查看弹窗报告；“重置数据”恢复到初始数据集。

---

## 9. 质量与健壮性考虑

- 边界条件：
  - 数据大小限制与最小条宽确保可视化稳定；
  - 访问 `data` 时统一加锁，避免后台线程与 UI 线程竞争；
  - 排序递归/循环均在合适位置检查 `isSorting` 与 `isPaused`；
  - 计时采用微秒再转毫秒，保证精度，避免负数或零除。

- 可维护性：
  - 控制台与 GUI 的算法实现思路一致但分别编码，便于动画细节控制；
  - 字体与主题设置从属函数封装，避免泄漏与空指针；
  - 通过私有辅助函数（`highlightBars`、`swap`、`sleepAnimation` 等）集中控制副作用。

---

## 10. 建议改进与下一步规划

- 统一算法核心：将 GUI 的排序逻辑抽象为“回调式迭代器/事件钩子”（比较、交换、写入事件），复用 `SortingSystem` 的实现，GUI 仅订阅事件从而绘制动画。这样能减少重复代码与维护成本。
- 更平滑的暂停：改用条件变量（`std::condition_variable`）实现暂停/恢复，避免忙等 `Sleep(100)`。
- 更丰富的可视化：
  - 动态颜色映射（按值大小/已排序区域渐变）
  - 显示数值或索引标签（可切换）
- 交互优化：
  - 禁用按钮于排序期间，避免重复点击引发竞态；
  - 增加“停止”按钮，优雅终止排序线程。
- 性能与工程化：
  - 将 `SetWindowTheme` 的动态加载改为显式链接或封装安全检查；
  - 对 `barWidth` 与缩放计算进一步抽象，支持窗口缩放时的自适应与最小间距。
- 测试与验证：
  - 增加单元测试（对 `SortingSystem` 的 `isSorted()`、各算法性能计数器进行校验）；
  - 增加基准测试脚本，验证不同数据规模下的时间复杂度趋势。

---

## 11. 许可证与版权

- 本报告未引入第三方代码片段；项目主要依赖标准库与 Windows 系统 API；请确保在分发时遵循相关平台与工具链许可条款（例如 MinGW/Windows SDK）。

---

## 12. 快速开始（构建与运行）

- Windows（PowerShell）：

```powershell
# 生成构建目录并运行 CMake（示例）
cmake -S . -B build ; cmake --build build --config Debug

# 运行控制台版本
./build/12_15.exe

# 运行 GUI 版本
./build/win_gui_visualizer.exe
```

如已有 `cmake-build-debug`，也可直接运行其中的可执行文件。

---

如需我进一步将 GUI 与控制台的算法逻辑统一、增加单元测试或完善可视化样式，请告知，我可以继续完善。
