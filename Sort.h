#pragma once
#include "Strip.h"
#include "ConfigManager.h"
#include "Counter.h"
#include <future>
#include <execution>
#include <Windows.h>
#include <easyx.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <functional>
#include <list>
#include <mutex>
#include <random>
#include <stack>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
#include "WideError.h"
#include <concepts>
#include <deque>
#include <exception>
#include <forward_list>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <stop_token>
#include <tuple>
#include <unordered_map>
#include <map>

namespace NVisualSort {

	// 对数据量的约束
	class NumRequire {

	private:

		std::wstring m_requireInform; // 约束信息
		std::function<bool(size_t)> m_checkFunc; // 校验函数（校验失败返回false）

	public:

		NumRequire() : m_requireInform(L"无约束"), m_checkFunc([](size_t) { return true; }) {}
		NumRequire(std::wstring inform, std::function<bool(size_t)> checkFunc)
			: m_requireInform(std::move(inform)), m_checkFunc(std::move(checkFunc)) {
		}
		NumRequire(const NumRequire&) = default;
		NumRequire(NumRequire&&) = default;
		NumRequire& operator = (const NumRequire&) = default;
		NumRequire& operator = (NumRequire&&) = default;

		bool Check(size_t num_) const {
			return this->m_checkFunc(num_);
		}

		void SetCheckFunc(const std::function<bool(size_t)>& check_func_) {
			this->m_checkFunc = check_func_;
		}

		const std::wstring& GetRequireInform() const noexcept {
			return this->m_requireInform;
		}

		void SetRequireInform(const std::wstring& require_inform_) {
			this->m_requireInform = require_inform_;
		}

	};

	class Sort {

	private:

		std::wstring m_sortName; // 排序名称
		int m_maxSize = 0; // 排序最大数据量
		std::function<void(std::vector<int>&)> m_intSortFunc; // int 排序func实例
		std::function<void(std::vector<Counter>&)> m_counterSortFunc;
		std::function<void(std::vector<Strip>&)> m_stripSortFunc;

		std::vector<NumRequire> m_numRequires;
		bool m_isUnpredictable = false; // 算法是否不可预测（不可预测如猴子排序，睡眠排序）
		bool m_isMulThread = false; // 算法是否为多线程

	public:

		Sort(const std::wstring& sort_name_, int max_size_,
			const std::function<void(std::vector<int>&)>& int_sort_func_,
			const std::function<void(std::vector<Counter>&)>& counter_sort_func_,
			const std::function<void(std::vector<Strip>&)>& strip_sort_func_,
			std::vector<NumRequire> num_requires_ = {},
			bool is_unpredictable_ = false, bool is_mul_thread_ = false) :
			m_sortName(sort_name_), m_maxSize(max_size_), m_intSortFunc(int_sort_func_),
			m_counterSortFunc(counter_sort_func_), m_stripSortFunc(strip_sort_func_),
			m_numRequires(num_requires_), m_isUnpredictable(is_unpredictable_),
			m_isMulThread(is_mul_thread_) {
		}

		void SetMaxSize(int max_size_) {
			this->m_maxSize = max_size_;
		}
		int GetMaxSize() const {
			return this->m_maxSize;
		}

		void SetSortName(const std::wstring& sort_name_) {
			this->m_sortName = sort_name_;
		}
		const std::wstring& GetSortName() const {
			return this->m_sortName;
		}

		void SetIntSortFunc(const std::function<void(std::vector<int>&)>& int_sort_func_) {
			this->m_intSortFunc = int_sort_func_;
		}

		void SetCounterSortFunc(const std::function<void(std::vector<Counter>&)>& counter_sort_func_) {
			this->m_counterSortFunc = counter_sort_func_;
		}

		void SetStripSortFunc(const std::function<void(std::vector<Strip>&)>& strip_sort_func_) {
			this->m_stripSortFunc = strip_sort_func_;
		}

		void SetNumRequires(const std::vector<NumRequire>& num_requires_) {
			this->m_numRequires = num_requires_;
		}
		const std::vector<NumRequire>& GetNumRequires() const {
			return this->m_numRequires;
		}

		void AddNumRequire(const NumRequire& num_require_) {
			this->m_numRequires.push_back(num_require_);
		}

		bool EraseNumRequire(size_t index_) {
			if (index_ < this->m_numRequires.size()) {
				this->m_numRequires.erase(this->m_numRequires.begin() + index_);
				return true;
			}
			return false;
		}

		void SetIsUnpredictable(bool is_unpredictable_sort_) {
			this->m_isUnpredictable = is_unpredictable_sort_;
		}
		bool GetIsUnpredictable() const {
			return this->m_isUnpredictable;
		}

		void SetIsMulThread(bool is_mul_thread_) {
			this->m_isMulThread = is_mul_thread_;
		}
		bool GetIsMulThread() const {
			return this->m_isMulThread;
		}

		void RunIntSort(std::vector<int>& data_) {
			this->m_intSortFunc(data_);
		}

		void RunCounterSort(std::vector<Counter>& data_) {
			this->m_counterSortFunc(data_);
		}

		void RunStripSort(std::vector<Strip>& data_) {
			this->m_stripSortFunc(data_);
		}

		template<typename T> requires
			(std::same_as<T, int> || std::same_as<T, Counter> || std::same_as<T, Strip>)
		void RunSort(std::vector<T>& data_) {
			if constexpr (std::is_same_v<T, int>) {
				this->m_intSortFunc(data_);
			}
			else if constexpr (std::is_same_v<T, Counter>) {
				this->m_counterSortFunc(data_);
			}
			else if constexpr (std::is_same_v<T, Strip>) {
				this->m_stripSortFunc(data_);
			}
		}

	};

	// 排序算法实现（这些算法只考虑了 int,Counter,Strip 作为元素类型的情况，请谨慎在别的项目使用）
	namespace NSortAlgorithms {

		// 猴子排序专用初始随机数类，保证真实排序时间计算正确。
		class BogoSortRandomEngine {
		private:
			inline static std::atomic<bool> s_isIntUsed = false;
			inline static std::atomic<bool> s_isCounterUsed = false;
			inline static std::atomic<bool> s_isStripUsed = false;
			inline static std::atomic<int> s_randomNumber = GetConfigManager().GenerateRandom();
			template<class T> friend void BogoSort(std::vector<T>& data_);
			template<class T> static int GetBogoRandom() {
				if (s_isIntUsed.load() && s_isCounterUsed.load() && s_isStripUsed.load()) {
					s_isIntUsed = false;
					s_isCounterUsed = false;
					s_isStripUsed = false;
					s_randomNumber = GetConfigManager().GenerateRandom();
				}
				if constexpr (std::is_same_v<T, int>) {
					s_isIntUsed = true;
				}
				else if constexpr (std::is_same_v<T, Counter>) {
					s_isCounterUsed = true;
				}
				else if constexpr (std::is_same_v<T, Strip>) {
					s_isStripUsed = true;
				}
				return s_randomNumber;
			}
		};
		// 猴子排序
		template<class T = int> void BogoSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
			std::mt19937 randomEngin(BogoSortRandomEngine::GetBogoRandom<T>());
			bool isSorted = false;
			int timeToLive = 1000000;
			while (timeToLive--) {
				ptrdiff_t i = 0;
				for (; i < dataSize; ++i) {
					std::swap(data_[i], data_[randomEngin() % dataSize]);
				}
				for (i = 1; i < dataSize; ++i) {
					if (data_[i] < data_[i - 1]) break;
				}
				if (i == dataSize) {
					return;
				}
			}
			throw WideError(L"猴子排序超时！");
		}

		// 臭皮匠排序
		template<class T = int> void StoogeSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}

			// 使用栈来模拟递归调用
			std::stack<std::pair<ptrdiff_t, ptrdiff_t>> localStack;
			localStack.push(std::make_pair(static_cast<ptrdiff_t>(0), static_cast<ptrdiff_t>(data_.size() - 1)));

			while (!localStack.empty()) {
				ptrdiff_t i = localStack.top().first;
				ptrdiff_t j = localStack.top().second;
				localStack.pop();
				if (i >= j) {
					continue;
				}
				if (data_[i] > data_[j]) {
					std::swap(data_[i], data_[j]);
				}
				// 如果子数组有3个或更多元素，继续分割
				if (j - i > 1) {
					ptrdiff_t k = (j - i + 1) / 3;
					// 将三个子数组按相反顺序压入栈（因为是LIFO）
					// 这样处理顺序会是：先处理第一部分，然后第二部分，然后再次第一部分
					localStack.push(std::make_pair(i, j - k));// 第一次处理第一部分
					localStack.push(std::make_pair(i + k, j));// 处理第二部分
					localStack.push(std::make_pair(i, j - k));// 第二次处理第一部分
				}
			}
		}

		// 睡眠排序（睡眠的时间浮动且不可预测，所以排序时间的显示不可靠）
		template<class T = int> void SleepSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}

			if (data_.size() > 300) {
				throw WideError(L"睡眠排序数据量过大，可能会导致栈溢出！");
			}

			ptrdiff_t dataSize = data_.size();
			int maxValue = data_[0];
			int minValue = data_[0];
			for (int i = 1; i < dataSize; ++i) {
				if (data_[i] > maxValue) maxValue = data_[i];
				else if (data_[i] < minValue) minValue = data_[i];
			}
			long long rangeSize = static_cast<long long>(maxValue) - minValue;
			if (rangeSize == 0) {
				return;
			}
			else if (rangeSize > 100000) {
				throw WideError(L"该数据最小值与最大值差距过大，不建议使用睡眠排序！");
			}

			std::promise<void> startPromise;
			std::shared_future<void> startPuture = startPromise.get_future().share();

			std::vector<std::thread> workers;
			std::atomic<ptrdiff_t> workerOKNum = 0;
			std::mutex resultMutex;  // 保护结果数组的写入

			for (size_t i = 0; i < data_.size(); ++i) {
				long long sleepTime = static_cast<long long>(data_[i]) - minValue;

				workers.emplace_back([startPuture, sleepTime, minValue, &data_, &workerOKNum, &resultMutex]() {
					// 所有线程同时开始等待信号
					startPuture.wait();  // 这里没有锁，所有线程并行等待

					// 并行睡眠
					if constexpr (std::is_same_v<T, Strip>) {
						std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime * 100));
						AnimationStepNum += 1000;
					}
					else {
						std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
					}
					if constexpr (std::is_same_v<T, Counter>) {
						ActualStepNum += 1000;
					}

					// 将结果放入结果数组（需要保护，因为多个线程可能同时醒来）
					std::lock_guard<std::mutex> lock(resultMutex);
					data_[workerOKNum++] = static_cast<int>(sleepTime + minValue);
				});
			}

			// 注意：这里需要给线程一点时间启动，否则可能有些线程还没开始等待
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			startPromise.set_value();

			for (auto& worker : workers) {
				if (worker.joinable()) {
					worker.join();
				}
				else {
					throw WideError(L"睡眠排序出错：存在无法回归的线程！");
				}
			}

			for (auto it = data_.begin() + 1; it != data_.end(); ++it) {
				if (*it >= *(it - 1)) {
					continue;
				}
				T key = std::move(*it);
				auto targetPos = std::upper_bound(data_.begin(), it, key);
				std::move_backward(targetPos, it, it + 1);
				*targetPos = std::move(key);
			}
		}

		// 循环排序
		template<class T = int> void CycleSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
			for (ptrdiff_t cycleStart = 0; cycleStart < dataSize - 1; ++cycleStart) {
				T item(data_[cycleStart]);
				// 寻找 item 应该放置的位置
				ptrdiff_t itemPosition = cycleStart;
				for (ptrdiff_t i = cycleStart + 1; i < dataSize; ++i) {
					if (data_[i] < item) {
						++itemPosition;
					}
				}
				// 如果 item 已经在正确位置，继续下一个循环
				if (itemPosition == cycleStart) {
					continue;
				}
				// 跳过相等的元素
				while (item == data_[itemPosition]) {
					++itemPosition;
				}
				// 如果找到了不同元素，交换
				if (itemPosition != cycleStart) {
					std::swap(item, data_[itemPosition]);
				}
				// 继续旋转剩余的循环
				while (itemPosition != cycleStart) {
					itemPosition = cycleStart;
					// 为当前 item 寻找正确位置
					for (ptrdiff_t i = cycleStart + 1; i < dataSize; ++i) {
						if (data_[i] < item) {
							++itemPosition;
						}
					}
					// 跳过相等的元素
					while (item == data_[itemPosition]) {
						++itemPosition;
					}
					// 交换直到 item 回到原始位置
					if (item != data_[itemPosition]) {
						std::swap(item, data_[itemPosition]);
					}
				}
			}
		}

		// 冒泡老祖
		template<class T = int> void BubbleSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
			for (ptrdiff_t i = 0; i < dataSize; ++i) {
				bool noSwapped = true;
				for (ptrdiff_t j = 0; j < dataSize - i - 1; ++j) {
					if (data_[j] > data_[j + 1]) {
						std::swap(data_[j], data_[j + 1]);
						noSwapped = false;
					}
				}
				if (noSwapped) {
					break;
				}
			}
		}

		// 双向冒泡
		template<class T = int> void BidirectionalBubbleSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
			for (ptrdiff_t i = 0; i < (dataSize >> 1); ++i) {
				bool noSwap = true;
				for (ptrdiff_t j = i; j < dataSize - i - 1; ++j) {
					if (data_[j] > data_[j + 1]) {
						std::swap(data_[j], data_[j + 1]);
						noSwap = false;
					}
				}
				for (ptrdiff_t j = dataSize - i - 1; j > i; --j) {
					if (data_[j] < data_[j - 1]) {
						std::swap(data_[j], data_[j - 1]);
						noSwap = false;
					}
				}
				if (noSwap) {
					return;
				}
			}
		}

		// 奇偶排序
		template<class T = int> void OddEvenSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			ptrdiff_t dataSizeSub1 = static_cast<ptrdiff_t>(data_.size() - 1);
			bool odd = true;
			bool even = true;
			while (odd && even) {
				odd = false;
				even = false;
				for (ptrdiff_t i = 0; i < dataSizeSub1; i += 2) {
					if (data_[i] > data_[i + 1]) {
						std::swap(data_[i], data_[i + 1]);
						odd = true;
					}
				}
				for (ptrdiff_t i = 1; i < dataSizeSub1; i += 2) {
					if (data_[i] > data_[i + 1]) {
						std::swap(data_[i], data_[i + 1]);
						even = true;
					}
				}
			}
		}

		// 选择排序
		template<class T = int> void SelectionSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
			for (ptrdiff_t i = 0; i < dataSize - 1; ++i) {
				ptrdiff_t minValuePos = i;
				if constexpr (std::is_same_v<T, Strip>) {
					data_[i].SetColor(GREEN);
				}
				for (ptrdiff_t j = i + 1; j < dataSize; ++j) {
					if (data_[j] < data_[minValuePos]) {
						minValuePos = j;
					}
				}
				std::swap(data_[i], data_[minValuePos]);
			}
		}

		// 双向选择
		template<class T = int> void BidirectionalSelectionSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
			for (ptrdiff_t i = 0; i < (dataSize >> 1); ++i) {
				if constexpr (std::is_same_v<T, Strip>) {
					data_[i].SetColor(GREEN);
				}
				ptrdiff_t maxValuePos = i;
				ptrdiff_t minValuePos = i;
				ptrdiff_t j = i + 1;
				for (; j < dataSize - i; ++j) {
					if (data_[j] > data_[maxValuePos]) maxValuePos = j;
					else if (data_[j] < data_[minValuePos]) minValuePos = j;
				}

				if (maxValuePos == minValuePos) {
					return;
				}
				else if (i == maxValuePos) {
					std::swap(data_[j - 1], data_[maxValuePos]);
					if (j - 1 != minValuePos) {
						std::swap(data_[i], data_[minValuePos]);
					}
				}
				else {
					std::swap(data_[i], data_[minValuePos]);
					std::swap(data_[j - 1], data_[maxValuePos]);
				}
			}
		}

		// 插入排序
		template<class T = int> void InsertionSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}

			for (auto it = data_.begin() + 1; it != data_.end(); ++it) {
				if (*it >= *(it - 1)) {
					continue;
				}
				T key = std::move(*it);
				auto targetPos = std::upper_bound(data_.begin(), it, key);
				std::move_backward(targetPos, it, it + 1);
				*targetPos = std::move(key);
			}
		}

		// 珠排序
		template<class T = int> void BeadSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
				throw WideError(L"珠排序不支持浮点数！");
			}
			ptrdiff_t dataSize = data_.size();
			int maxValue = data_[0];
			int minValue = data_[0];
			for (int i = 1; i < dataSize; ++i) {
				if (data_[i] > maxValue) maxValue = data_[i];
				else if (data_[i] < minValue) minValue = data_[i];
			}
			long long rangeSize = static_cast<long long>(maxValue) - minValue;
			if (rangeSize == 0) {
				return;
			}
			else if (rangeSize > 10000000) {
				throw WideError(L"该数据最小值与最大值差距过大，不适合使用珠排序！");
			}
			std::vector<ptrdiff_t> beadQueue(rangeSize, 0);
			for (ptrdiff_t i = 0; i < dataSize; ++i) {
				for (int j = 0; j < static_cast<ptrdiff_t>(data_[i]) - minValue; ++j) {
					++beadQueue[j];
				}
			}
			ptrdiff_t i = 0;
			for (; i < dataSize - beadQueue[0]; ++i) {
				data_[i] = minValue;
			}
			for (; i < dataSize - *beadQueue.rbegin(); ++i) {
				for (ptrdiff_t j = static_cast<long long>(data_[i - 1]) - minValue; j < rangeSize; ++j) {
					if (beadQueue[j] < dataSize - i) {
						data_[i] = static_cast<int>(j + minValue);
						break;
					}
					if constexpr (std::is_same_v<T, Counter>) {
						++ActualStepNum;
					}
					else if constexpr (std::is_same_v<T, Strip>) {
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
						++AnimationStepNum;
					}
				}
			}
			for (; i < dataSize; ++i) {
				data_[i] = maxValue;
			}
		}

		// 梳排序
		template<class T = int> void CombSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}

			ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
			ptrdiff_t gap = dataSize;
			bool swapped = true;
			constexpr double shrink = 1.3;

			while (gap > 1 || swapped) {
				gap = static_cast<ptrdiff_t>(gap / shrink);
				if (gap < 1) {
					gap = 1;
				}

				swapped = false;

				for (ptrdiff_t i = 0; i + gap < dataSize; ++i) {
					if (data_[i] > data_[i + gap]) {
						std::swap(data_[i], data_[i + gap]);
						swapped = true;
					}
				}

				if (gap == 1 && !swapped) {
					break;
				}
			}
		}

		// 希尔排序
		template<class T = int> void ShellSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());

			// 使用 Ciura 增量序列
			std::vector<ptrdiff_t> gaps = { 701, 301, 132, 57, 23, 10, 4, 1 };

			for (ptrdiff_t gap : gaps) {
				if (gap >= dataSize) continue;

				for (ptrdiff_t i = gap; i < dataSize; ++i) {
					T temp = data_[i];
					ptrdiff_t j = i;

					while (j >= gap && data_[j - gap] > temp) {
						data_[j] = data_[j - gap];
						j -= gap;
					}
					data_[j] = temp;
				}
			}
		}

		// 双调排序
		template<class T = int> void BitonicSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			if ((data_.size() & (data_.size() - 1)) != 0) {
				throw WideError(L"双调排序要求数据量为2的非负整数次幂！");
			}

			struct Range {
				ptrdiff_t m_position;
				ptrdiff_t m_length;
				bool m_ascending;
				bool m_type;
			};
			std::stack<Range> localStack;
			localStack.push({ 0, static_cast<ptrdiff_t>(data_.size()), true, true });

			while (!localStack.empty()) {
				ptrdiff_t position = localStack.top().m_position;
				ptrdiff_t length = localStack.top().m_length;
				bool ascending = localStack.top().m_ascending;
				bool type = localStack.top().m_type;
				localStack.pop();

				if (length <= 1) {
					continue;
				}

				ptrdiff_t middleIndex = length / 2;

				if (type == false) {  // func1: 执行合并
					// 执行合并操作
					for (ptrdiff_t i = 0; i < middleIndex; ++i) {
						if ((data_[i + position] > data_[i + middleIndex + position]) == ascending) {
							std::swap(data_[i + position], data_[i + middleIndex + position]);
						}
					}

					// 递归处理子序列
					localStack.push({ position + middleIndex, middleIndex, ascending, false });
					localStack.push({ position, middleIndex, ascending, false });
				}
				else {  // func2: 递归分解
					// 添加func1合并任务（最后执行）
					localStack.push({ position, length, ascending, false });

					// 添加右半部分任务
					localStack.push({ position + middleIndex, length - middleIndex, ascending, true });

					// 添加左半部分任务（asd取反）
					localStack.push({ position, middleIndex, !ascending, true });
				}
			}
		}

		// 归并排序
		template<class T = int> void MergeSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}

			struct stackItem {
				ptrdiff_t m_leftIndex;
				ptrdiff_t m_rightIndex;
				bool m_isProcessed;
			};

			std::vector<stackItem> localStack;
			localStack.reserve(2 * static_cast<size_t>(log2(data_.size() + 1)) + 1);
			localStack.push_back({ 0, static_cast<ptrdiff_t>(data_.size() - 1), false });

			std::vector<T> dataQueue(data_.size() / 2 + 1);

			while (!localStack.empty()) {
				ptrdiff_t leftIndex = localStack.rbegin()->m_leftIndex;
				ptrdiff_t rightIndex = localStack.rbegin()->m_rightIndex;
				bool isProcessed = localStack.rbegin()->m_isProcessed;
				localStack.pop_back();

				if (leftIndex >= rightIndex) {
					continue;
				}

				ptrdiff_t middleIndex = leftIndex + (rightIndex - leftIndex) / 2;

				if (!isProcessed) {
					// 第一次访问：需要先处理子问题
					// 压入当前任务（标记为已处理）
					localStack.push_back({ leftIndex, rightIndex, true });
					// 压入右子任务
					localStack.push_back({ middleIndex + 1, rightIndex, false });
					// 压入左子任务
					localStack.push_back({ leftIndex, middleIndex, false });
				}
				// 第二次访问，子问题已处理，开始合并
				else {
					for (ptrdiff_t i = leftIndex; i <= middleIndex; ++i) {
						dataQueue[i - leftIndex] = data_[i];
					}
					ptrdiff_t dataIndex = middleIndex + 1;
					ptrdiff_t queueIndex = 0;
					ptrdiff_t targetIndex = leftIndex;
					ptrdiff_t queueSize = middleIndex - leftIndex;
					while (dataIndex <= rightIndex && queueIndex <= queueSize) {
						if (data_[dataIndex] < dataQueue[queueIndex]) {
							data_[targetIndex++] = data_[dataIndex++];
						}
						else {
							data_[targetIndex++] = dataQueue[queueIndex++];
						}
					}
					while (queueIndex <= queueSize) {
						data_[targetIndex++] = dataQueue[queueIndex++];
					}
				}
			}
		}

		// 堆排序
		template<class T = int> void HeapSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			auto sortFunc = [&data_](ptrdiff_t heap_size_) {
				ptrdiff_t leftIndex = 1;
				ptrdiff_t rightIndex = 2;
				ptrdiff_t index = 0;
				while (leftIndex < heap_size_) {
					ptrdiff_t largestIndex = 0;
					if (data_[leftIndex] < data_[rightIndex] && rightIndex < heap_size_) {
						largestIndex = rightIndex;
					}
					else largestIndex = leftIndex;
					if (data_[index] > data_[largestIndex]) {
						largestIndex = index;
					}
					if (index == largestIndex) {
						break;
					}
					if constexpr (std::is_same_v<T, Strip>) {
						SwapWithoutSetColor(data_[index], data_[largestIndex]);
					}
					else if constexpr (std::is_same_v<T, Counter>) {
						swap(data_[index], data_[largestIndex]);
					}
					else {
						std::swap(data_[index], data_[largestIndex]);
					}
					index = largestIndex;
					leftIndex = 2 * index + 1;
					rightIndex = leftIndex + 1;
				}
				};
			ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
			for (ptrdiff_t i = 0; i < dataSize; ++i) {
				ptrdiff_t currentIndex = i;
				if constexpr (std::is_same_v<T, Strip>) {
					const std::vector<COLORREF> heapColor = {
						BLUE,GREEN,CYAN,RED,MAGENTA,BROWN,YELLOW,LIGHTBLUE,LIGHTGREEN,LIGHTCYAN,LIGHTRED,LIGHTMAGENTA
					};
					data_[i].SetColor(heapColor[static_cast<size_t>(log2(i + 1)) % heapColor.size()]);
				}
				ptrdiff_t fatherIndex = (currentIndex - 1) / 2;
				while (data_[currentIndex] > data_[fatherIndex]) {
					if constexpr (std::is_same_v<T, Strip>) {
						SwapWithoutSetColor(data_[currentIndex], data_[fatherIndex]);
					}
					else if constexpr (std::is_same_v<T, Counter>) {
						swap(data_[currentIndex], data_[fatherIndex]);
					}
					else {
						std::swap(data_[currentIndex], data_[fatherIndex]);
					}
					currentIndex = fatherIndex;
					fatherIndex = (currentIndex - 1) / 2;
				}
			}
			ptrdiff_t heapSize = dataSize;
			while (heapSize > 1) {
				std::swap(data_[0], data_[--heapSize]);
				sortFunc(heapSize);
			}
		}

		// 快速排序
		template<class T = int> void QuickSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			std::stack<std::pair<ptrdiff_t, ptrdiff_t>> localStack;
			localStack.push({ static_cast<ptrdiff_t>(0), static_cast<ptrdiff_t>(data_.size()) - 1 });

			while (!localStack.empty()) {
				ptrdiff_t leftIndex = localStack.top().first;
				ptrdiff_t rightIndex = localStack.top().second;
				localStack.pop();

				if (leftIndex >= rightIndex) {
					continue;
				}

				ptrdiff_t i = leftIndex;
				ptrdiff_t j = rightIndex;
				T base = data_[leftIndex];

				if constexpr (std::is_same_v<T, Strip>) {
					data_[leftIndex].SetColor(GREEN);
				}

				while (i < j) {
					while (i < j && data_[j] >= base) --j;
					while (i < j && data_[i] <= base) ++i;
					if (i < j) std::swap(data_[i], data_[j]);
				}

				std::swap(data_[i], data_[leftIndex]);

				// 将子区间压入栈中，先处理较大的区间以减少栈深度
				if (i - leftIndex < rightIndex - i) {
					localStack.push(std::make_pair(i + 1, rightIndex));
					localStack.push(std::make_pair(leftIndex, i - 1));
				}
				else {
					localStack.push(std::make_pair(leftIndex, i - 1));
					localStack.push(std::make_pair(i + 1, rightIndex));
				}
			}
		}

		// 基数排序给桶染色用的，可以随意修改颜色（至少要有一个颜色）
		inline std::vector<COLORREF> RadixSortBucketColor = {
			BLUE,MAGENTA,CYAN,RED,BROWN,YELLOW,GREEN,WHITE,BLACK,
			LIGHTBLUE,LIGHTGREEN,LIGHTCYAN,LIGHTRED,LIGHTMAGENTA
		};
		// 基数排序
		template<class T = int> void RadixSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
				throw WideError(L"基数排序不支持浮点数！");
			}
			ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
			int maxValue = data_[0];
			int minValue = data_[0];
			for (ptrdiff_t i = 1; i < dataSize; ++i) {
				if (data_[i] > maxValue) maxValue = data_[i];
				else if (data_[i] < minValue) minValue = data_[i];
			}
			long long rangeSize = static_cast<long long>(maxValue) - minValue;
			if (rangeSize == 0) {
				return;
			}

			const int base = 16;
			std::vector<std::list<T>> bucket(base);
			for (ptrdiff_t i = 0; i < dataSize; ++i) {
				if constexpr (std::is_same_v<T, Strip>) {
					ptrdiff_t bucketIndex = (static_cast<ptrdiff_t>(data_[i].GetValue()) - minValue) % base;
					bucket[bucketIndex].push_back(data_[i].GetValue());
					bucket[bucketIndex].back().SetColor(RadixSortBucketColor[bucketIndex % RadixSortBucketColor.size()]);
					data_[i].SetColor(RadixSortBucketColor[bucketIndex % RadixSortBucketColor.size()]);
					Strip::DrawStrip1(data_[i], StripCopyColor);
					++AnimationStepNum;
				}
				else {
					int tempValue = data_[i];
					bucket[(static_cast<ptrdiff_t>(tempValue) - minValue) % base].push_back(tempValue);
				}
			}

			std::list<T> dataQueue;
			for (int i = 0; i < base; ++i) {
				dataQueue.splice(dataQueue.end(), bucket[i]);
			}
			auto copyDataFromQueue = [&dataQueue, &data_]() {
				ptrdiff_t dataIndex = 0;
				for (auto it = dataQueue.begin(); it != dataQueue.end(); ++it) {
					if constexpr (std::is_same_v<T, Strip>) {
						data_[dataIndex++].CopyValueAndColor(*it);
					}
					else {
						data_[dataIndex++] = *it;
					}
				}
				};
			if constexpr (std::is_same_v<T, Strip> || std::is_same_v<T, Counter>) {
				copyDataFromQueue();
			}

			int digit = 0;
			for (ptrdiff_t i = rangeSize; i > 0; i /= base) {
				++digit;
			}
			int divNum = base;
			for (int i = 1; i < digit; ++i) {
				if constexpr (std::is_same_v<T, Strip>) {
					ptrdiff_t dataIndex = 0;
					while (!dataQueue.empty()) {
						ptrdiff_t bucketIndex = ((static_cast<ptrdiff_t>(*dataQueue.begin()) - minValue) / static_cast<ptrdiff_t>(divNum)) % base;
						bucket[bucketIndex].splice(bucket[bucketIndex].end(), dataQueue, dataQueue.begin());
						bucket[bucketIndex].back().SetColor(RadixSortBucketColor[bucketIndex % RadixSortBucketColor.size()]);
						data_[dataIndex].SetColor(RadixSortBucketColor[bucketIndex % RadixSortBucketColor.size()]);
						Strip::DrawStrip1(data_[dataIndex++], StripCopyColor);
						++AnimationStepNum;
					}
				}
				else {
					while (!dataQueue.empty()) {
						ptrdiff_t bucketIndex = ((static_cast<ptrdiff_t>(*dataQueue.begin()) - minValue) / static_cast<ptrdiff_t>(divNum)) % base;
						bucket[bucketIndex].splice(bucket[bucketIndex].end(), dataQueue, dataQueue.begin());
					}
				}
				if constexpr (std::is_same_v<T, Counter>) {
					ActualStepNum += dataSize;
				}
				for (int j = 0; j < base; ++j) {
					dataQueue.splice(dataQueue.end(), bucket[j]);
				}
				if constexpr (std::is_same_v<T, Strip> || std::is_same_v<T, Counter>) {
					copyDataFromQueue();
				}
				divNum *= base;
			}

			if constexpr (std::is_same_v<T, int>) {
				copyDataFromQueue();
			}
		}

		// 计数排序
		template<class T = int> void CountingSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
				throw WideError(L"计数排序不支持浮点数！");
			}
			ptrdiff_t dataSize = static_cast<ptrdiff_t>(data_.size());
			int maxValue = data_[0];
			int minValue = data_[0];
			for (ptrdiff_t i = 1; i < dataSize; ++i) {
				if (data_[i] > maxValue) maxValue = data_[i];
				else if (data_[i] < minValue) minValue = data_[i];
			}
			long long rangeSize = static_cast<long long>(maxValue) - minValue + 1;
			if (rangeSize == 1) {
				return;
			}
			if (rangeSize > 10000000) {
				throw WideError(L"该数据最小值与最大值差距过大，不适合使用计数排序！");
			}
			std::vector<int> countQueue(rangeSize, 0);
			for (ptrdiff_t i = 0; i < dataSize; ++i) {
				++countQueue[static_cast<ptrdiff_t>(data_[i]) - minValue];
			}
			ptrdiff_t j = 0;
			for (ptrdiff_t i = 0; i < rangeSize; ++i) {
				while (countQueue[i]) {
					data_[j++] = static_cast<int>(i + minValue);
					--countQueue[i];
				}
			}
		}

		// C++标准库排序
		template<class T = int> void StdSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			std::sort(data_.begin(), data_.end());
		}

		// C++标准库 稳定排序
		template<class T = int> void StdStableSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			std::stable_sort(data_.begin(), data_.end());
		}

		// C++标准库 建堆 + 堆排序
		template<class T = int> void StdHeapSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			std::make_heap(data_.begin(), data_.end());
			std::sort_heap(data_.begin(), data_.end());
		}

		// C++标准库 Partial 排序
		template<class T = int> void StdPartialSort(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			std::partial_sort(data_.begin(), data_.end(), data_.end());
		}

		// C++标准库排序（并行，线程池设计导致 Strip::st_scopeGuard 在程序结束时才全部析构，Strip 颜色残留）
		template<class T = int> void StdSort_Parallel(std::vector<T>& data_) {
			if (data_.size() < 2) {
				return;
			}
			std::sort(std::execution::par, data_.begin(), data_.end());
		}

	}

}