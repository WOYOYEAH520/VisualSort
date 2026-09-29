# VisualSort

一个基于 **C++20** 和 **EasyX** 图形库的排序算法可视化工具。20+ 种排序算法、实时统计、可调速度、动画展示。

> [English](#english) · [中文](#中文)

---

<a id="english"></a>
## English

### 📖 Overview

VisualSort is an educational tool that brings sorting algorithms to life. Each element is rendered as a colored bar whose height corresponds to its value. During sorting, the bars are animated — color changes indicate comparisons, copies, or writes. Real-time statistics are also displayed.

### ✨ Features

- **20+ algorithms**: BogoSort, BubbleSort, QuickSort, HeapSort, MergeSort, RadixSort, CountingSort, SleepSort, StoogeSort, CycleSort, BitonicSort, Parallel `std::sort`, and more.
- **Visual feedback**:
  - 🔵 Blue — element is being read / copied
  - 🔴 Red — element is being written / modified
  - ⚪ Gray-to-white gradient — relative value mapping
- **Three data modes** (all driven by the same algorithm implementation):
  - `int` — raw integer sort, used to measure real time
  - `Counter` — counts actual operations
  - `Strip` — visual bar with full animation
- **Multi-thread support**: Algorithms such as SleepSort and parallel `std::sort` run in worker threads; the UI stays responsive. Algorithms marked as *multi-threaded* enter a "fast-forward" mode on exit (skipping animations) so the sort converges quickly.
- **Customizable window**: Fullscreen / windowed, adjustable width & height.
- **Data validation**: Per-algorithm constraints (e.g., BitonicSort requires power-of-two sizes; a maximum size limit).

### 🛠️ Dependencies

- **EasyX** (Windows graphics library, 2023 or later)
- **Windows** (EasyX is Windows-only)
- **C++20**-compatible compiler (MSVC recommended)
- **Visual Studio 2022 17.10+** (for the `.slnx` solution format)

### 🔧 Build & Run

1. Install EasyX (see [easyx.cn](https://easyx.cn)).
2. Clone this repository:

   ```bash
   git clone https://github.com/WOYOYEAH520/VisualSort.git
   ```

3. Open `VisualSort.slnx` with Visual Studio 2022 (17.10+).
4. Build and run (x86 or x64, Debug or Release).

> ⚠️ The project uses EasyX in batch-drawing mode. Make sure the EasyX library is linked correctly.

### 📁 Project Structure

```
VisualSort/
├── src/
│   ├── Common/     – Core utilities (WideError, ScopeGuard, Coordinate, Fraction)
│   ├── Graphics/   – Configuration and drawing (ConfigManager, DrawingTool)
│   ├── UI/         – UI widgets (Sketch, Button, Dialog, InputBox)
│   ├── Sort/       – Sorting core (Counter, Strip, Sort)
│   ├── App/        – Application layer (VisualSort, MainMenu)
│   └── main.cpp    – Entry point
├── VisualSort.slnx
├── VisualSort.vcxproj
├── VisualSort.vcxproj.filters
└── README.md
```

All headers are resolved through **Additional Include Directories** in `VisualSort.vcxproj` (`src/Common`, `src/Graphics`, `src/UI`, `src/Sort`, `src/App`), so `#include "Foo.h"` works from any file regardless of its location.

### 🚀 Usage

1. Launch the program → main menu.
2. Click **Start** → choose a sorting algorithm.
3. Enter the desired data size (subject to algorithm-specific constraints).
4. Watch the animation. Use **Pause / Resume** and the speed slider to control playback.
5. When sorting finishes, the result is verified automatically: correct bars turn **green**, incorrect ones turn **red**.

### 🔧 Custom Sorting Algorithms

#### Removing an algorithm

1. Open `src/App/VisualSort.h`.
2. Locate the `VisualSort` constructor.
3. Find the initializer list for `this->m_sorts` and delete the line you don't need.

#### Adding an algorithm

1. **Write the function** — in `src/Sort/Sort.h`, inside `namespace NVisualSort::NSortAlgorithms`, add:

   ```cpp
   template<class T>
   void MySort(std::vector<T>& data_) {
       // only operate on data_[i]
   }
   ```

   - Only operations on **array elements** (`data_[i]`) are visualized.
   - Avoid `std::move` — the `Strip` type does not visualize moves.
   - Prefer non-recursive implementations, or use an explicit stack (see `QuickSort` / `MergeSort`).
   - Auxiliary arrays should use `T` as the element type, not `int`.

2. **Register it** — back in `src/App/VisualSort.h`, add a line to the `this->m_sorts` initializer list:

   ```cpp
   Sort(L"MySort", 1024, MySort<int>, MySort<Counter>, MySort<Strip>),
   ```

   Constructor parameters (in order):
   - name (wide string)
   - maximum data size (`int`)
   - `int`-version, `Counter`-version, `Strip`-version of the algorithm
   - *(optional)* `std::vector<NumRequire>` constraints
   - *(optional)* `true` if unpredictable (e.g., BogoSort)
   - *(optional)* `true` if multi-threaded

3. Rebuild. The new algorithm appears automatically in the menu.

### 📜 License

[MIT License](LICENSE) © 2026 WOYOYEAH520

---

<a id="中文"></a>
## 中文

### 📖 项目简介

VisualSort 是一个基于 **C++20** 和 **EasyX** 图形库的排序算法可视化工具。它将每个数据项绘制为彩色条形，高度对应数值。排序过程中，条形会通过颜色变化动态展示算法的每一步，并实时统计比较次数、移动次数与动画步数。

### ✨ 主要特点

- **20+ 种排序算法**：冒泡、快排、堆排、归并、基数、计数、睡眠、臭皮匠、猴子…… 应有尽有。
- **直观的视觉反馈**：
  - 🔵 蓝色高亮 —— 元素被读取 / 复制
  - 🔴 红色高亮 —— 元素被写入 / 修改
  - ⚪ 灰度渐变 —— 条形颜色随数值大小变化
- **三种数据模式**（同一份算法代码驱动）：
  - `int` —— 原始整数排序，用于测量真实耗时
  - `Counter` —— 精确统计操作次数
  - `Strip` —— 可视化条形，含完整动画
- **多线程支持**：睡眠排序、并行 `std::sort` 等算法在独立线程中运行，界面保持响应。标记为多线程的算法在退出时会进入"快进"模式（跳过动画），让排序快速收敛。
- **可调节窗口**：支持全屏 / 窗口切换，动态调整宽高。
- **数据合法性检查**：每种算法可自带约束（如双调排序要求数据量为 2 的幂，以及最大数据量限制）。

### 🛠️ 依赖

- **EasyX** 图形库（2023 或更高版本）
- **Windows** 操作系统（EasyX 仅支持 Windows）
- 支持 **C++20** 的编译器（推荐 MSVC）
- **Visual Studio 2022 17.10 或更高版本**（用于打开 `.slnx` 解决方案格式）

### 🔧 编译与运行

1. 安装 EasyX（参考 [easyx.cn](https://easyx.cn)）。
2. 克隆本仓库：

   ```bash
   git clone https://github.com/WOYOYEAH520/VisualSort.git
   ```

3. 用 Visual Studio 2022（17.10+）打开 `VisualSort.slnx`。
4. 选择目标平台（x86 或 x64）和配置（Debug 或 Release），编译运行。

> ⚠️ 项目使用 EasyX 的批量绘制模式，请确保 EasyX 库已正确链接。

### 📁 项目结构

```
VisualSort/
├── src/
│   ├── Common/     – 基础工具（WideError、ScopeGuard、Coordinate、Fraction）
│   ├── Graphics/   – 配置与绘图（ConfigManager、DrawingTool）
│   ├── UI/         – 界面组件（Sketch、Button、Dialog、InputBox）
│   ├── Sort/       – 排序核心（Counter、Strip、Sort）
│   ├── App/        – 应用层（VisualSort、MainMenu）
│   └── main.cpp    – 程序入口
├── VisualSort.slnx
├── VisualSort.vcxproj
├── VisualSort.vcxproj.filters
└── README.md
```

所有头文件通过 `VisualSort.vcxproj` 里的**附加包含目录**解析（`src/Common`、`src/Graphics`、`src/UI`、`src/Sort`、`src/App`），因此在任意文件中写 `#include "Foo.h"` 都能正常编译。

### 🚀 使用说明

1. 启动程序，进入主菜单。
2. 点击 **开始** → 选择一种排序算法。
3. 输入数据量（注意算法可能有额外约束）。
4. 观看动画。可用 **暂停 / 继续** 按钮和速度滑块控制演示。
5. 排序结束后，程序自动验证结果：正确的条形变**绿色**，错误的变**红色**。

### 🔧 自定义排序算法

#### 删除算法

1. 打开 `src/App/VisualSort.h`。
2. 找到 `VisualSort` 类的构造函数。
3. 在 `this->m_sorts` 的初始化列表中，删除你不要的那一行。

#### 添加算法

1. **写函数** —— 在 `src/Sort/Sort.h` 的 `namespace NVisualSort::NSortAlgorithms` 中，按已有算法的模式添加：

   ```cpp
   template<class T>
   void MySort(std::vector<T>& data_) {
       // 只对 data_[i] 操作
   }
   ```

   - 只有对**数组元素**（`data_[i]`）的操作才会被可视化。
   - 避免 `std::move` —— `Strip` 类型不支持移动操作的可视化。
   - 优先使用非递归实现，或用显式栈模拟（参考 `QuickSort` / `MergeSort`）。
   - 辅助数组的元素类型必须是 `T`，不要写成 `int`。

2. **注册** —— 回到 `src/App/VisualSort.h`，在 `this->m_sorts` 的初始化列表中添加一行：

   ```cpp
   Sort(L"我的排序", 1024, MySort<int>, MySort<Counter>, MySort<Strip>),
   ```

   构造参数依次为：
   - 排序名称（宽字符串）
   - 允许的最大数据量（`int`）
   - `int` 版本、`Counter` 版本、`Strip` 版本的排序函数
   - *(可选)* `std::vector<NumRequire>` 数据量约束
   - *(可选)* 是否不可预测（如猴子排序设为 `true`）
   - *(可选)* 是否多线程

3. 重新编译。新算法会自动出现在菜单中。

### 📜 许可证

[MIT License](LICENSE) © 2026 WOYOYEAH520