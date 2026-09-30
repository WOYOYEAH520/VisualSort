# VisualSort

基于 **C++20** 和 **EasyX** 的排序算法可视化工具。**200+ 种排序算法**、实时统计、可调速度、动画展示。

> [English](#english) · [中文](#中文)

---

<a id="english"></a>
## English

### 📖 Overview

VisualSort is an educational tool that brings sorting algorithms to life. Each element is rendered as a colored bar whose height corresponds to its value. During sorting, the bars are animated — color changes indicate comparisons, copies, or writes. Real-time statistics are displayed as well.

The algorithm library is a C++20 port of the **ArrayV** project (Java, MIT — <https://github.com/Gaming32/ArrayV>), covering exchange, selection, insertion, merge, distribution, hybrid, concurrent, and quick sort families.

### ✨ Features

- **200+ sorting algorithms** — from classic bubble/quick/merge to exotic ones (Bogo, Stooge, Sleep, Pancake, Stalin, Grail, Wiki, Kota, Ecta, and many more).
- **Visual feedback**:
  - 🔵 Blue — element is being read / copied
  - 🔴 Red — element is being written / modified
  - ⚪ Gray-to-white gradient — relative value mapping
- **Three data modes** (all driven by the same algorithm implementation):
  - `int` — raw integer sort, used to measure real time
  - `Counter` — counts actual operations
  - `Strip` — visual bar with full animation
- **Multi-thread support** — SleepSort, parallel `std::sort`, parallel merge/bitonic, and more.
- **Customizable window** — fullscreen / windowed, adjustable width & height.
- **Data validation** — per-algorithm constraints (e.g. BitonicSort requires power-of-two sizes).

### 🛠️ Dependencies

- **EasyX** (Windows graphics library, 2023 or later)
- **Windows** (EasyX is Windows-only)
- **C++20**-compatible compiler (MSVC recommended)
- **Visual Studio 2022 17.10+** (for the `.slnx` solution format)

### ⚠️ Required Compiler Options: `/utf-8` and `/bigobj`

This project uses **UTF-8 encoded source files** with plenty of Chinese strings, and it instantiates a very large number of function templates (each sorting algorithm exists in three variants: `int`, `Counter`, `Strip`). Therefore **two** compiler options are mandatory:

#### 1. `/utf-8` — for UTF-8 source files

All `.hpp` files are saved as UTF-8. MSVC, by default, parses source files using the system code page (e.g. GBK 936 on Chinese systems). Without `/utf-8`, you will get errors like:

```
error C2001: 常量中有换行符
error C3688: 文本后缀“...”无效；未找到文文本运算符或文本运算符模板“operator ""...” 
```

**Add `/utf-8` before building:**

- Visual Studio: `Project Properties → C/C++ → Command Line → Additional Options` → add `/utf-8`
- Or in `VisualSort.vcxproj`:
  ```xml
  <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
  ```

#### 2. `/bigobj` — for large object files

Without `/bigobj`, MSVC will fail with:

```
fatal error C1128: 节数超过对象文件格式限制: 请使用 /bigobj 重新编译
(fatal error C1128: number of sections exceeded object file format limit: compile with /bigobj)
```

**Add `/bigobj` as well:**

- Visual Studio: `Project Properties → C/C++ → Command Line → Additional Options` → add `/bigobj`
- Or in `VisualSort.vcxproj`:
  ```xml
  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
  ```

`/bigobj` is already configured in the committed `.vcxproj`.  
**`/utf-8` may need to be added manually** — please check before building.

### 🔧 Build & Run

1. Install EasyX (see [easyx.cn](https://easyx.cn)).
2. Clone this repository:
   ```bash
   git clone https://github.com/WOYOYEAH520/VisualSort.git
   ```
3. Open `VisualSort.slnx` with Visual Studio 2022 (17.10+).
4. **Make sure `/utf-8` and `/bigobj` are present** (see above).
5. Build and run (x86 or x64, Debug or Release).

### 📁 Project Structure

```
VisualSort/
├── src/
│   ├── Common/     – Core utilities (WideError, ScopeGuard, Coordinate, Fraction, Channel)
│   ├── Graphics/   – Configuration and drawing (ConfigManager, DrawingTool)
│   ├── UI/         – UI widgets (Sketch, Button, Dialog, InputBox)
│   ├── Sort/       – Sorting core
│   │   ├── Counter.hpp, Strip.hpp
│   │   ├── Sort.hpp              – Sort registration and entry points
│   │   ├── SortHelpers.hpp       – Shared helpers (grail, wiki, kota, tim, pdq, quad)
│   │   ├── ExchangeSorts.hpp     – Exchange family
│   │   ├── SelectSorts.hpp       – Selection / cycle / heap / tournament family
│   │   ├── InsertSorts.hpp       – Insertion / shell / library / tree family
│   │   ├── MergeSorts.hpp        – Merge family
│   │   ├── DistributeSorts.hpp   – Distribution / radix / bogo family
│   │   ├── HybridSorts.hpp       – Hybrid family (Grail, Wiki, Kota, Ecta, Tim, ...)
│   │   ├── ConcurrentSorts.hpp   – Sorting networks and threaded sorts
│   │   ├── MiscSorts.hpp         – Pancake, Stalin, etc.
│   │   └── QuickSorts.hpp        – Ternary quicksorts
│   ├── App/        – Application layer (VisualSort, MainMenu, MouseMessageSource)
│   └── main.cpp    – Entry point
├── VisualSort.slnx
├── VisualSort.vcxproj
├── VisualSort.vcxproj.filters
└── README.md
```

All headers are resolved through **Additional Include Directories** in `VisualSort.vcxproj`, so `#include "Foo.hpp"` works from any file regardless of its location.

### 🚀 Usage

1. Launch the program → main menu.
2. Click **Start** → choose a sorting algorithm.
3. Enter the desired data size (subject to algorithm-specific constraints).
4. Watch the animation. Use **Pause / Resume** and the speed slider.
5. When done, the result is verified: correct bars turn **green**, incorrect ones **red**.

### 🔧 Adding a Custom Sort

1. Write your template function in any of the family headers (`src/Sort/*Sorts.hpp`) inside `namespace NVisualSort::NSortAlgorithms`:
   ```cpp
   template<class T>
   void MySort(std::vector<T>& data_) {
       // only operate on data_[i]
   }
   ```
   Rules:
   - Only operations on array elements (`data_[i]`) are visualized.
   - Avoid `std::move` — `Strip` does not visualize moves.
   - Auxiliary arrays must use `T` as the element type, not `int`.

2. Register in `src/App/VisualSort.hpp`'s `m_sorts` initializer list:
   ```cpp
   Sort(L"MySort", 1024, MySort<int>, MySort<Counter>, MySort<Strip>),
   ```
   Constructor parameters: name, max size, three typed variants, *(optional)* `NumRequire` constraints, *(optional)* unpredictable flag, *(optional)* multi-thread flag.

3. Rebuild (make sure `/utf-8` and `/bigobj` are on). The new sort appears in the menu.

### 📜 License

[MIT License](LICENSE) © 2026 WOYOYEAH520

---

<a id="中文"></a>
## 中文

### 📖 项目简介

VisualSort 是一个基于 **C++20** 和 **EasyX** 图形库的排序算法可视化工具。**200+ 种排序算法**，动画展示每一步比较/赋值，并实时统计操作次数。

算法库是从 **ArrayV**（Java，MIT 协议 — <https://github.com/Gaming32/ArrayV>）移植到 C++20 的，涵盖交换、选择、插入、归并、分配、混合、并发、快速等多个分类。

### ✨ 主要特点

- **200+ 种排序算法** —— 从经典冒泡、快排、归并，到奇葩的猴子排序、臭皮匠排序、睡眠排序、煎饼排序、斯大林排序，以及 Grail、Wiki、Kota、Ecta 等高级算法。
- **直观的视觉反馈**：
  - 🔵 蓝色高亮 —— 元素被读取 / 复制
  - 🔴 红色高亮 —— 元素被写入 / 修改
  - ⚪ 灰度渐变 —— 条形颜色随数值大小变化
- **三种数据模式**（同一份算法代码驱动）：
  - `int` —— 原始整数排序，用于测量真实耗时
  - `Counter` —— 精确统计操作次数
  - `Strip` —— 可视化条形，含完整动画
- **多线程支持** —— 睡眠排序、并行 `std::sort`、并行归并/双调等算法均在独立线程中运行。
- **可调节窗口** —— 支持全屏 / 窗口切换，动态调整宽高。
- **数据合法性检查** —— 每种算法可自带约束（如双调排序要求数据量为 2 的幂）。

### 🛠️ 依赖

- **EasyX** 图形库（2023 或更高版本）
- **Windows** 操作系统（EasyX 仅支持 Windows）
- 支持 **C++20** 的编译器（推荐 MSVC）
- **Visual Studio 2022 17.10 或更高版本**（用于打开 `.slnx` 解决方案格式）

### ⚠️ 必须的编译选项：`/utf-8` 和 `/bigobj`

本项目使用 **UTF-8 编码的源文件**，包含大量中文字符串；同时实例化了**大量函数模板**（每个排序算法都有 `int` / `Counter` / `Strip` 三个版本）。因此以下两个编译选项**必须添加**：

#### 1. `/utf-8` —— 用于 UTF-8 源文件

所有 `.hpp` 文件均保存为 UTF-8。MSVC 默认按系统代码页（中文系统为 GBK 936）解析源文件。如果不加 `/utf-8`，会报错：

```
error C2001: 常量中有换行符
error C3688: 文本后缀“...”无效；未找到文文本运算符或文本运算符模板“operator ""...”
```

**编译前请添加 `/utf-8`：**

- VS 图形界面：`项目属性 → C/C++ → 命令行 → 其他选项` → 填 `/utf-8`
- 或者直接在 `VisualSort.vcxproj` 中：
  ```xml
  <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
  ```

#### 2. `/bigobj` —— 用于大型对象文件

不加 `/bigobj`，MSVC 会报错：

```
fatal error C1128: 节数超过对象文件格式限制: 请使用 /bigobj 重新编译
```

**同时添加 `/bigobj`：**

- VS 图形界面：`项目属性 → C/C++ → 命令行 → 其他选项` → 填 `/bigobj`
- 或者直接在 `VisualSort.vcxproj` 中：
  ```xml
  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
  ```

仓库里已经配置好了 `/bigobj`。  
**`/utf-8` 可能需要你手动确认添加**——编译前请务必检查。

### 🔧 编译与运行

1. 安装 EasyX（参考 [easyx.cn](https://easyx.cn)）。
2. 克隆本仓库：
   ```bash
   git clone https://github.com/WOYOYEAH520/VisualSort.git
   ```
3. 用 Visual Studio 2022（17.10+）打开 `VisualSort.slnx`。
4. **确认已添加 `/utf-8` 和 `/bigobj`**（见上文）。
5. 选择目标平台（x86 或 x64）和配置（Debug 或 Release），编译运行。

### 📁 项目结构

```
VisualSort/
├── src/
│   ├── Common/     – 基础工具（WideError、ScopeGuard、Coordinate、Fraction、Channel）
│   ├── Graphics/   – 配置与绘图（ConfigManager、DrawingTool）
│   ├── UI/         – 界面组件（Sketch、Button、Dialog、InputBox）
│   ├── Sort/       – 排序核心
│   │   ├── Counter.hpp, Strip.hpp
│   │   ├── Sort.hpp              – 排序注册与入口
│   │   ├── SortHelpers.hpp       – 共享工具（grail、wiki、kota、tim、pdq、quad 等）
│   │   ├── ExchangeSorts.hpp     – 交换类
│   │   ├── SelectSorts.hpp       – 选择 / 循环 / 堆 / 锦标赛类
│   │   ├── InsertSorts.hpp       – 插入 / 希尔 / 图书馆 / 树类
│   │   ├── MergeSorts.hpp        – 归并类
│   │   ├── DistributeSorts.hpp   – 分配 / 基数 / 猴子类
│   │   ├── HybridSorts.hpp       – 混合类（Grail、Wiki、Kota、Ecta、Tim 等）
│   │   ├── ConcurrentSorts.hpp   – 排序网络与多线程排序
│   │   ├── MiscSorts.hpp         – 煎饼、斯大林等
│   │   └── QuickSorts.hpp        – 三路快速排序
│   ├── App/        – 应用层（VisualSort、MainMenu、MouseMessageSource）
│   └── main.cpp    – 程序入口
├── VisualSort.slnx
├── VisualSort.vcxproj
├── VisualSort.vcxproj.filters
└── README.md
```

所有头文件通过 `VisualSort.vcxproj` 里的**附加包含目录**解析，任意文件中写 `#include "Foo.hpp"` 都能正常编译。

### 🚀 使用说明

1. 启动程序，进入主菜单。
2. 点击 **开始** → 选择一种排序算法。
3. 输入数据量（注意算法可能有额外约束）。
4. 观看动画。可用 **暂停 / 继续** 按钮和速度滑块控制演示。
5. 排序结束后，程序自动验证结果：正确的条形变**绿色**，错误的变**红色**。

### 🔧 自定义排序算法

1. 在任意家族头文件（`src/Sort/*Sorts.hpp`）的 `namespace NVisualSort::NSortAlgorithms` 中写模板函数：
   ```cpp
   template<class T>
   void MySort(std::vector<T>& data_) {
       // 只对 data_[i] 操作
   }
   ```
   注意：
   - 只有对**数组元素**（`data_[i]`）的操作才会被可视化。
   - 避免 `std::move` —— `Strip` 不支持移动操作的可视化。
   - 辅助数组的元素类型必须是 `T`，不要写成 `int`。

2. 回到 `src/App/VisualSort.hpp`，在 `m_sorts` 初始化列表中添加一行：
   ```cpp
   Sort(L"我的排序", 1024, MySort<int>, MySort<Counter>, MySort<Strip>),
   ```
   构造参数依次为：名称、最大数据量、`int`/`Counter`/`Strip` 三个版本函数、*(可选)* 数据量约束列表、*(可选)* 是否不可预测、*(可选)* 是否多线程。

3. 重新编译（记得 `/utf-8` 和 `/bigobj`），新算法会出现在菜单中。

### 📜 许可证

[MIT License](LICENSE) © 2026 WOYOYEAH520

### 🙏 致谢

- [ArrayV](https://github.com/Gaming32/ArrayV) —— 本项目的算法库来源（Java，MIT）
- [EasyX](https://easyx.cn) —— 图形库