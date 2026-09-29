#pragma once
#include "Sort.h"
#include "Dialog.h"
#include "ConfigManager.h"
#include "Counter.h"
#include "Button.h"
#include "DrawingTool.h"
#include "Sketch.h"
#include "Strip.h"
#include <Windows.h>
#include <easyx.h>
#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>
#include "Fraction.h"
#include "WideError.h"
#include <atomic>
#include <algorithm>
#include "ScopeGuard.h"
#include <shared_mutex>
#include <cmath>
#include <random>

namespace NVisualSort {

	class VisualSort {

		friend class MainMenu;

	private:

		std::vector<int> m_sourceData;
		std::function<void(size_t, std::vector<int>&)> m_initDataFunc;

		std::vector<int> m_intSortData;
		std::vector<Counter> m_counterSortData;
		std::vector<Strip> m_stripSortData;

		std::optional<size_t> m_sortIndex = std::nullopt;
		std::vector<Sort> m_sorts;
		bool m_showShuffle = false;
		Fraction m_displaySpeed = Fraction(1, 2); // 演示速度（每调用一次 DrawStrip，就睡 m_displaySpeed 毫秒，各线程互不干扰）
		std::shared_mutex m_speedMutex;

		ButtonSequence m_controlButtons;
		std::vector<Sketch> m_messages;

		std::chrono::steady_clock::time_point m_stripSortBeginTime;
		std::chrono::microseconds m_intSortDuration = {};

		VisualSort() {
			this->m_initDataFunc = [](size_t data_size_, std::vector<int>& data_) {
				data_.resize(data_size_);
				for (size_t dataIndex = 0; dataIndex < data_.size(); ++dataIndex) {
					data_[dataIndex] = static_cast<int>(dataIndex + 1);
				}
				};
			using namespace NSortAlgorithms;
			this->m_sorts = {
				Sort(L"猴子排序",8,BogoSort<int>,BogoSort<Counter>,BogoSort<Strip>,{},true),
				Sort(L"臭皮匠排序",64,StoogeSort<int>,StoogeSort<Counter>,StoogeSort<Strip>),
				Sort(L"睡眠排序",128,SleepSort<int>,SleepSort<Counter>,SleepSort<Strip>,{},true),
				Sort(L"循环排序",256,CycleSort<int>,CycleSort<Counter>,CycleSort<Strip>),
				Sort(L"冒泡排序",256,BubbleSort<int>,BubbleSort<Counter>,BubbleSort<Strip>),
				Sort(L"双向冒泡排序",256,BidirectionalBubbleSort<int>,BidirectionalBubbleSort<Counter>,BidirectionalBubbleSort<Strip>),
				Sort(L"奇偶排序",256,OddEvenSort<int>,OddEvenSort<Counter>,OddEvenSort<Strip>),
				Sort(L"选择排序",256,SelectionSort<int>,SelectionSort<Counter>,SelectionSort<Strip>),
				Sort(L"双向选择排序",256,BidirectionalSelectionSort<int>,BidirectionalSelectionSort<Counter>,BidirectionalSelectionSort<Strip>),
				Sort(L"插入排序",256,InsertionSort<int>,InsertionSort<Counter>,InsertionSort<Strip>),
				Sort(L"珠排序",256,BeadSort<int>,BeadSort<Counter>,BeadSort<Strip>),
				Sort(L"梳排序",8192,CombSort<int>,CombSort<Counter>,CombSort<Strip>),
				Sort(L"希尔排序",8192,ShellSort<int>,ShellSort<Counter>,ShellSort<Strip>),
				Sort(L"双调排序",8192,BitonicSort<int>,BitonicSort<Counter>,BitonicSort<Strip>,
					{{L"数据量必须为2的正整数次幂",[](size_t data_size_)->bool { return ((data_size_ & (data_size_ - 1)) == 0) && data_size_ > 0; }}}),
				Sort(L"归并排序",8192,MergeSort<int>,MergeSort<Counter>,MergeSort<Strip>),
				Sort(L"堆排序",8192,HeapSort<int>,HeapSort<Counter>,HeapSort<Strip>),
				Sort(L"快速排序",8192,QuickSort<int>,QuickSort<Counter>,QuickSort<Strip>),
				Sort(L"基数排序",8192,RadixSort<int>,RadixSort<Counter>,RadixSort<Strip>),
				Sort(L"计数排序",32768,CountingSort<int>,CountingSort<Counter>,CountingSort<Strip>),
				Sort(L"std::sort",8192,StdSort<int>,StdSort<Counter>,StdSort<Strip>),
				Sort(L"并行std::sort",8192,StdSort_Parallel<int>,StdSort_Parallel<Counter>,StdSort_Parallel<Strip>,{},false,true),
				Sort(L"std::stable_sort",8192,StdStableSort<int>,StdStableSort<Counter>,StdStableSort<Strip>),
				Sort(L"std::sort_heap",8192,StdHeapSort<int>,StdHeapSort<Counter>,StdHeapSort<Strip>),
				Sort(L"std::partial_sort",8192,StdPartialSort<int>,StdPartialSort<Counter>,StdPartialSort<Strip>),
				Sort(L"二分地精排序",256,BinaryGnomeSort<int>,BinaryGnomeSort<Counter>,BinaryGnomeSort<Strip>),
				Sort(L"冒泡猴子排序",32,BubbleBogoSort<int>,BubbleBogoSort<Counter>,BubbleBogoSort<Strip>,{},true,false),
				Sort(L"冒泡排序(ArrayV)",256,BubbleSortJava<int>,BubbleSortJava<Counter>,BubbleSortJava<Strip>),
				Sort(L"环形排序(迭代)",1024,CircleSortIterative<int>,CircleSortIterative<Counter>,CircleSortIterative<Strip>),
				Sort(L"环形排序(递归)",1024,CircleSortRecursive<int>,CircleSortRecursive<Counter>,CircleSortRecursive<Strip>),
				Sort(L"Circloid Sort",1024,CircloidSort<int>,CircloidSort<Counter>,CircloidSort<Strip>),
				Sort(L"三平滑梳排序(经典)",1024,ClassicThreeSmoothCombSort<int>,ClassicThreeSmoothCombSort<Counter>,ClassicThreeSmoothCombSort<Strip>),
				Sort(L"鸡尾酒排序",256,CocktailShakerSort<int>,CocktailShakerSort<Counter>,CocktailShakerSort<Strip>),
				Sort(L"梳排序(ArrayV)",8192,CombSortJava<int>,CombSortJava<Counter>,CombSortJava<Strip>),
				Sort(L"Complete Graph Sort",1024,CompleteGraphSort<int>,CompleteGraphSort<Counter>,CompleteGraphSort<Strip>),
				Sort(L"双枢轴快速排序",8192,DualPivotQuickSort<int>,DualPivotQuickSort<Counter>,DualPivotQuickSort<Strip>),
				Sort(L"交换猴子排序",16,ExchangeBogoSort<int>,ExchangeBogoSort<Counter>,ExchangeBogoSort<Strip>,{},true,false),
				Sort(L"强制稳定快速排序",1024,ForcedStableQuickSort<int>,ForcedStableQuickSort<Counter>,ForcedStableQuickSort<Strip>),
				Sort(L"Fun Sort",64,FunSort<int>,FunSort<Counter>,FunSort<Strip>,{},true,false),
				Sort(L"地精排序",256,GnomeSort<int>,GnomeSort<Counter>,GnomeSort<Strip>),
				Sort(L"左左指针快速排序",8192,LLQuickSort<int>,LLQuickSort<Counter>,LLQuickSort<Strip>),
				Sort(L"左右指针快速排序",8192,LRQuickSort<int>,LRQuickSort<Counter>,LRQuickSort<Strip>),
				Sort(L"左右指针快速排序(并行)",8192,LRQuickSortParallel<int>,LRQuickSortParallel<Counter>,LRQuickSortParallel<Strip>,{},false,true),
				Sort(L"奇偶排序(ArrayV)",256,OddEvenSortJava<int>,OddEvenSortJava<Counter>,OddEvenSortJava<Strip>),
				Sort(L"冒泡排序(优化)",256,OptimizedBubbleSort<int>,OptimizedBubbleSort<Counter>,OptimizedBubbleSort<Strip>),
				Sort(L"鸡尾酒排序(优化)",256,OptimizedCocktailShakerSort<int>,OptimizedCocktailShakerSort<Counter>,OptimizedCocktailShakerSort<Strip>),
				Sort(L"地精排序(优化)",256,OptimizedGnomeSort<int>,OptimizedGnomeSort<Counter>,OptimizedGnomeSort<Strip>),
				Sort(L"臭皮匠排序(优化)",256,OptimizedStoogeSort<int>,OptimizedStoogeSort<Counter>,OptimizedStoogeSort<Strip>),
				Sort(L"臭皮匠排序(Studio)",256,OptimizedStoogeSortStudio<int>,OptimizedStoogeSortStudio<Counter>,OptimizedStoogeSortStudio<Strip>),
				Sort(L"Quad Stooge Sort",256,QuadStoogeSort<int>,QuadStoogeSort<Counter>,QuadStoogeSort<Strip>),
				Sort(L"Shove Sort",256,ShoveSort<int>,ShoveSort<Counter>,ShoveSort<Strip>),
				Sort(L"Silly Sort",128,SillySort<int>,SillySort<Counter>,SillySort<Strip>),
				Sort(L"Slope Sort",256,SlopeSort<int>,SlopeSort<Counter>,SlopeSort<Strip>),
				Sort(L"Slow Sort",64,SlowSort<int>,SlowSort<Counter>,SlowSort<Strip>),
				Sort(L"Snuffle Sort",64,SnuffleSort<int>,SnuffleSort<Counter>,SnuffleSort<Strip>),
				Sort(L"稳定排列排序",9,StablePermutationSort<int>,StablePermutationSort<Counter>,StablePermutationSort<Strip>,{},true,false),
				Sort(L"稳定快速排序",1024,StableQuickSort<int>,StableQuickSort<Counter>,StableQuickSort<Strip>),
				Sort(L"稳定快速排序(并行)",8192,StableQuickSortParallel<int>,StableQuickSortParallel<Counter>,StableQuickSortParallel<Strip>,{},false,true),
				Sort(L"臭皮匠排序(ArrayV)",256,StoogeSortJava<int>,StoogeSortJava<Counter>,StoogeSortJava<Strip>),
				Sort(L"无交换冒泡排序",256,SwaplessBubbleSort<int>,SwaplessBubbleSort<Counter>,SwaplessBubbleSort<Strip>),
				Sort(L"表排序",1024,TableSort<int>,TableSort<Counter>,TableSort<Strip>),
				Sort(L"三平滑梳排序(迭代)",1024,ThreeSmoothCombSortIterative<int>,ThreeSmoothCombSortIterative<Counter>,ThreeSmoothCombSortIterative<Strip>),
				Sort(L"三平滑梳排序(并行)",1024,ThreeSmoothCombSortParallel<int>,ThreeSmoothCombSortParallel<Counter>,ThreeSmoothCombSortParallel<Strip>,{},false,true),
				Sort(L"三平滑梳排序(递归)",1024,ThreeSmoothCombSortRecursive<int>,ThreeSmoothCombSortRecursive<Counter>,ThreeSmoothCombSortRecursive<Strip>),
				Sort(L"冒泡排序(未优化)",256,UnoptimizedBubbleSort<int>,UnoptimizedBubbleSort<Counter>,UnoptimizedBubbleSort<Strip>),
				Sort(L"鸡尾酒排序(未优化)",256,UnoptimizedCocktailShakerSort<int>,UnoptimizedCocktailShakerSort<Counter>,UnoptimizedCocktailShakerSort<Strip>),
				Sort(L"异步排序",64,AsynchronousSort<int>,AsynchronousSort<Counter>,AsynchronousSort<Strip>,{},true,false),
				Sort(L"Bad Sort",32,BadSort<int>,BadSort<Counter>,BadSort<Strip>,{},true,false),
				Sort(L"Base-N Max Heap",8192,BaseNMaxHeapSort<int>,BaseNMaxHeapSort<Counter>,BaseNMaxHeapSort<Strip>),
				Sort(L"宾果排序",256,BingoSort<int>,BingoSort<Counter>,BingoSort<Strip>),
				Sort(L"二项堆排序",8192,BinomialHeapSort<int>,BinomialHeapSort<Counter>,BinomialHeapSort<Strip>),
				Sort(L"二项平滑排序",8192,BinomialSmoothSort<int>,BinomialSmoothSort<Counter>,BinomialSmoothSort<Strip>),
				Sort(L"自底向上堆排序",8192,BottomUpHeapSort<int>,BottomUpHeapSort<Counter>,BottomUpHeapSort<Strip>),
				Sort(L"经典锦标赛排序",8192,ClassicTournamentSort<int>,ClassicTournamentSort<Counter>,ClassicTournamentSort<Strip>),
				Sort(L"循环排序(ArrayV)",256,CycleSortJava<int>,CycleSortJava<Counter>,CycleSortJava<Strip>),
				Sort(L"双端选择排序",256,DoubleSelectionSort<int>,DoubleSelectionSort<Counter>,DoubleSelectionSort<Strip>),
				Sort(L"翻转最小堆排序",8192,FlippedMinHeapSort<int>,FlippedMinHeapSort<Counter>,FlippedMinHeapSort<Strip>),
				Sort(L"惰性堆排序",1024,LazyHeapSort<int>,LazyHeapSort<Counter>,LazyHeapSort<Strip>),
				Sort(L"最大堆排序",8192,MaxHeapSort<int>,MaxHeapSort<Counter>,MaxHeapSort<Strip>),
				Sort(L"最小堆排序",8192,MinHeapSort<int>,MinHeapSort<Counter>,MinHeapSort<Strip>),
				Sort(L"最小最大堆排序",8192,MinMaxHeapSort<int>,MinMaxHeapSort<Counter>,MinMaxHeapSort<Strip>),
				Sort(L"非原地堆排序",8192,OutOfPlaceHeapSort<int>,OutOfPlaceHeapSort<Counter>,OutOfPlaceHeapSort<Strip>),
				Sort(L"Poplar Heap Sort",8192,PoplarHeapSort<int>,PoplarHeapSort<Counter>,PoplarHeapSort<Strip>),
				Sort(L"选择排序(ArrayV)",256,SelectionSortJava<int>,SelectionSortJava<Counter>,SelectionSortJava<Strip>),
				Sort(L"平滑排序",8192,SmoothSort<int>,SmoothSort<Counter>,SmoothSort<Strip>),
				Sort(L"稳定循环排序",256,StableCycleSort<int>,StableCycleSort<Counter>,StableCycleSort<Strip>),
				Sort(L"稳定选择排序",256,StableSelectionSort<int>,StableSelectionSort<Counter>,StableSelectionSort<Strip>),
				Sort(L"三叉堆排序",8192,TernaryHeapSort<int>,TernaryHeapSort<Counter>,TernaryHeapSort<Strip>),
				Sort(L"锦标赛排序",8192,TournamentSort<int>,TournamentSort<Counter>,TournamentSort<Strip>),
				Sort(L"三角堆排序",8192,TriangularHeapSort<int>,TriangularHeapSort<Counter>,TriangularHeapSort<Strip>),
				Sort(L"弱堆排序",8192,WeakHeapSort<int>,WeakHeapSort<Counter>,WeakHeapSort<Strip>),
				Sort(L"插入排序(ArrayV)",256,InsertionSortJava<int>,InsertionSortJava<Counter>,InsertionSortJava<Strip>),
				Sort(L"二分插入排序",256,BinaryInsertionSort<int>,BinaryInsertionSort<Counter>,BinaryInsertionSort<Strip>),
				Sort(L"双重插入排序",256,DoubleInsertionSort<int>,DoubleInsertionSort<Counter>,DoubleInsertionSort<Strip>),
				Sort(L"二分双重插入排序",256,BinaryDoubleInsertionSort<int>,BinaryDoubleInsertionSort<Counter>,BinaryDoubleInsertionSort<Strip>),
				Sort(L"块插入排序",1024,BlockInsertionSort<int>,BlockInsertionSort<Counter>,BlockInsertionSort<Strip>),
				Sort(L"希尔排序(ArrayV)",8192,ShellSortJava<int>,ShellSortJava<Counter>,ShellSortJava<Strip>),
				Sort(L"递归希尔排序",8192,RecursiveShellSort<int>,RecursiveShellSort<Counter>,RecursiveShellSort<Strip>),
				Sort(L"希尔排序(并行)",8192,ShellSortParallel<int>,ShellSortParallel<Counter>,ShellSortParallel<Strip>,{},false,true),
				Sort(L"图书馆排序",1024,LibrarySort<int>,LibrarySort<Counter>,LibrarySort<Strip>),
				Sort(L"简易图书馆排序",1024,SimplifiedLibrarySort<int>,SimplifiedLibrarySort<Counter>,SimplifiedLibrarySort<Strip>),
				Sort(L"耐心排序",8192,PatienceSort<int>,PatienceSort<Counter>,PatienceSort<Strip>),
				Sort(L"经典树排序",256,ClassicTreeSort<int>,ClassicTreeSort<Counter>,ClassicTreeSort<Strip>),
				Sort(L"树排序",256,TreeSort<int>,TreeSort<Counter>,TreeSort<Strip>),
				Sort(L"AA树排序",8192,AATreeSort<int>,AATreeSort<Counter>,AATreeSort<Strip>),
				Sort(L"AVL树排序",8192,AVLTreeSort<int>,AVLTreeSort<Counter>,AVLTreeSort<Strip>),
				Sort(L"红黑树排序",8192,RedBlackTreeSort<int>,RedBlackTreeSort<Counter>,RedBlackTreeSort<Strip>),
				Sort(L"伸展树排序",8192,SplaySort<int>,SplaySort<Counter>,SplaySort<Strip>),
				Sort(L"汉诺塔排序",32,HanoiSort<int>,HanoiSort<Counter>,HanoiSort<Strip>),
				Sort(L"AndreySort",8192,AndreySort<int>,AndreySort<Counter>,AndreySort<Strip>),
				Sort(L"块交换归并排序",1024,BlockSwapMergeSort<int>,BlockSwapMergeSort<Counter>,BlockSwapMergeSort<Strip>),
				Sort(L"自底向上归并排序",8192,BottomUpMergeSort<int>,BottomUpMergeSort<Counter>,BottomUpMergeSort<Strip>),
				Sort(L"臭皮匠排序(缓冲)",256,BufferedStoogeSort<int>,BufferedStoogeSort<Counter>,BufferedStoogeSort<Strip>),
				Sort(L"原地归并排序(改进)",256,ImprovedInPlaceMergeSort<int>,ImprovedInPlaceMergeSort<Counter>,ImprovedInPlaceMergeSort<Strip>),
				Sort(L"原地归并排序",256,InPlaceMergeSort<int>,InPlaceMergeSort<Counter>,InPlaceMergeSort<Strip>),
				Sort(L"迭代归并排序",8192,IterativeTopDownMergeSort<int>,IterativeTopDownMergeSort<Counter>,IterativeTopDownMergeSort<Strip>),
				Sort(L"LazyStableSort",1024,LazyStableSort<int>,LazyStableSort<Counter>,LazyStableSort<Strip>),
				Sort(L"归并排序(ArrayV)",8192,MergeSortJava<int>,MergeSortJava<Counter>,MergeSortJava<Strip>),
				Sort(L"归并排序(并行)",8192,MergeSortParallel<int>,MergeSortParallel<Counter>,MergeSortParallel<Strip>,{},false,true),
				Sort(L"NewShuffleMergeSort",1024,NewShuffleMergeSort<int>,NewShuffleMergeSort<Counter>,NewShuffleMergeSort<Strip>),
				Sort(L"PDMergeSort",8192,PDMergeSort<int>,PDMergeSort<Counter>,PDMergeSort<Strip>),
				Sort(L"QuadSort",8192,QuadSort<int>,QuadSort<Counter>,QuadSort<Strip>),
				Sort(L"旋转归并排序",1024,RotateMergeSort<int>,RotateMergeSort<Counter>,RotateMergeSort<Strip>),
				Sort(L"旋转归并排序(并行)",1024,RotateMergeSortParallel<int>,RotateMergeSortParallel<Counter>,RotateMergeSortParallel<Strip>,{},false,true),
				Sort(L"无栈旋转归并排序",1024,StacklessRotateMergeSort<int>,StacklessRotateMergeSort<Counter>,StacklessRotateMergeSort<Strip>),
				Sort(L"StrandSort",256,StrandSort<int>,StrandSort<Counter>,StrandSort<Strip>),
				Sort(L"TwinSort",8192,TwinSort<int>,TwinSort<Counter>,TwinSort<Strip>),
				Sort(L"WeavedMergeSort",8192,WeavedMergeSort<int>,WeavedMergeSort<Counter>,WeavedMergeSort<Strip>),
				Sort(L"美式国旗排序",32768,AmericanFlagSort<int>,AmericanFlagSort<Counter>,AmericanFlagSort<Strip>),
				Sort(L"二分快速排序(迭代)",32768,BinaryQuickSortIterative<int>,BinaryQuickSortIterative<Counter>,BinaryQuickSortIterative<Strip>),
				Sort(L"二分快速排序(递归)",32768,BinaryQuickSortRecursive<int>,BinaryQuickSortRecursive<Counter>,BinaryQuickSortRecursive<Strip>),
				Sort(L"乱序冒泡(Bogo Bogo)",5,BogoBogoSort<int>,BogoBogoSort<Counter>,BogoBogoSort<Strip>,{},true,false),
				Sort(L"乱序冒泡(ArrayV)",10,BogoSortJava<int>,BogoSortJava<Counter>,BogoSortJava<Strip>,{},true,false),
				Sort(L"乱序交换",11,BozoSort<int>,BozoSort<Counter>,BozoSort<Strip>,{},true,false),
				Sort(L"经典重力排序",256,ClassicGravitySort<int>,ClassicGravitySort<Counter>,ClassicGravitySort<Strip>),
				Sort(L"乱序冒泡(双向)",32,CocktailBogoSort<int>,CocktailBogoSort<Counter>,CocktailBogoSort<Strip>,{},true,false),
				Sort(L"计数排序(ArrayV)",32768,CountingSortJava<int>,CountingSortJava<Counter>,CountingSortJava<Strip>),
				Sort(L"乱序冒泡(全排列)",11,DeterministicBogoSort<int>,DeterministicBogoSort<Counter>,DeterministicBogoSort<Strip>,{},true,false),
				Sort(L"闪电排序",32768,FlashSort<int>,FlashSort<Counter>,FlashSort<Strip>),
				Sort(L"重力排序",256,GravitySort<int>,GravitySort<Counter>,GravitySort<Strip>),
				Sort(L"猜测排序",7,GuessSort<int>,GuessSort<Counter>,GuessSort<Strip>,{},true,false),
				Sort(L"原地LSD基数排序",256,InPlaceLSDRadixSort<int>,InPlaceLSDRadixSort<Counter>,InPlaceLSDRadixSort<Strip>),
				Sort(L"索引排序",32768,IndexSort<int>,IndexSort<Counter>,IndexSort<Strip>),
				Sort(L"LSD基数排序",32768,LSDRadixSort<int>,LSDRadixSort<Counter>,LSDRadixSort<Strip>),
				Sort(L"乱序冒泡(递减)",32,LessBogoSort<int>,LessBogoSort<Counter>,LessBogoSort<Strip>,{},true,false),
				Sort(L"MSD基数排序",32768,MSDRadixSort<int>,MSDRadixSort<Counter>,MSDRadixSort<Strip>),
				Sort(L"乱序快速(中位)",23,MedianQuickBogoSort<int>,MedianQuickBogoSort<Counter>,MedianQuickBogoSort<Strip>,{},true,false),
				Sort(L"乱序归并",22,MergeBogoSort<int>,MergeBogoSort<Counter>,MergeBogoSort<Strip>,{},true,false),
				Sort(L"猜测排序(优化)",8,OptimizedGuessSort<int>,OptimizedGuessSort<Counter>,OptimizedGuessSort<Strip>,{},true,false),
				Sort(L"鸽巢排序",32768,PigeonholeSort<int>,PigeonholeSort<Counter>,PigeonholeSort<Strip>),
				Sort(L"乱序快速",22,QuickBogoSort<int>,QuickBogoSort<Counter>,QuickBogoSort<Strip>,{},true,false),
				Sort(L"猜测排序(随机)",8,RandomGuessSort<int>,RandomGuessSort<Counter>,RandomGuessSort<Strip>,{},true,false),
				Sort(L"旋转LSD基数排序",32768,RotateLSDRadixSort<int>,RotateLSDRadixSort<Counter>,RotateLSDRadixSort<Strip>),
				Sort(L"旋转MSD基数排序",32768,RotateMSDRadixSort<int>,RotateMSDRadixSort<Counter>,RotateMSDRadixSort<Strip>),
				Sort(L"乱序选择",32,SelectionBogoSort<int>,SelectionBogoSort<Counter>,SelectionBogoSort<Strip>,{},true,false),
				Sort(L"Shatter Sort",32768,ShatterSort<int>,ShatterSort<Counter>,ShatterSort<Strip>),
				Sort(L"Simple Shatter Sort",32768,SimpleShatterSort<int>,SimpleShatterSort<Counter>,SimpleShatterSort<Strip>),
				Sort(L"简易重力排序",256,SimplisticGravitySort<int>,SimplisticGravitySort<Counter>,SimplisticGravitySort<Strip>),
				Sort(L"乱序冒泡(智能)",11,SmartBogoBogoSort<int>,SmartBogoBogoSort<Counter>,SmartBogoBogoSort<Strip>,{},true,false),
				Sort(L"猜测排序(智能)",19,SmartGuessSort<int>,SmartGuessSort<Counter>,SmartGuessSort<Strip>,{},true,false),
				Sort(L"无栈美式国旗排序",32768,StacklessAmericanFlagSort<int>,StacklessAmericanFlagSort<Counter>,StacklessAmericanFlagSort<Strip>),
				Sort(L"无栈二分快速排序",32768,StacklessBinaryQuickSort<int>,StacklessBinaryQuickSort<Counter>,StacklessBinaryQuickSort<Strip>),
				Sort(L"静态排序",32768,StaticSort<int>,StaticSort<Counter>,StaticSort<Strip>),
				Sort(L"时间排序",8,TimeSort<int>,TimeSort<Counter>,TimeSort<Strip>,{},true,true),
				Sort(L"Adaptive Grail Sort",8192,AdaptiveGrailSort<int>,AdaptiveGrailSort<Counter>,AdaptiveGrailSort<Strip>),
				Sort(L"二分归并排序",8192,BinaryMergeSort<int>,BinaryMergeSort<Counter>,BinaryMergeSort<Strip>),
				Sort(L"Buffer Partition Merge",1024,BufferPartitionMergeSort<int>,BufferPartitionMergeSort<Counter>,BufferPartitionMergeSort<Strip>),
				Sort(L"Chalice Sort",1024,ChaliceSort<int>,ChaliceSort<Counter>,ChaliceSort<Strip>),
				Sort(L"Circular Grail Sort",8192,CircularGrailSort<int>,CircularGrailSort<Counter>,CircularGrailSort<Strip>),
				Sort(L"Cocktail Merge Sort",8192,CocktailMergeSort<int>,CocktailMergeSort<Counter>,CocktailMergeSort<Strip>),
				Sort(L"Drop Merge Sort",8192,DropMergeSort<int>,DropMergeSort<Counter>,DropMergeSort<Strip>),
				Sort(L"Ecta Sort",8192,EctaSort<int>,EctaSort<Counter>,EctaSort<Strip>),
				Sort(L"Fifth Merge Sort",1024,FifthMergeSort<int>,FifthMergeSort<Counter>,FifthMergeSort<Strip>),
				Sort(L"Flan Sort",1024,FlanSort<int>,FlanSort<Counter>,FlanSort<Strip>),
				Sort(L"Flux Sort",8192,FluxSort<int>,FluxSort<Counter>,FluxSort<Strip>),
				Sort(L"Grail Sort",8192,GrailSort<int>,GrailSort<Counter>,GrailSort<Strip>),
				Sort(L"混合梳排序",8192,HybridCombSort<int>,HybridCombSort<Counter>,HybridCombSort<Strip>),
				Sort(L"Improved Block Selection",1024,ImprovedBlockSelectionSort<int>,ImprovedBlockSelectionSort<Counter>,ImprovedBlockSelectionSort<Strip>),
				Sort(L"Intro Circle Sort (迭代)",1024,IntroCircleSortIterative<int>,IntroCircleSortIterative<Counter>,IntroCircleSortIterative<Strip>),
				Sort(L"Intro Circle Sort (递归)",1024,IntroCircleSortRecursive<int>,IntroCircleSortRecursive<Counter>,IntroCircleSortRecursive<Strip>),
				Sort(L"内省排序",8192,IntroSort<int>,IntroSort<Counter>,IntroSort<Strip>),
				Sort(L"Kota Sort",8192,KotaSort<int>,KotaSort<Counter>,KotaSort<Strip>),
				Sort(L"Lazierest Sort",1024,LazierestSort<int>,LazierestSort<Counter>,LazierestSort<Strip>),
				Sort(L"Laziest Sort",1024,LaziestSort<int>,LaziestSort<Counter>,LaziestSort<Strip>),
				Sort(L"Median Merge Sort",8192,MedianMergeSort<int>,MedianMergeSort<Counter>,MedianMergeSort<Strip>),
				Sort(L"归并插入排序",1024,MergeInsertionSort<int>,MergeInsertionSort<Counter>,MergeInsertionSort<Strip>),
				Sort(L"优化自底向上归并",8192,OptimizedBottomUpMergeSort<int>,OptimizedBottomUpMergeSort<Counter>,OptimizedBottomUpMergeSort<Strip>),
				Sort(L"优化双枢轴快速排序",8192,OptimizedDualPivotQuickSort<int>,OptimizedDualPivotQuickSort<Counter>,OptimizedDualPivotQuickSort<Strip>),
				Sort(L"Optimized Lazy Stable",8192,OptimizedLazyStableSort<int>,OptimizedLazyStableSort<Counter>,OptimizedLazyStableSort<Strip>),
				Sort(L"Optimized Rotate Merge",1024,OptimizedRotateMergeSort<int>,OptimizedRotateMergeSort<Counter>,OptimizedRotateMergeSort<Strip>),
				Sort(L"Weave Merge Sort (优化)",8192,OptimizedWeaveMergeSort<int>,OptimizedWeaveMergeSort<Counter>,OptimizedWeaveMergeSort<Strip>),
				Sort(L"PDQ Sort",8192,PDQBranchedSort<int>,PDQBranchedSort<Counter>,PDQBranchedSort<Strip>),
				Sort(L"PDQ Sort (无分支)",8192,PDQBranchlessSort<int>,PDQBranchlessSort<Counter>,PDQBranchlessSort<Strip>),
				Sort(L"块归并排序(并行)",8192,ParallelBlockMergeSort<int>,ParallelBlockMergeSort<Counter>,ParallelBlockMergeSort<Strip>,{},false,true),
				Sort(L"Grail Sort (并行)",8192,ParallelGrailSort<int>,ParallelGrailSort<Counter>,ParallelGrailSort<Strip>,{},false,true),
				Sort(L"Remi Sort",1024,RemiSort<int>,RemiSort<Counter>,RemiSort<Strip>),
				Sort(L"Sqrt Sort",1024,SqrtSort<int>,SqrtSort<Counter>,SqrtSort<Strip>),
				Sort(L"无栈双枢轴快速排序",8192,StacklessDualPivotQuickSort<int>,StacklessDualPivotQuickSort<Counter>,StacklessDualPivotQuickSort<Strip>),
				Sort(L"无栈混合快速排序",8192,StacklessHybridQuickSort<int>,StacklessHybridQuickSort<Counter>,StacklessHybridQuickSort<Strip>),
				Sort(L"Synchronous Sqrt Sort",8192,SynchronousSqrtSort<int>,SynchronousSqrtSort<Counter>,SynchronousSqrtSort<Strip>),
				Sort(L"Tim Sort",8192,TimSort<int>,TimSort<Counter>,TimSort<Strip>),
				Sort(L"Unstable Grail Sort",8192,UnstableGrailSort<int>,UnstableGrailSort<Counter>,UnstableGrailSort<Strip>),
				Sort(L"Weave Merge Sort",8192,WeaveMergeSort<int>,WeaveMergeSort<Counter>,WeaveMergeSort<Strip>),
				Sort(L"Wiki Sort",8192,WikiSort<int>,WikiSort<Counter>,WikiSort<Strip>),
				Sort(L"Yuji's Buffered Merge 2",8192,YujisBufferedMergeSort2<int>,YujisBufferedMergeSort2<Counter>,YujisBufferedMergeSort2<Strip>),
				Sort(L"双调排序(迭代)",1024,BitonicSortIterative<int>,BitonicSortIterative<Counter>,BitonicSortIterative<Strip>),
				Sort(L"双调排序(并行)",1024,BitonicSortParallel<int>,BitonicSortParallel<Counter>,BitonicSortParallel<Strip>,
					{{L"需要数据规模为2的幂次方",[](size_t data_size_)->bool { return (data_size_ & (data_size_ - 1)) == 0 && data_size_ > 0; }}},false,true),
				Sort(L"双调排序(递归)",1024,BitonicSortRecursive<int>,BitonicSortRecursive<Counter>,BitonicSortRecursive<Strip>),
				Sort(L"Bose-Nelson排序(迭代)",1024,BoseNelsonSortIterative<int>,BoseNelsonSortIterative<Counter>,BoseNelsonSortIterative<Strip>),
				Sort(L"Bose-Nelson排序(并行)",1024,BoseNelsonSortParallel<int>,BoseNelsonSortParallel<Counter>,BoseNelsonSortParallel<Strip>,{},false,true),
				Sort(L"Bose-Nelson排序(递归)",1024,BoseNelsonSortRecursive<int>,BoseNelsonSortRecursive<Counter>,BoseNelsonSortRecursive<Strip>),
				Sort(L"折痕排序",1024,CreaseSort<int>,CreaseSort<Counter>,CreaseSort<Strip>),
				Sort(L"钻石排序(迭代)",1024,DiamondSortIterative<int>,DiamondSortIterative<Counter>,DiamondSortIterative<Strip>),
				Sort(L"钻石排序(递归)",1024,DiamondSortRecursive<int>,DiamondSortRecursive<Counter>,DiamondSortRecursive<Strip>,
					{{L"需要数据规模为2的幂次方",[](size_t data_size_)->bool { return (data_size_ & (data_size_ - 1)) == 0 && data_size_ > 0; }}}),
				Sort(L"折叠排序",1024,FoldSort<int>,FoldSort<Counter>,FoldSort<Strip>),
				Sort(L"矩阵排序",1024,MatrixSort<int>,MatrixSort<Counter>,MatrixSort<Strip>),
				Sort(L"归并交换排序(迭代)",1024,MergeExchangeSortIterative<int>,MergeExchangeSortIterative<Counter>,MergeExchangeSortIterative<Strip>),
				Sort(L"奇偶归并排序(迭代)",1024,OddEvenMergeSortIterative<int>,OddEvenMergeSortIterative<Counter>,OddEvenMergeSortIterative<Strip>),
				Sort(L"奇偶归并排序(并行)",1024,OddEvenMergeSortParallel<int>,OddEvenMergeSortParallel<Counter>,OddEvenMergeSortParallel<Strip>,{},false,true),
				Sort(L"奇偶归并排序(递归)",1024,OddEvenMergeSortRecursive<int>,OddEvenMergeSortRecursive<Counter>,OddEvenMergeSortRecursive<Strip>),
				Sort(L"Pairwise Merge排序(迭代)",1024,PairwiseMergeSortIterative<int>,PairwiseMergeSortIterative<Counter>,PairwiseMergeSortIterative<Strip>),
				Sort(L"Pairwise Merge排序(递归)",1024,PairwiseMergeSortRecursive<int>,PairwiseMergeSortRecursive<Counter>,PairwiseMergeSortRecursive<Strip>),
				Sort(L"Pairwise排序(迭代)",1024,PairwiseSortIterative<int>,PairwiseSortIterative<Counter>,PairwiseSortIterative<Strip>),
				Sort(L"Pairwise排序(递归)",1024,PairwiseSortRecursive<int>,PairwiseSortRecursive<Counter>,PairwiseSortRecursive<Strip>),
				Sort(L"编织排序(迭代)",1024,WeaveSortIterative<int>,WeaveSortIterative<Counter>,WeaveSortIterative<Strip>),
				Sort(L"编织排序(并行)",1024,WeaveSortParallel<int>,WeaveSortParallel<Counter>,WeaveSortParallel<Strip>,
					{{L"需要数据规模为2的幂次方",[](size_t data_size_)->bool { return (data_size_ & (data_size_ - 1)) == 0 && data_size_ > 0; }}},false,true),
				Sort(L"编织排序(递归)",1024,WeaveSortRecursive<int>,WeaveSortRecursive<Counter>,WeaveSortRecursive<Strip>),
				Sort(L"煎饼排序",256,PancakeSort<int>,PancakeSort<Counter>,PancakeSort<Strip>),
				Sort(L"烧焦煎饼排序",256,BurntPancakeSort<int>,BurntPancakeSort<Counter>,BurntPancakeSort<Strip>),
				Sort(L"煎饼插入排序",256,PancakeInsertionSort<int>,PancakeInsertionSort<Counter>,PancakeInsertionSort<Strip>),
				Sort(L"斯大林排序",256,StalinSort<int>,StalinSort<Counter>,StalinSort<Strip>),
				Sort(L"三元快速排序(LL)",8192,TernaryLLQuickSort<int>,TernaryLLQuickSort<Counter>,TernaryLLQuickSort<Strip>),
				Sort(L"三元快速排序(LR)",8192,TernaryLRQuickSort<int>,TernaryLRQuickSort<Counter>,TernaryLRQuickSort<Strip>)
			};
		}
		VisualSort(const VisualSort&) = delete;
		VisualSort(VisualSort&&) = delete;
		VisualSort& operator = (const VisualSort&) = delete;
		VisualSort& operator = (VisualSort&&) = delete;

		std::function<void()> GetSleepFunc() {
			return [this]() {
				static thread_local Fraction accum = 0;
				std::shared_lock lock(this->m_speedMutex);
				static thread_local Fraction lastSpeed = this->m_displaySpeed;
				Fraction currSpeed = this->m_displaySpeed;
				lock.unlock();
				if (lastSpeed != currSpeed) {
					accum = 0;
					lastSpeed = currSpeed;
				}
				accum += currSpeed.Reciprocal();
				while (accum >= 1) {
					long long ms = static_cast<long long>(accum);
					std::this_thread::sleep_for(std::chrono::milliseconds(ms));
					accum -= ms;
				}
				};
		}

		std::function<void(RECT, COLORREF)> GetDrawFunc() {
			if (this->m_sourceData.size() * 6 > static_cast<size_t>(GetConfigManager().GetWidth())) {
				return [](RECT rect_, COLORREF color_) {
					GetDrawingTool().SolidRectangle(rect_, color_);
					};
			}
			else {
				return [](RECT rect_, COLORREF color_) {
					GetDrawingTool().FillRectangle(rect_, 1, PS_SOLID, BLACK, color_);
				};
			}
		}

		std::atomic<size_t> m_updateMessageTime;
		inline static constexpr size_t UpdateMessageGap = 10;

		std::function<void()> GetUpdateMessageFunc() {
			return [this]() {
				std::unique_lock lock(Strip::s_threadsMutex);
				size_t stripThreadNum = Strip::s_threads.size();
				lock.unlock();
				if ((++this->m_updateMessageTime) % (VisualSort::UpdateMessageGap * stripThreadNum) != 0) {
					return;
				}
				long long stripSortTime = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()
					- this->m_stripSortBeginTime)).count() - Strip::s_stripSortStopTime.count();
				bool notShowProgress = this->m_sorts[this->m_sortIndex.value()].GetIsUnpredictable();
				this->m_messages[1].SetTextWithoutResize(
					L"演示时间：" + std::to_wstring(stripSortTime / 1000) + L"." +
					std::to_wstring((stripSortTime % 1000) / 100) + (notShowProgress ? L"s 排序时间" : L"s 排序进度：") +
					std::to_wstring(AnimationStepNum * this->m_intSortDuration.count() / ActualStepNum)
					+ (notShowProgress ? L"us" : (L"us/" + std::to_wstring(this->m_intSortDuration.count()) + L"us = " +
						std::to_wstring(AnimationStepNum * 100 / ActualStepNum) + L"." +
						std::to_wstring((AnimationStepNum * 1000 / ActualStepNum) % 10) + L"%"))
				);
				this->m_messages[1].DrawSketch(false);
				this->m_messages[2].SetTextWithoutResize(
					L"样本比较：" + std::to_wstring(StripCompareNum) + L"次 " +
					L"样本引用：" + std::to_wstring(StripCopyNum) + L"次 " +
					L"样本修改：" + std::to_wstring(StripChangeNum) + L"次");
				this->m_messages[2].DrawSketch(false);
				GetDrawingTool().FlushBatchDraw(0, this->m_messages[1].GetTop(), GetConfigManager().GetWidth(), this->m_messages[2].GetBottom());
				};
		}

		void UpdateLastMessage() {
			long long stripSortTime = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()
				- this->m_stripSortBeginTime)).count() - Strip::s_stripSortStopTime.count();
			bool notShowProgress = this->m_sorts[this->m_sortIndex.value()].GetIsUnpredictable();
			this->m_messages[1].SetTextWithoutResize(
				L"演示时间：" + std::to_wstring(stripSortTime / 1000) + L"." +
				std::to_wstring((stripSortTime % 1000) / 100) + (notShowProgress ? L"s 排序时间" : L"s 排序进度：") +
				std::to_wstring(AnimationStepNum * this->m_intSortDuration.count() / ActualStepNum)
				+ (notShowProgress ? L"us" : (L"us/" + std::to_wstring(this->m_intSortDuration.count()) + L"us = " +
					std::to_wstring(AnimationStepNum * 100 / ActualStepNum) + L"." +
					std::to_wstring((AnimationStepNum * 1000 / ActualStepNum) % 10) + L"%"))
			);
			this->m_messages[1].DrawSketch();
			this->m_messages[2].SetTextWithoutResize(
				L"样本比较：" + std::to_wstring(StripCompareNum) + L"次 " +
				L"样本引用：" + std::to_wstring(StripCopyNum) + L"次 " +
				L"样本修改：" + std::to_wstring(StripChangeNum) + L"次");
			this->m_messages[2].DrawSketch();
		}

		void SetMessageAuto() {
			this->m_messages.resize(3);
			Sketch& titleSketch = this->m_messages[0];
			titleSketch.SetSketch(0, 0, GetConfigManager().GetWidth(), Strip::StripMaxTop() / 4,
				this->m_sorts[this->m_sortIndex.value()].GetSortName() + L" 样本大小：" + std::to_wstring(this->m_sourceData.size()));
			GetDrawingTool().ExecuteWithLock([&titleSketch]() {
				::settextstyle(titleSketch.GetTextSize(), 0, titleSketch.GetTypeface().c_str());
				titleSketch.SetRightWithoutResize(::textwidth(titleSketch.GetText().c_str()) + (std::min)(titleSketch.GetHeight(), titleSketch.GetRight()) / 20);
				});
			titleSketch.SetHasFrame(false).SetTextMode(DT_LEFT);

			Sketch& timeSketch = this->m_messages[1];
			timeSketch.SetHasFrame(false).SetTextMode(DT_LEFT).SetSketch(0, titleSketch.GetBottom(),
				GetConfigManager().GetWidth(), Strip::StripMaxTop() / 2, L"演示时间：0.0s 排序" +
				std::wstring(this->m_sorts[this->m_sortIndex.value()].GetIsUnpredictable() ? L"时间：0us" : (L"进度：0us/" + std::to_wstring(this->m_intSortDuration.count()) + L"us = 0%")));

			Sketch& countSketch = this->m_messages[2];
			countSketch.SetHasFrame(false).SetTextMode(DT_LEFT).SetSketch(0, timeSketch.GetBottom(),
				GetConfigManager().GetWidth(), Strip::StripMaxTop() * 3 / 4, L"样本比较：0次 样本引用：0次 样本修改：0次");
		}

		void SetControlButtonsAuto() {
			this->m_controlButtons.Clear();
			this->m_controlButtons.GetButtons().resize(2);
			this->m_controlButtons.GetButtons()[0].SetButton(GetConfigManager().GetWidth() * 15 / 16, 0, GetConfigManager().GetWidth(), this->m_messages[0].GetBottom(), L"暂停",
				[](Button& button_, ExMessage) {
					if (Strip::s_stopStripSort.load(std::memory_order_acquire)) {
						Strip::s_stopStripSort.store(false, std::memory_order_release);
						button_.GetSketch().SetTextWithoutResize(L"暂停");
					}
					else {
						Strip::s_stopStripSort.store(true, std::memory_order_release);
						button_.GetSketch().SetTextWithoutResize(L"继续");
					}
					Button::GetDefaultHoverDrawFunction()(button_, {});
				}
			);
			this->m_controlButtons.GetButtons()[1].SetThumb(RECT(0, this->m_messages[2].GetBottom(), GetConfigManager().GetWidth(), Strip::StripMaxTop()),
				Fraction(log10(static_cast<double>(this->m_displaySpeed)) + 1) / 2, [this](Fraction frac_) -> std::wstring {
					Fraction tempSpeed(pow(10.0, 2 * frac_ - 1));
					std::unique_lock lock(this->m_speedMutex);
					this->m_displaySpeed = tempSpeed;
					lock.unlock();
					int speed = static_cast<int>(frac_ * 100);
					return L"演示速度：" + std::to_wstring(speed < 1 ? 1 : speed);
				});
			if (!this->m_sorts[this->m_sortIndex.value()].GetIsMulThread()) {
				this->m_controlButtons.GetButtons().emplace_back(GetConfigManager().GetWidth() * 7 / 8, 0, this->m_controlButtons.GetButtons()[0].GetSketch().GetLeft(), this->m_messages[0].GetBottom(), L"退出",
					[this](Button& button_, ExMessage) {
						Strip::s_exitStripSort.store(true, std::memory_order_release);
						this->m_controlButtons.SetExitFlag(true);
					}
				);
			}
		}

		void RunErrorWindow(const std::vector<std::wstring>& error_messages_) {
			static std::mutex errorWindowMutex;
			std::lock_guard<std::mutex> lock(errorWindowMutex);
			Dialog errorWindow(error_messages_);
			errorWindow.SetCrossAuto();
			errorWindow.RunBlockDialog();
		}

		bool RunIntSort() {
			try {
				this->m_intSortDuration = {};
				auto startTime = std::chrono::high_resolution_clock::now();
				this->m_sorts[this->m_sortIndex.value()].RunSort(this->m_intSortData);
				auto endTime = std::chrono::high_resolution_clock::now();
				this->m_intSortDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
			}
			catch (const WideError& errorMessage) {
				this->RunErrorWindow({ errorMessage.What() });
				return false;
			}
			return true;
		}

		bool RunCounterSort() {
			ActualStepNum = 0;
			try {
				this->m_sorts[this->m_sortIndex.value()].RunSort(this->m_counterSortData);
			}
			catch (const WideError& errorMessage) {
				this->RunErrorWindow({ errorMessage.What() });
				return false;
			}
			return true;
		}

		bool RunStripSort() {
			Strip::InitValues();
			if (!this->m_showShuffle) {
				GetDrawingTool().ClearDevice();
				Strip::DrawStrips(this->m_stripSortData);
				for (auto it = this->m_messages.begin(); it != this->m_messages.end(); ++it) {
					it->DrawSketch();
				}
				this->m_controlButtons.RunNonBlockButtonLoop();
			}
			this->m_updateMessageTime.store(0, std::memory_order_release);
			try {
				this->m_stripSortBeginTime = std::chrono::high_resolution_clock::now();
				this->m_sorts[this->m_sortIndex.value()].RunSort(this->m_stripSortData);
			}
			catch (const WideError& errorMessage) {
				if (errorMessage.What() != SortEndsPrematurely) {
					this->RunErrorWindow({ errorMessage.What() });
				}
				return false;
			}
			Strip::DrawRemainingStrip();
			this->UpdateLastMessage();
			return true;
		}

		bool CheckData() {
			bool isCorrect = true;
			try {
				ScopeGuard scopeGuard([this]() {
					this->m_controlButtons.SetExitFlag(true);
					});
				if (this->m_sourceData.size() != this->m_stripSortData.size()) {
					throw WideError(L"排序结果的样本大小不正确");
				}
				std::stable_sort(this->m_sourceData.begin(), this->m_sourceData.end());
				for (size_t i = 0; i < this->m_sourceData.size(); ++i) {
					if (this->m_stripSortData[i].GetValue() == this->m_sourceData[i]) {
						this->m_stripSortData[i].SetColor(GREEN);
						Strip::DrawCheckStrip(this->m_stripSortData[i], GREEN);
					}
					else {
						this->m_stripSortData[i].SetColor(RED);
						Strip::DrawCheckStrip(this->m_stripSortData[i], RED);
						isCorrect = false;
					}
				}
			}
			catch (const WideError& errorMessage) {
				if (errorMessage.What() != SortEndsPrematurely) {
					this->RunErrorWindow({ errorMessage.What() });
				}
				return false;
			}
			Sketch resultSketch(0, 0, GetConfigManager().GetWidth(), Strip::StripMaxTop() / 4);
			if (isCorrect) {
				resultSketch.SetText(this->m_sorts[this->m_sortIndex.value()].GetSortName() + L"正确！ 样本大小：" + std::to_wstring(this->m_sourceData.size()));
			}
			else {
				resultSketch.SetText(this->m_sorts[this->m_sortIndex.value()].GetSortName() + L"错误！ 样本大小：" + std::to_wstring(this->m_sourceData.size()));
			}
			GetDrawingTool().ExecuteWithLock([&resultSketch]() {
				::settextstyle(resultSketch.GetTextSize(), 0, resultSketch.GetTypeface().c_str());
				resultSketch.SetRightWithoutResize(::textwidth(resultSketch.GetText().c_str()) + (std::min)(resultSketch.GetHeight(), resultSketch.GetRight()) / 20);
				});
			resultSketch.SetHasFrame(false).SetTextMode(DT_LEFT);
			RECT tempRect = { 0,0,GetConfigManager().GetWidth(),
				this->m_controlButtons.GetButtons()[0].GetSketch().GetBottom() +
				this->m_controlButtons.GetButtons()[0].GetSketch().GetFrameThick() };
			GetDrawingTool().ClearRectangle(tempRect);
			resultSketch.DrawSketch(false);
			ButtonSequence exitButton(1);
			exitButton.SetButton(0, this->m_controlButtons.GetButtons()[0].GetSketch().GetFrameRect(), L"退出", [&exitButton](Button&, ExMessage) {
				exitButton.SetExitFlag(true);
				});
			exitButton.RunBlockButtonLoop();
			return isCorrect;
		}

	public:

		void SetInitDataFunc(const std::function<void(size_t, std::vector<int>&)>& init_data_func_) {
			this->m_initDataFunc = init_data_func_;
		}

		std::vector<Sort>& GetSorts() noexcept {
			return this->m_sorts;
		}

		constexpr bool GetShowShuffle() const noexcept {
			return this->m_showShuffle;
		}

		constexpr void SetShowShuffle(bool show_shuffle_) noexcept {
			this->m_showShuffle = show_shuffle_;
		}

		template<typename T>
		static void Shuffle(std::vector<T>& data_, unsigned int rand_device_) {
			std::mt19937 rnd(rand_device_);
			for (size_t i = 0; i < data_.size(); ++i) {
				size_t randNum = rnd() % (i + 1);
				std::swap(data_[i], data_[randNum]);
			}
		}

		// 返回值为数据大小是否满足要求，不是返回排序是否成功
		bool SortPreparation(size_t sort_index_, size_t data_size_) {
			if (sort_index_ >= this->m_sorts.size()) {
				throw WideError(L"找不到排序");
			}
			this->m_sortIndex = sort_index_;
			ScopeGuard sg([this]() {
				this->m_sortIndex = std::nullopt;
				});
			std::vector<std::wstring> errorMessages;
			if (data_size_ > this->m_sorts[sort_index_].GetMaxSize()) {
				errorMessages.emplace_back(L"数据量超过允许最大值");
			}
			for (size_t i = 0; i < this->m_sorts[sort_index_].GetNumRequires().size(); ++i) {
				if (!this->m_sorts[sort_index_].GetNumRequires()[i].Check(data_size_)) {
					errorMessages.emplace_back(this->m_sorts[sort_index_].GetNumRequires()[i].GetRequireInform());
				}
			}
			if (!errorMessages.empty()) {
				this->RunErrorWindow(errorMessages);
				return false;
			}
			this->m_initDataFunc(data_size_, this->m_sourceData);

			Sketch inSortingPrompt;
			inSortingPrompt.SetFrameRect(RECT{ 0,0,static_cast<int>(GetConfigManager().GetWidth()),static_cast<int>(GetConfigManager().GetHeight()) }).
				SetText(this->m_sorts[sort_index_].GetSortName() + L"准备中，请稍候...").
				SetTextSize((std::min)(GetConfigManager().GetWidth() / 34, GetConfigManager().GetHeight() / 21)).
				SetHasBackground(false).SetHasFrame(false);
			GetDrawingTool().ClearDevice();
			inSortingPrompt.DrawSketch();

			if (!this->m_showShuffle) {
				VisualSort::Shuffle(this->m_sourceData, GetConfigManager().GenerateRandom());
				this->m_intSortData = this->m_sourceData;
				if (!this->RunIntSort()) {
					return true;
				}

				Counter::SetCounters(this->m_sourceData, this->m_counterSortData);
				if (!this->RunCounterSort()) {
					return true;
				}

				this->SetMessageAuto();
				this->SetControlButtonsAuto();
				Strip::InitValues(this->m_sourceData, this->m_stripSortData, this->GetSleepFunc(),
					this->GetDrawFunc(), this->GetUpdateMessageFunc(), this->m_sorts[sort_index_].GetIsMulThread());
			}
			else {
				const unsigned int randInt = GetConfigManager().GenerateRandom();
				this->m_intSortData = this->m_sourceData;
				VisualSort::Shuffle(this->m_intSortData, randInt);
				if (!this->RunIntSort()) {
					return true;
				}

				Counter::SetCounters(this->m_sourceData, this->m_counterSortData);
				VisualSort::Shuffle(this->m_counterSortData, randInt);
				if (!this->RunCounterSort()) {
					return true;
				}

				this->SetMessageAuto();
				this->SetControlButtonsAuto();
				Strip::InitValues(this->m_sourceData, this->m_stripSortData, this->GetSleepFunc(),
					this->GetDrawFunc(), []() {}, this->m_sorts[sort_index_].GetIsMulThread());
				VisualSort::Shuffle(m_sourceData, randInt);

				GetDrawingTool().ClearDevice();
				GetDrawingTool().FlushBatchDraw();

				for (auto it = this->m_messages.begin(); it != this->m_messages.end(); ++it) {
					it->DrawSketch(false);
				}
				this->m_controlButtons.RunNonBlockButtonLoop();

				try {
					for (auto it = this->m_stripSortData.begin(); it != this->m_stripSortData.end(); ++it) {
						Strip::DrawCheckStrip(*it, it->GetColor());
					}
					for (size_t i = 0; i < 100; ++i) {
						do {
							if (Strip::s_exitStripSort && !Strip::s_isMulThreadSort) {
								throw WideError(SortEndsPrematurely);
							}
							std::this_thread::sleep_for(std::chrono::milliseconds(10));
						} while (Strip::s_stopStripSort);
					}
					VisualSort::Shuffle(this->m_stripSortData, randInt);
					Strip::DrawRemainingStrip();
					for (size_t i = 0; i < 100; ++i) {
						do {
							if (Strip::s_exitStripSort && !Strip::s_isMulThreadSort) {
								throw WideError(SortEndsPrematurely);
							}
							std::this_thread::sleep_for(std::chrono::milliseconds(10));
						} while (Strip::s_stopStripSort);
					}
				}
				catch (const WideError& errorMessage) {
					if (errorMessage.What() != SortEndsPrematurely) {
						this->RunErrorWindow({ errorMessage.What() });
					}
					return true;
				}
				Strip::s_updateMessageFunc = this->GetUpdateMessageFunc();
			}

			if (this->RunStripSort()) {
				this->CheckData();
			}

			return true;
		}

		friend inline VisualSort& GetVisualSort();

	};

	inline VisualSort& GetVisualSort() {
		static VisualSort instance;
		return instance;
	}

}