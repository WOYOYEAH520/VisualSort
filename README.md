English（后面有中文:）

📖 Overview

VisualSort is an educational tool that brings sorting algorithms to life. Each element is represented as a colored bar whose height corresponds to its value. During sorting, the bars are animated – color changes indicate comparisons, copies, or writes. The program also provides:



Real-time statistics (comparisons, copies, writes, animation steps)

Adjustable animation speed

Support for 20+ classic sorting algorithms

Interactive UI with buttons, switches, and sliders

Error handling and data validation



✨ Features

Rich Algorithm Set: BogoSort, BubbleSort, QuickSort, HeapSort, MergeSort, RadixSort, etc.

Visual Feedback:

Blue highlight → element being read/copied

Red highlight → element being written/modified

Gray-to-white gradient → relative value mapping



Three Data Modes:

int – raw integer sort (for performance measurement)

Counter – count actual operations

Strip – visual bar with full animation

Multi-thread Support: Some algorithms (e.g., SleepSort, parallel std::sort) run in separate threads; the UI remains responsive.

Customizable Window: Switch between fullscreen and windowed mode, adjust width/height.

Data Validation: Enforces constraints (e.g., power-of-two for BitonicSort, size limits).



🛠️ Dependencies

EasyX – Windows graphics library (version 2023 or later)

Windows OS (because EasyX is Windows-only)

C++20 compatible compiler (MSVC recommended)



🔧 Build \& Run

Install EasyX (follow instructions on easyx.cn).

Clone this repository:

git clone https://github.com/yourname/VisualSort.git

Open the solution file (.sln) with Visual Studio (2022 or newer).

Build and run (target platform: x86/x64, Debug/Release).

⚠️ Note: The project uses EasyX in batch drawing mode; ensure your project settings link the EasyX library correctly.



🚀 Usage

Launch the program – you'll see the main menu.

Click Start to choose a sorting algorithm.

Enter the desired data size (subject to algorithm constraints).

Watch the animation! Use the Pause/Resume button and the speed slider.

After sorting, the program verifies correctness and highlights each bar in green (correct) or red (incorrect).



📁 Project Structure

VisualSort.h/cpp – core controller

Sort.h – sorting algorithm definitions

Strip.h – visual bar class with animation

Counter.h – operation counter wrapper

Button.h / Sketch.h – UI components

Fraction.h – precise arithmetic for layout

MainMenu.h – entry point and menu system



🔧 Custom Sorting Algorithms

VisualSort makes it easy to add or remove sorting algorithms for testing and experimentation.



Removing an Algorithm

Open VisualSort.h.

Locate the VisualSort class constructor inside the NVisualSort namespace.

Find the initializer list for this->m\_sorts and delete the line corresponding to the algorithm you want to remove. (Make sure there's no trailing comma after the last item.)



Adding a New Algorithm

Define the sorting function

In Sort.h, inside the NVisualSort::NSortAlgorithms namespace, write your own template function following the pattern of existing algorithms (e.g., BubbleSort).

Signature: template<class T> void YourSort(std::vector<T>\& data\_)

The function should operate only on the data\_ array (comparisons, swaps, assignments). Avoid global variables or external I/O, as they won't be visualized.

Important: Do not use move operations (std::move) because the Strip type lacks visualization support for moves.

You can mimic non‑recursive implementations (e.g., CycleSort) or use an explicit stack (like the iterative QuickSort). If recursion is unavoidable, ensure the depth is safe.



Register the algorithm

Back in VisualSort.h, add a new Sort object to the this->m\_sorts initializer list in the VisualSort constructor.



Constructor parameters (in order):

Name (wide string, e.g., L"MySort")

Maximum allowed data size (int)

Three callable objects for int, Counter, and Strip versions (usually your template function instantiated with the appropriate type)

(Optional) A std::vector<NumRequire> for data constraints

(Optional) true if the algorithm is unpredictable (like BogoSort)

(Optional) true if it is multi‑threaded (disables the exit button)



Example:

Sort(L"MySort", 1024, MySort<int>, MySort<Counter>, MySort<Strip>)

Remember: no trailing comma after the last item.



Important Notes

All operations must be performed on array elements (e.g., data\_\[i]). Operations on local variables are not counted or visualized.

If your algorithm uses auxiliary storage (like a temporary array in merge sort), make sure its element type is T, not int, so that Counter and Strip behave correctly.

For multi‑threaded algorithms, you are responsible for thread safety. The animation functions (Strip::DrawStrip1, etc.) are already thread‑local and safe to call from worker threads.

After adding your algorithm, rebuild the project – it will appear automatically in the algorithm selection menu.



中文



📖 项目简介

VisualSort 是一个基于 C++20 和 EasyX 图形库的排序算法可视化工具。它将每个数据项绘制为彩色条形，高度对应数值。排序过程中，条形会通过颜色变化（蓝色表示读取/复制，红色表示写入）动态展示算法的每一步，并实时统计比较次数、移动次数和动画步数，是学习算法和数据结构的绝佳辅助工具。



✨ 主要特点

20+ 种排序算法：从冒泡、快排到猴子排序、睡眠排序，应有尽有。



直观的视觉反馈：

蓝色高亮：元素被读取或复制

红色高亮：元素被写入或修改

灰度渐变：条形颜色随数值大小变化



三种数据模式：

int：原始整数排序（用于测量真实耗时）

Counter：计数组件，精确统计操作次数

Strip：可视化条形，包含完整动画

多线程支持：部分算法（如睡眠排序、并行 std::sort）在独立线程中运行，界面不会卡死。

可调节窗口：支持全屏/窗口切换，动态调整窗口尺寸。

数据合法性检查：根据算法要求验证输入数据量（如双调排序要求数据量为 2 的幂）。



🛠️ 依赖

EasyX 图形库（2023 或更高版本）

Windows 操作系统（EasyX 仅支持 Windows）

支持 C++20 的编译器（推荐 Visual Studio）



🔧 编译与运行

安装 EasyX（访问 easyx.cn 下载安装）。

克隆本仓库：

git clone https://github.com/yourname/VisualSort.git

用 Visual Studio（2022 或更新）打开解决方案文件 (.sln)。

选择目标平台（x86/x64）和配置（Debug/Release），编译运行。



🚀 使用说明

启动程序，进入主菜单。

点击 开始，选择一种排序算法。

输入数据量（注意算法可能有额外约束）。

观看动画！可用 暂停/继续 按钮和速度滑块控制演示。

排序结束后，程序会自动验证结果：正确的条形变为绿色，错误的变为红色。



📁 项目结构

VisualSort.h/cpp – 核心控制器

Sort.h – 排序算法定义

Strip.h – 可视化条形类（含动画）

Counter.h – 操作计数组件

Button.h / Sketch.h – 界面元素

Fraction.h – 精确分数计算（用于布局）

MainMenu.h – 入口与主菜单

🔧 自定义排序算法

VisualSort 支持轻松添加或删除排序算法，方便你测试自己的实现。



删除算法

打开 VisualSort.h 文件。

在 NVisualSort 命名空间的 VisualSort 类的默认构造函数中，找到成员 this->m\_sorts 的初始化列表。

删除你不想要的 Sort 对象那一行（注意列表最后一项后面不要有多余的逗号）。



添加算法

定义排序函数

在 Sort.h 文件的 NVisualSort::NSortAlgorithms 命名空间中，模仿已有的排序算法（如 BubbleSort）编写你自己的模板函数。

函数签名必须为：template<class T> void YourSort(std::vector<T>\& data\_)。

函数内部只对 data\_ 数组进行的操作（例如比较、交换、赋值）会被可视化。

重要：避免使用移动语义（std::move），因为 Strip 类型没有针对移动操作的可视化支持。

可以参照 CycleSort、MergeSort 等非递归实现，或使用栈模拟递归（如 QuickSort 的非递归版本）。如果必须使用递归，请确保递归深度可控。



注册算法

回到 VisualSort.h 中 VisualSort 的构造函数，在 this->m\_sorts 的初始化列表中添加一个新的 Sort 对象。



构造参数依次为：

排序名称（宽字符串，如 L"我的排序"）

允许的最大数据量（int）

三个函数指针/可调用对象：分别对应 int、Counter、Strip 版本的排序函数（通常直接使用你定义的模板函数即可）

（可选）数据量约束列表（std::vector<NumRequire>）

（可选）是否不可预测（如猴子排序设为 true）

（可选）是否多线程（设为 true 会禁用退出按钮）



例如：



Sort(L"我的排序", 1024, MySort<int>, MySort<Counter>, MySort<Strip>)

注意列表最后一项末尾不要加逗号。



注意事项

所有对数据的操作必须通过数组元素进行（如 data\_\[i]），单独的局部变量操作不会被统计和可视化。

多线程算法需要自行管理线程安全，并确保动画函数（Strip::DrawStrip1 等）能在子线程中正确调用（已内置线程局部存储支持）。

添加后重新编译即可在菜单中看到新算法。

