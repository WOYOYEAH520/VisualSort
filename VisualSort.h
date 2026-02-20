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
				Sort(L"std::partial_sort",8192,StdPartialSort<int>,StdPartialSort<Counter>,StdPartialSort<Strip>)
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
			if(!this->m_sorts[this->m_sortIndex.value()].GetIsMulThread()) {
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