#pragma once
#include <atomic>
#include <random>
#include <Windows.h>
#include <algorithm>
#include <utility>
#include <type_traits>
#include "Coordinate.h"
#include "Fraction.h"
#include <easyx.h>
#include "WideError.h"

namespace NVisualSort {

	constexpr unsigned int DefaultWidth = 800;
	constexpr unsigned int DefaultHeight = 600;
	constexpr COLORREF DefaultCanvasColor = RGB(0x33, 0x33, 0x33); // 默认画布颜色为灰色
	constexpr wchar_t DefaultTypeface[] = L"楷体"; // 默认字体为楷体，楷体好看捏:）

	// 配置管理器类，使用单例模式
	class ConfigManager {

	private:

		std::atomic<unsigned int> m_width = DefaultWidth;
		std::atomic<unsigned int> m_height = DefaultHeight;
		std::atomic<unsigned int> m_minWidth = DefaultWidth;
		std::atomic<unsigned int> m_minHeight = DefaultHeight;
		std::atomic<unsigned int> m_maxWidth = DefaultWidth;
		std::atomic<unsigned int> m_maxHeight = DefaultHeight;
		std::atomic<COLORREF> m_canvasColor = DefaultCanvasColor;

		// 单例模式下，禁止拷贝和移动
		ConfigManager() noexcept {
			this->SetMinMaxSizeAuto();
		}
		ConfigManager(const ConfigManager&) = delete;
		ConfigManager& operator=(const ConfigManager&) = delete;
		ConfigManager(ConfigManager&&) = delete;
		ConfigManager& operator=(ConfigManager&&) = delete;

		void SetMinMaxSizeAuto() noexcept {
			DEVMODE devMode = {};
			EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
			this->m_minWidth = devMode.dmPelsWidth * 2 / 5;
			this->m_maxWidth = devMode.dmPelsWidth;
			this->m_minHeight = devMode.dmPelsHeight * 2 / 5;
			this->m_maxHeight = devMode.dmPelsHeight;
			this->m_width = std::clamp(this->m_width.load(), this->m_minWidth.load(), this->m_maxWidth.load());
			this->m_height = std::clamp(this->m_height.load(), this->m_minHeight.load(), this->m_maxHeight.load());
		}

	public:

		// 获取宽度
		Fraction GetWidth() const noexcept {
			return Fraction(this->m_width.load(std::memory_order_acquire));
		}
		// 设置宽度
		void SetWidth(unsigned int width_) noexcept {
			width_ = std::clamp(width_, this->m_minWidth.load(), this->m_maxWidth.load());
			this->m_width.store(width_, std::memory_order_release);
		}

		Fraction GetMinWidth() const noexcept {
			return Fraction(this->m_minWidth.load(std::memory_order_acquire));
		}

		Fraction GetMaxWidth() const noexcept {
			return Fraction(this->m_maxWidth.load(std::memory_order_acquire));
		}

		// 获取高度
		Fraction GetHeight() const noexcept {
			return Fraction(this->m_height.load(std::memory_order_acquire));
		}
		// 设置高度
		void SetHeight(unsigned int height_) noexcept {
			height_ = std::clamp(height_, this->m_minHeight.load(), this->m_maxHeight.load());
			this->m_height.store(height_, std::memory_order_release);
		}

		Fraction GetMinHeight() const noexcept {
			return Fraction(this->m_minHeight.load(std::memory_order_acquire));
		}

		Fraction GetMaxHeight() const noexcept {
			return Fraction(this->m_maxHeight.load(std::memory_order_acquire));
		}

		Fraction GetMaxClientHeight() const {
			HWND hwnd = GetHWnd();
			if (!hwnd) {
				throw WideError(L"窗口还未创建");
			}

			RECT workArea = {};
			SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
			int screenWorkHeight = workArea.bottom - workArea.top;

			// 获取窗口非客户区各部分高度
			RECT rcClient = { 0, 0, 0, 0 };
			DWORD style = WS_OVERLAPPEDWINDOW;  // 普通窗口样式
			DWORD exStyle = 0;
			AdjustWindowRectEx(&rcClient, style, FALSE, exStyle);

			int topBorderHeight = -rcClient.top;
			int bottomBorderHeight = rcClient.bottom;
			int totalNonClientHeight = topBorderHeight + bottomBorderHeight;

			// 只减去上边框的情况
			int maxClientHeight_topOnly = screenWorkHeight - topBorderHeight;
			return Fraction(maxClientHeight_topOnly);
		}

		// 设置宽度和高度
		void SetDimensions(unsigned int width_, unsigned  int height_) noexcept {
			width_ = std::clamp(width_, this->m_minWidth.load(), this->m_maxWidth.load());
			height_ = std::clamp(height_, this->m_minHeight.load(), this->m_maxHeight.load());
			this->m_width.store(width_, std::memory_order_release);
			this->m_height.store(height_, std::memory_order_release);
		}
		void SetDimensions(std::pair<unsigned int, unsigned int> size_) noexcept {
			size_.first = std::clamp(size_.first, this->m_minWidth.load(), this->m_maxWidth.load());
			size_.second = std::clamp(size_.second, this->m_minHeight.load(), this->m_maxHeight.load());
			this->m_width.store(size_.first, std::memory_order_release);
			this->m_height.store(size_.second, std::memory_order_release);
		}

		Fraction GetCenterX() const noexcept {
			return this->GetWidth() / 2;
		}

		Fraction GetCenterY() const noexcept {
			return this->GetHeight() / 2;
		}

		// 获取中心坐标
		Coordinate GetCenterXY() const noexcept {
			return Coordinate(this->GetCenterX(), this->GetCenterY());
		}

		// 获取画布颜色
		COLORREF GetCanvasColor() const noexcept {
			return this->m_canvasColor.load(std::memory_order_acquire);
		}
		// 设置画布颜色
		void SetCanvasColor(COLORREF color_) noexcept {
			this->m_canvasColor.store(color_, std::memory_order_release);
		}

		RECT GetCanvasRect() const noexcept {
			return { 0, 0, static_cast<LONG>(this->GetWidth()), static_cast<LONG>(this->GetHeight()) };
		}

		// 生成随机数（线程安全）
		unsigned int GenerateRandom() const {
			static thread_local std::mt19937 engine(std::random_device{}());
			return engine();
		}

		// 生成指定范围的随机数
		template<typename T = int>
		T GenerateRandomRange(T min_value_, T max_value_) const {
			static thread_local std::mt19937 engine(std::random_device{}());
			if constexpr (std::is_integral_v<T>) {
				std::uniform_int_distribution<T> dist(min_value_, max_value_);
				return dist(engine);
			}
			else {
				std::uniform_real_distribution<T> dist(min_value_, max_value_);
				return dist(engine);
			}
		}

		// 获取配置管理器实例（单例模式）
		friend inline ConfigManager& GetConfigManager() noexcept;

	};

	inline ConfigManager& GetConfigManager() noexcept {
		static ConfigManager instance;
		return instance;
	}

}