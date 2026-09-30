#pragma once
#include "ConfigManager.hpp"
#include "DrawingTool.hpp"
#include <chrono>
#include <easyx.h>
#include <optional>
#include <atomic>
#include <Windows.h>
#include <functional>
#include <thread>
#include <utility>
#include <vector>
#include "Fraction.hpp"
#include "WideError.hpp"
#include "ScopeGuard.hpp"
#include <set>
#include <mutex>

namespace NVisualSort {

	inline std::atomic<size_t> StripCompareNum{ 0 };  // 比较次数
	inline std::atomic<size_t> StripCopyNum{ 0 };     // 引用次数
	inline std::atomic<size_t> StripChangeNum{ 0 };   // 写入次数
	inline std::atomic<size_t> AnimationStepNum{ 0 }; // 动画步数

	constexpr COLORREF StripCopyColor = LIGHTBLUE;
	constexpr COLORREF StripChangeColor = RED;
	constexpr COLORREF StripFirstColor = RGB(0x88, 0x88, 0x88);
	constexpr COLORREF StripLastColor = WHITE;

	constexpr Fraction StripMaxTopFrac{ 2,9 };
	constexpr Fraction StripDensityThreshold{ 1,6 };

	constexpr const wchar_t* SortEndsPrematurely = L"排序提前结束";

	class Strip {

		friend class VisualSort;

	private:

		inline static std::chrono::milliseconds s_stripSortStopTime;
		inline static std::atomic<bool> s_stopStripSort = false;
		inline static std::atomic<bool> s_exitStripSort = false;

		int m_value = 0;
		int m_left = -1;
		int m_right = -1;
		int m_top = 0;
		COLORREF m_color = WHITE;
		bool m_notTemp = false;

		inline static long long s_maxValue = 0;
		inline static long long s_minValue = 0;
		inline static int s_zeroLineY = 0;
		inline static bool s_isAllSame = false;
		inline static bool s_isAllZero = false;

		struct Interval {
			int left = 0;
			int right = 0;
			constexpr Interval() : Interval(0, 0) {}
			constexpr Interval(int left_, int right_) noexcept : left(left_), right(right_) {}
		};

		inline static thread_local unsigned int st_lastOperationNum = 0;
		inline static thread_local Interval st_lastInterval1;
		inline static thread_local std::pair<Interval, Interval> st_lastInterval2;

		inline static void(*s_sleepFunc)() = nullptr;
		inline static void(*s_drawFunc)(RECT, COLORREF) = nullptr;
		inline static void(*s_updateMessageFunc)() = nullptr;

		inline static std::set<std::thread::id> s_threads;
		inline static std::mutex s_threadsMutex;
		inline static std::thread::id s_mainThreadId = std::this_thread::get_id();
		inline static bool s_isMulThreadSort = false;

		static void InitValues() {
			Strip::st_lastOperationNum = 0;
			Strip::s_stripSortStopTime = std::chrono::milliseconds(0);
			Strip::s_stopStripSort.store(false, std::memory_order_release);
			Strip::s_exitStripSort.store(false, std::memory_order_release);
			StripCompareNum.store(0, std::memory_order_release);
			StripCopyNum.store(0, std::memory_order_release);
			StripChangeNum.store(0, std::memory_order_release);
			AnimationStepNum.store(0, std::memory_order_release);
			Strip::s_mainThreadId = std::this_thread::get_id();
			std::thread::id thisId = Strip::s_mainThreadId;
			std::erase_if(Strip::s_threads, [thisId](const auto& id) {
				return id != thisId;
				});
		}

		static void InitValues(const std::vector<int> data_,
			std::vector<Strip>& strips_,
			void(*sleep_func)(),
			void(*update_message_func_)(),
			bool is_mul_thread_sort_) {
			Strip::InitValues();
			Strip::s_sleepFunc = sleep_func;
			Strip::s_updateMessageFunc = update_message_func_;
			Strip::s_isMulThreadSort = is_mul_thread_sort_;
			strips_.clear(); // 直接清空，否则如果resize，那么vector底层可能会复制strip，从而触发动画
			strips_.resize(data_.size());
			long long tempMinValue = static_cast<long long>(data_[0]);
			long long tempMaxValue = static_cast<long long>(data_[0]);
			for (size_t dataIndex = 1; dataIndex < data_.size(); ++dataIndex) {
				if (tempMinValue > data_[dataIndex]) tempMinValue = static_cast<long long>(data_[dataIndex]);
				else if (tempMaxValue < data_[dataIndex]) tempMaxValue = static_cast<long long>(data_[dataIndex]);
			}
			Strip::s_minValue = tempMinValue;
			Strip::s_maxValue = tempMaxValue;
			auto height = GetConfigManager().GetHeight();
			static int MiddleTop = 0;
			MiddleTop = (1 + StripMaxTopFrac) * height / 2;
			if (tempMinValue == tempMaxValue) {
				Strip::s_isAllSame = true;
				if (tempMinValue > 0) {
					Strip::s_isAllZero = false;
					Strip::s_minValue = 0;
					Strip::s_maxValue *= 2;
					Strip::s_zeroLineY = height;
				}
				else if (tempMinValue < 0) {
					Strip::s_isAllZero = false;
					Strip::s_maxValue = 0;
					Strip::s_zeroLineY = (1 + StripMaxTopFrac) * height / 2;
				}
				else {
					Strip::s_isAllZero = true;
					Strip::s_zeroLineY = height;
				}
				if (height * StripDensityThreshold < data_.size()) {
					Strip::s_drawFunc = [](RECT rect_, COLORREF color_) {
						GetDrawingTool().SolidRectangle(rect_.left, MiddleTop, rect_.right, GetConfigManager().GetHeight(), color_);
						};
				}
				else {
					Strip::s_drawFunc = [](RECT rect_, COLORREF color_) {
						GetDrawingTool().FillRectangle(rect_.left, MiddleTop, rect_.right, GetConfigManager().GetHeight(), 1, PS_SOLID, BLACK, color_);
						};
				}
			}
			else {
				Strip::s_isAllSame = false;
				Strip::s_isAllZero = false;
				if (tempMinValue < 0) {
					Strip::s_zeroLineY = height + tempMinValue * (height - Strip::StripMaxTop()) / (tempMaxValue - tempMinValue);
				}
				else {
					Strip::s_zeroLineY = height;
				}
				if (height * StripDensityThreshold < data_.size()) {
					Strip::s_drawFunc = [](RECT rect_, COLORREF color_) {
						GetDrawingTool().SolidRectangle(rect_, color_);
						};
				}
				else {
					Strip::s_drawFunc = [](RECT rect_, COLORREF color_) {
						GetDrawingTool().FillRectangle(rect_, 1, PS_SOLID, BLACK, color_);
						};
				}
			}
			for (size_t stripIndex = 0; stripIndex < strips_.size(); ++stripIndex) {
				int tempLeft = stripIndex * GetConfigManager().GetWidth() / strips_.size();
				int tempRight = (stripIndex + 1) * GetConfigManager().GetWidth() / strips_.size();
				strips_[stripIndex].SetStrip(data_[stripIndex], tempLeft, tempRight);
			}
		}

		static void FlushRemainingStrip() {
			if (Strip::st_lastOperationNum == 1) {
				GetDrawingTool().FlushBatchDraw(Strip::st_lastInterval1.left, Strip::StripMaxTop(), Strip::st_lastInterval1.right, GetConfigManager().GetHeight());
			}
			else if (Strip::st_lastOperationNum == 2) {
				GetDrawingTool().FlushBatchDraw(Strip::st_lastInterval2.first.left, Strip::StripMaxTop(), Strip::st_lastInterval2.first.right, GetConfigManager().GetHeight());
				GetDrawingTool().FlushBatchDraw(Strip::st_lastInterval2.second.left, Strip::StripMaxTop(), Strip::st_lastInterval2.second.right, GetConfigManager().GetHeight());
			}
		}

		inline static thread_local ScopeGuard st_scopeGuard{ []() {
			if (::GetHWnd()) Strip::FlushRemainingStrip();
			std::lock_guard lock(Strip::s_threadsMutex);
			Strip::s_threads.erase(std::this_thread::get_id());
		} };

		Strip& SetTopAuto() noexcept {
			auto height = GetConfigManager().GetHeight();
			if (Strip::s_isAllZero) [[unlikely]] {
				if (this->m_value == 0) {
					this->m_top = (1 + StripMaxTopFrac) * height / 2;
				}
				else if (this->m_value > 0) {
					this->m_top = Strip::StripMaxTop();
				}
				else {
					this->m_top = height;
				}
				return *this;
			}
			int maxTop = static_cast<int>(Strip::StripMaxTop());
			long long lowestValue = Strip::s_minValue > 0 ? 0 : Strip::s_minValue;
			int computedTop = static_cast<int>(
				height - ((static_cast<long long>(this->m_value) - lowestValue) *
					(height - maxTop) / (Strip::s_maxValue - lowestValue)));
			if (computedTop < maxTop) computedTop = maxTop;
			if (height < computedTop) computedTop = height;
			this->m_top = computedTop;
			return *this;
		}

	public:

		Strip& SetColorAuto() noexcept {
			if (Strip::s_isAllSame) [[unlikely]] {
				this->m_color = WHITE;
				return *this;
			}
			COLORREF tempColor = static_cast<COLORREF>((StripLastColor - StripFirstColor) / RGB(1, 1, 1) *
				(static_cast<long long>(this->m_value) - Strip::s_minValue) / (Strip::s_maxValue - Strip::s_minValue));
			this->m_color = RGB(tempColor, tempColor, tempColor) + StripFirstColor;
			return *this;
		}

		Strip& SetTopAndColorAuto() noexcept {
			this->SetTopAuto();
			this->SetColorAuto();
			return *this;
		}

	private:

		static void StopSort() {
			if (std::this_thread::get_id() != Strip::s_mainThreadId) {
				while (Strip::s_stopStripSort) {
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}
			}
			else {
				auto stopBeginTime1 = std::chrono::steady_clock::now();
				std::optional<std::chrono::steady_clock::time_point> stopBeginTime;
				while (Strip::s_stopStripSort) {
					if (Strip::s_exitStripSort && !Strip::s_isMulThreadSort) {
						throw WideError(SortEndsPrematurely);
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					stopBeginTime = stopBeginTime1;
				}
				if (stopBeginTime.has_value()) {
					auto stopEndTime = std::chrono::steady_clock::now();
					Strip::s_stripSortStopTime += std::chrono::duration_cast<std::chrono::milliseconds>(stopEndTime - stopBeginTime.value());
				}
				if (Strip::s_exitStripSort && !Strip::s_isMulThreadSort) {
					throw WideError(SortEndsPrematurely);
				}
			}
		}

		static void RegisterCurrentThread() {
			auto temp = (&Strip::st_scopeGuard, true);
			if (std::this_thread::get_id() == Strip::s_mainThreadId) {
				Strip::st_scopeGuard.Dismiss();
			}
			std::lock_guard<std::mutex> lock(Strip::s_threadsMutex);
			Strip::s_threads.insert(std::this_thread::get_id());
		}

	public:

		constexpr Strip(int value_ = 0, bool not_temp_ = false) :m_value(value_), m_notTemp(not_temp_) {
			this->m_left = -1;
			this->m_right = -1;
			this->m_top = 0;
			this->m_color = BLACK;
		}

		Strip(int value_, int left_, int right_) :m_value(value_), m_left(left_), m_right(right_) {
			this->m_notTemp = true;
			this->SetTopAndColorAuto();
		}

		Strip(const Strip& strip_) {
			this->m_value = strip_.m_value;
			this->m_left = -1;
			this->m_right = -1;
			this->m_top = 0;
			this->m_color = BLACK;
			this->m_notTemp = false;
			if (strip_.m_notTemp) {
				Strip::DrawStrip1((Strip::AddNumCopy1_StripToInt(), strip_), StripCopyColor);
			}
		}

		Strip& SetStrip(int value_, int left_, int right_) {
			this->m_value = value_;
			this->m_left = left_;
			this->m_right = right_;
			this->m_notTemp = true;
			this->SetTopAndColorAuto();
			return *this;
		}

		constexpr int GetValue() const noexcept {
			return this->m_value;
		}
		Strip& SetValue(int value_) noexcept {
			this->m_value = value_;
		}

		constexpr COLORREF GetColor() const noexcept {
			return this->m_color;
		}
		Strip& SetColor(COLORREF color_) noexcept {
			this->m_color = color_;
			return *this;
		}

		constexpr bool GetNotTemp() const noexcept {
			return this->m_notTemp;
		}
		Strip& SetNotTemp(bool not_temp_) noexcept {
			this->m_notTemp = not_temp_;
		}

		static void DrawStrips(const std::vector<Strip>& strips_) {
			const int width = GetConfigManager().GetWidth();
			const int height = GetConfigManager().GetHeight();
			const int top = Strip::StripMaxTop();
			GetDrawingTool().ClearRectangle(0, top, width, height);
			for (const auto& strip : strips_) {
				const int l = (std::max)(0, strip.m_left);
				const int r = (std::min)(width - 1, strip.m_right);
				if (l - r != 0) {
					Strip::s_drawFunc(RECT(l, strip.m_top, r, Strip::s_zeroLineY), strip.m_color);
				}
			}
			GetDrawingTool().FlushBatchDraw(0, top, width, height);
		}

		static Fraction StripMaxTop() noexcept {
			return GetConfigManager().GetHeight() * StripMaxTopFrac;
		}

		static void DrawStrip1(const Strip& strip_, COLORREF color_) {
			static thread_local auto temp = (Strip::RegisterCurrentThread(), true);
			if (Strip::s_exitStripSort.load(std::memory_order_acquire)) {
				return;
			}
			GetDrawingTool().ClearRectangle(strip_.m_left, Strip::StripMaxTop(), strip_.m_right, GetConfigManager().GetHeight());
			Strip::s_drawFunc(RECT(strip_.m_left, strip_.m_top, strip_.m_right, Strip::s_zeroLineY), color_);
			GetDrawingTool().FlushBatchDraw(strip_.m_left, Strip::StripMaxTop(), strip_.m_right, GetConfigManager().GetHeight());
			Strip::s_updateMessageFunc();
			Strip::FlushRemainingStrip();
			Strip::s_sleepFunc();
			Strip::StopSort();
			Strip::s_drawFunc(RECT(strip_.m_left, strip_.m_top, strip_.m_right, Strip::s_zeroLineY), strip_.m_color);
			Strip::st_lastOperationNum = 1;
			Strip::st_lastInterval1 = { strip_.m_left,strip_.m_right };
		}

		static void DrawStrip2(const Strip& strip1_, COLORREF color1_, const Strip& strip2_, COLORREF color2_) {
			static thread_local auto temp = (Strip::RegisterCurrentThread(), true);
			if (Strip::s_exitStripSort.load(std::memory_order_acquire)) {
				return;
			}
			GetDrawingTool().ClearRectangle(strip1_.m_left, Strip::StripMaxTop(), strip1_.m_right, GetConfigManager().GetHeight());
			GetDrawingTool().ClearRectangle(strip2_.m_left, Strip::StripMaxTop(), strip2_.m_right, GetConfigManager().GetHeight());
			Strip::s_drawFunc(RECT(strip1_.m_left, strip1_.m_top, strip1_.m_right, Strip::s_zeroLineY), color1_);
			Strip::s_drawFunc(RECT(strip2_.m_left, strip2_.m_top, strip2_.m_right, Strip::s_zeroLineY), color2_);
			GetDrawingTool().FlushBatchDraw(strip1_.m_left, Strip::StripMaxTop(), strip1_.m_right, GetConfigManager().GetHeight());
			GetDrawingTool().FlushBatchDraw(strip2_.m_left, Strip::StripMaxTop(), strip2_.m_right, GetConfigManager().GetHeight());
			Strip::s_updateMessageFunc();
			Strip::FlushRemainingStrip();
			Strip::s_sleepFunc();
			Strip::StopSort();
			Strip::s_drawFunc(RECT(strip1_.m_left, strip1_.m_top, strip1_.m_right, Strip::s_zeroLineY), strip1_.m_color);
			Strip::s_drawFunc(RECT(strip2_.m_left, strip2_.m_top, strip2_.m_right, Strip::s_zeroLineY), strip2_.m_color);
			Strip::st_lastOperationNum = 2;
			Strip::st_lastInterval2 = { {strip1_.m_left,strip1_.m_right},{strip2_.m_left,strip2_.m_right} };
		}

		static void DrawCheckStrip(const Strip& strip_, COLORREF color_) {
			if (Strip::s_exitStripSort.load(std::memory_order_acquire)) {
				return;
			}
			Strip::s_drawFunc(RECT(strip_.m_left, strip_.m_top, strip_.m_right, Strip::s_zeroLineY), color_);
			GetDrawingTool().FlushBatchDraw(RECT(strip_.m_left, Strip::StripMaxTop(), strip_.m_right, GetConfigManager().GetHeight()));
			Strip::StopSort();
			Strip::s_sleepFunc();
		}

		static void AddNumCompare1() {
			++StripCopyNum;
			++StripCompareNum;
			++AnimationStepNum;
		}

		static void AddNumCompare2() {
			StripCopyNum += 2;
			++StripCompareNum;
			++AnimationStepNum;
		}

		static void AddNumCopy1_IntToStrip() {
			++StripCopyNum;
			++StripChangeNum;
			++AnimationStepNum;
		}

		static void AddNumCopy1_StripToInt() {
			++StripCopyNum;
			++AnimationStepNum;
		}

		static void AddNumCopy2() {
			StripCopyNum += 2;
			++StripChangeNum;
			++AnimationStepNum;
		}

		static void AddNumCopy2_StripToInt() {
			StripCopyNum += 2;
			++AnimationStepNum;
		}

		static void AddNumSwap2() {
			StripCopyNum += 4;
			StripChangeNum += 2;
			++AnimationStepNum;
		}

		bool operator > (const int value_) const {
			if (this->m_notTemp) {
				Strip::DrawStrip1(*this, StripCopyColor);
				Strip::AddNumCompare1();
			}
			return this->m_value > value_;
		}
		bool operator > (const Strip& strip_) const {
			if (this->m_notTemp != strip_.m_notTemp) {
				Strip::DrawStrip1(this->m_notTemp ? *this : strip_, StripCopyColor);
				Strip::AddNumCompare1();
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(*this, StripCopyColor, strip_, StripCopyColor);
				Strip::AddNumCompare2();
			}
			return this->m_value > strip_.m_value;
		}
		friend bool operator > (const int, const Strip&);

		bool operator < (const int value_) const {
			if (this->m_notTemp) {
				Strip::DrawStrip1(*this, StripCopyColor);
				Strip::AddNumCompare1();
			}
			return this->m_value < value_;
		}
		bool operator < (const Strip& strip_) const {
			if (this->m_notTemp != strip_.m_notTemp) {
				Strip::DrawStrip1(this->m_notTemp ? *this : strip_, StripCopyColor);
				Strip::AddNumCompare1();
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(*this, StripCopyColor, strip_, StripCopyColor);
				Strip::AddNumCompare2();
			}
			return this->m_value < strip_.m_value;
		}
		friend bool operator < (const int, const Strip&);

		bool operator >= (const int value_) const {
			if (this->m_notTemp) {
				Strip::DrawStrip1(*this, StripCopyColor);
				Strip::AddNumCompare1();
			}
			return this->m_value >= value_;
		}
		bool operator >= (const Strip& strip_) const {
			if (this->m_notTemp != strip_.m_notTemp) {
				Strip::DrawStrip1(this->m_notTemp ? *this : strip_, StripCopyColor);
				Strip::AddNumCompare1();
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(*this, StripCopyColor, strip_, StripCopyColor);
				Strip::AddNumCompare2();
			}
			return this->m_value >= strip_.m_value;
		}
		friend bool operator >= (const int, const Strip&);

		bool operator <= (const int value_) const {
			if (this->m_notTemp) {
				Strip::DrawStrip1(*this, StripCopyColor);
				Strip::AddNumCompare1();
			}
			return this->m_value <= value_;
		}
		bool operator <= (const Strip& strip_) const {
			if (this->m_notTemp != strip_.m_notTemp) {
				Strip::DrawStrip1(this->m_notTemp ? *this : strip_, StripCopyColor);
				Strip::AddNumCompare1();
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(*this, StripCopyColor, strip_, StripCopyColor);
				Strip::AddNumCompare2();
			}
			return this->m_value <= strip_.m_value;
		}
		friend bool operator <= (const int, const Strip&);

		bool operator == (const int value_) const {
			if (this->m_notTemp) {
				Strip::DrawStrip1(*this, StripCopyColor);
				Strip::AddNumCompare1();
			}
			return this->m_value == value_;
		}
		bool operator == (const Strip& strip_) const {
			if (this->m_notTemp != strip_.m_notTemp) {
				Strip::DrawStrip1(this->m_notTemp ? *this : strip_, StripCopyColor);
				Strip::AddNumCompare1();
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(*this, StripCopyColor, strip_, StripCopyColor);
				Strip::AddNumCompare2();
			}
			return this->m_value == strip_.m_value;
		}
		friend bool operator == (const int, const Strip&);

		bool operator != (const int value_) const {
			if (this->m_notTemp) {
				Strip::DrawStrip1(*this, StripCopyColor);
				Strip::AddNumCompare1();
			}
			return this->m_value != value_;
		}
		bool operator != (const Strip& strip_) const {
			if (this->m_notTemp != strip_.m_notTemp) {
				Strip::DrawStrip1(this->m_notTemp ? *this : strip_, StripCopyColor);
				Strip::AddNumCompare1();
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(*this, StripCopyColor, strip_, StripCopyColor);
				Strip::AddNumCompare2();
			}
			return this->m_value != strip_.m_value;
		}
		friend bool operator != (const int, const Strip&);

		Strip& operator = (const int value_) {
			this->m_value = value_;
			if (this->m_notTemp) {
				Strip::DrawStrip1(this->SetTopAndColorAuto(), StripCopyColor);
				Strip::AddNumCopy1_IntToStrip();
			}
			return *this;
		}
		Strip& operator = (const Strip& strip_) {
			this->m_value = strip_.m_value;
			if (this->m_notTemp != strip_.m_notTemp) {
				if (this->m_notTemp) {
					Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
					Strip::AddNumCopy1_IntToStrip();
				}
				else {
					Strip::DrawStrip1(strip_, StripCopyColor);
					Strip::AddNumCopy1_StripToInt();
				}
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(this->SetTopAndColorAuto(), StripChangeColor, strip_, StripCopyColor);
				Strip::AddNumCopy2();
			}
			return *this;
		}
		operator int() const {
			if (this->m_notTemp) {
				Strip::DrawStrip1(*this, StripCopyColor);
				Strip::AddNumCopy1_StripToInt();
			}
			return this->m_value;
		}

		Strip& operator += (const int value_) {
			this->m_value += value_;
			if (this->m_notTemp) {
				Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
				Strip::AddNumCopy1_IntToStrip();
			}
			return *this;
		}
		Strip& operator += (const Strip& strip_) {
			this->m_value += strip_.m_value;
			if (this->m_notTemp != strip_.m_notTemp) {
				if (this->m_notTemp) {
					Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
					Strip::AddNumCopy1_IntToStrip();
				}
				else {
					Strip::DrawStrip1(strip_, StripCopyColor);
					Strip::AddNumCopy1_StripToInt();
				}
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(this->SetTopAndColorAuto(), StripChangeColor, strip_, StripCopyColor);
				Strip::AddNumCopy2();
			}
			return *this;
		}

		Strip& operator -= (const int value_) {
			this->m_value -= value_;
			if (this->m_notTemp) {
				Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
				Strip::AddNumCopy1_IntToStrip();
			}
			return *this;
		}
		Strip& operator -= (const Strip& strip_) {
			this->m_value -= strip_.m_value;
			if (this->m_notTemp != strip_.m_notTemp) {
				if (this->m_notTemp) {
					Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
					Strip::AddNumCopy1_IntToStrip();
				}
				else {
					Strip::DrawStrip1(strip_, StripCopyColor);
					Strip::AddNumCopy1_StripToInt();
				}
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(this->SetTopAndColorAuto(), StripChangeColor, strip_, StripCopyColor);
				Strip::AddNumCopy2();
			}
			return *this;
		}

		Strip& operator *= (const int value_) {
			this->m_value *= value_;
			if (this->m_notTemp) {
				Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
				Strip::AddNumCopy1_IntToStrip();
			}
			return *this;
		}
		Strip& operator *= (const Strip& strip_) {
			this->m_value *= strip_.m_value;
			if (this->m_notTemp != strip_.m_notTemp) {
				if (this->m_notTemp) {
					Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
					Strip::AddNumCopy1_IntToStrip();
				}
				else {
					Strip::DrawStrip1(strip_, StripCopyColor);
					Strip::AddNumCopy1_StripToInt();
				}
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(this->SetTopAndColorAuto(), StripChangeColor, strip_, StripCopyColor);
				Strip::AddNumCopy2();
			}
			return *this;
		}

		Strip& operator /= (const int value_) {
			this->m_value /= value_;
			if (this->m_notTemp) {
				Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
				Strip::AddNumCopy1_IntToStrip();
			}
			return *this;
		}
		Strip& operator /= (const Strip& strip_) {
			this->m_value /= strip_.m_value;
			if (this->m_notTemp != strip_.m_notTemp) {
				if (this->m_notTemp) {
					Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
					Strip::AddNumCopy1_IntToStrip();
				}
				else {
					Strip::DrawStrip1(strip_, StripCopyColor);
					Strip::AddNumCopy1_StripToInt();
				}
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(this->SetTopAndColorAuto(), StripChangeColor, strip_, StripCopyColor);
				Strip::AddNumCopy2();
			}
			return *this;
		}

		Strip& operator %= (const int value_) {
			this->m_value %= value_;
			if (this->m_notTemp) {
				Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
				Strip::AddNumCopy1_IntToStrip();
			}
			return *this;
		}
		Strip& operator %= (const Strip& strip_) {
			this->m_value %= strip_.m_value;
			if (this->m_notTemp != strip_.m_notTemp) {
				if (this->m_notTemp) {
					Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
					Strip::AddNumCopy1_IntToStrip();
				}
				else {
					Strip::DrawStrip1(strip_, StripCopyColor);
					Strip::AddNumCopy1_StripToInt();
				}
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(this->SetTopAndColorAuto(), StripChangeColor, strip_, StripCopyColor);
				Strip::AddNumCopy2();
			}
			return *this;
		}

		const int operator + (const int value_) const {
			if (this->m_notTemp) {
				Strip::DrawStrip1(*this, StripCopyColor);
				Strip::AddNumCopy1_StripToInt();
			}
			return this->m_value + value_;
		}
		const int operator + (const Strip& strip_) const {
			if (this->m_notTemp != strip_.m_notTemp) {
				Strip::DrawStrip1(this->m_notTemp ? *this : strip_, StripCopyColor);
				Strip::AddNumCopy1_StripToInt();
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(*this, StripCopyColor, strip_, StripCopyColor); Strip::AddNumCompare2();
				Strip::AddNumCopy2_StripToInt();
			}
			return this->m_value + strip_.m_value;
		}

		const int operator - (const int value_) const {
			if (this->m_notTemp) {
				Strip::DrawStrip1(*this, StripCopyColor);
				Strip::AddNumCopy1_StripToInt();
			}
			return this->m_value - value_;
		}
		const int operator - (const Strip& strip_) const {
			if (this->m_notTemp != strip_.m_notTemp) {
				Strip::DrawStrip1(this->m_notTemp ? *this : strip_, StripCopyColor);
				Strip::AddNumCopy1_StripToInt();
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(*this, StripCopyColor, strip_, StripCopyColor); Strip::AddNumCompare2();
				Strip::AddNumCopy2_StripToInt();
			}
			return this->m_value - strip_.m_value;
		}

		const int operator * (const int value_) const {
			if (this->m_notTemp) {
				Strip::DrawStrip1(*this, StripCopyColor);
				Strip::AddNumCopy1_StripToInt();
			}
			return this->m_value * value_;
		}
		const int operator * (const Strip& strip_) const {
			if (this->m_notTemp != strip_.m_notTemp) {
				Strip::DrawStrip1(this->m_notTemp ? *this : strip_, StripCopyColor);
				Strip::AddNumCopy1_StripToInt();
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(*this, StripCopyColor, strip_, StripCopyColor); Strip::AddNumCompare2();
				Strip::AddNumCopy2_StripToInt();
			}
			return this->m_value * strip_.m_value;
		}

		const int operator / (const int value_) const {
			if (this->m_notTemp) {
				Strip::DrawStrip1(*this, StripCopyColor);
				Strip::AddNumCopy1_StripToInt();
			}
			return this->m_value / value_;
		}
		const int operator / (const Strip& strip_) const {
			if (this->m_notTemp != strip_.m_notTemp) {
				Strip::DrawStrip1(this->m_notTemp ? *this : strip_, StripCopyColor);
				Strip::AddNumCopy1_StripToInt();
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(*this, StripCopyColor, strip_, StripCopyColor); Strip::AddNumCompare2();
				Strip::AddNumCopy2_StripToInt();
			}
			return this->m_value / strip_.m_value;
		}

		const int operator % (const int value_) const {
			if (this->m_notTemp) {
				Strip::DrawStrip1(*this, StripCopyColor);
				Strip::AddNumCopy1_StripToInt();
			}
			return this->m_value % value_;
		}
		const int operator % (const Strip& strip_) const {
			if (this->m_notTemp != strip_.m_notTemp) {
				Strip::DrawStrip1(this->m_notTemp ? *this : strip_, StripCopyColor);
				Strip::AddNumCopy1_StripToInt();
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(*this, StripCopyColor, strip_, StripCopyColor); Strip::AddNumCompare2();
				Strip::AddNumCopy2_StripToInt();
			}
			return this->m_value % strip_.m_value;
		}

		Strip& operator++() {
			++this->m_value;
			if (this->m_notTemp) {
				Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
				Strip::AddNumCopy1_IntToStrip();
			}
			return *this;
		}
		int operator++(int) {
			++this->m_value;
			if (this->m_notTemp) {
				Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
				Strip::AddNumCopy1_IntToStrip();
			}
			return this->m_value - 1;
		}

		Strip& operator--() {
			--this->m_value;
			if (this->m_notTemp) {
				Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
				Strip::AddNumCopy1_IntToStrip();
			}
			return *this;
		}
		int operator--(int) {
			--this->m_value;
			if (this->m_notTemp) {
				Strip::DrawStrip1(this->SetTopAndColorAuto(), StripChangeColor);
				Strip::AddNumCopy1_IntToStrip();
			}
			return this->m_value + 1;
		}

		Strip& CopyWithoutSetColor(const Strip& strip_) {
			this->m_value = strip_.m_value;
			if (this->m_notTemp != strip_.m_notTemp) {
				if (this->m_notTemp) {
					Strip::DrawStrip1(this->SetTopAuto(), StripChangeColor);
					Strip::AddNumCopy1_IntToStrip();
				}
				else {
					Strip::DrawStrip1(strip_, StripCopyColor);
					Strip::AddNumCopy1_StripToInt();
				}
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(this->SetTopAuto(), StripChangeColor, strip_, StripCopyColor);
				Strip::AddNumCopy2();
			}
			return *this;
		}

		Strip& CopyValueAndColor(const Strip& strip_) {
			this->m_value = strip_.m_value;
			this->m_color = strip_.m_color;
			if (this->m_notTemp != strip_.m_notTemp) {
				if (this->m_notTemp) {
					Strip::DrawStrip1(this->SetTopAuto(), StripChangeColor);
					Strip::AddNumCopy1_IntToStrip();
				}
				else {
					Strip::DrawStrip1(strip_, StripCopyColor);
					Strip::AddNumCopy1_StripToInt();
				}
			}
			else if (this->m_notTemp) {
				Strip::DrawStrip2(this->SetTopAuto(), StripChangeColor, strip_, StripCopyColor);
				Strip::AddNumCopy2();
			}
			return *this;
			return *this;
		}

		friend void swap(Strip&, Strip&);
		friend void swap(Strip&, int&);
		friend void swap(int&, Strip&);
		friend void SwapWithoutSetColor(Strip&, Strip&);

	};

	bool operator > (const int value_, const Strip& strip_) {
		if (strip_.m_notTemp) {
			Strip::DrawStrip1(strip_, StripCopyColor);
			Strip::AddNumCompare1();
		}
		return value_ > strip_.m_value;
	}

	bool operator < (const int value_, const Strip& strip_) {
		if (strip_.m_notTemp) {
			Strip::DrawStrip1(strip_, StripCopyColor);
			Strip::AddNumCompare1();
		}
		return value_ < strip_.m_value;
	}

	bool operator >= (const int value_, const Strip& strip_) {
		if (strip_.m_notTemp) {
			Strip::DrawStrip1(strip_, StripCopyColor);
			Strip::AddNumCompare1();
		}
		return value_ >= strip_.m_value;
	}

	bool operator <= (const int value_, const Strip& strip_) {
		if (strip_.m_notTemp) {
			Strip::DrawStrip1(strip_, StripCopyColor);
			Strip::AddNumCompare1();
		}
		return value_ <= strip_.m_value;
	}

	bool operator == (const int value_, const Strip& strip_) {
		if (strip_.m_notTemp) {
			Strip::DrawStrip1(strip_, StripCopyColor);
			Strip::AddNumCompare1();
		}
		return value_ == strip_.m_value;
	}

	bool operator != (const int value_, const Strip& strip_) {
		if (strip_.m_notTemp) {
			Strip::DrawStrip1(strip_, StripCopyColor);
			Strip::AddNumCompare1();
		}
		return value_ != strip_.m_value;
	}

	void swap(Strip& strip1_, Strip& strip2_) {
		std::swap(strip1_.m_value, strip2_.m_value);
		if (strip1_.m_notTemp != strip2_.m_notTemp) {
			Strip::DrawStrip1((strip1_.m_notTemp ? strip1_.SetTopAndColorAuto() : strip2_.SetTopAndColorAuto()), StripChangeColor);
			Strip::AddNumCopy2();
		}
		else if (strip1_.m_notTemp) {
			Strip::DrawStrip2(strip1_.SetTopAndColorAuto(), StripChangeColor, strip2_.SetTopAndColorAuto(), StripChangeColor);
			Strip::AddNumSwap2();
		}
	}
	void swap(Strip& strip_, int& value_) {
		std::swap(strip_.m_value, value_);
		if (strip_.m_notTemp) {
			Strip::DrawStrip1(strip_.SetTopAndColorAuto(), StripChangeColor);
			Strip::AddNumCopy2();
		}
	}
	void swap(int& value_, Strip& strip_) {
		std::swap(strip_.m_value, value_);
		if (strip_.m_notTemp) {
			Strip::DrawStrip1(strip_.SetTopAndColorAuto(), StripChangeColor);
			Strip::AddNumCopy2();
		}
	}
	void SwapWithoutSetColor(Strip& strip1_, Strip& strip2_) {
		std::swap(strip1_.m_value, strip2_.m_value);
		if (strip1_.m_notTemp != strip2_.m_notTemp) {
			Strip::DrawStrip1((strip1_.m_notTemp ? strip1_.SetTopAuto() : strip2_.SetTopAuto()), StripChangeColor);
			Strip::AddNumCopy2();
		}
		else if (strip1_.m_notTemp) {
			Strip::DrawStrip2(strip1_.SetTopAuto(), StripChangeColor, strip2_.SetTopAuto(), StripChangeColor);
			Strip::AddNumSwap2();
		}
	}

}