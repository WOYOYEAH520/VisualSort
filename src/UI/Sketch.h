#pragma once
#include <string>
#include <functional>
#include <Windows.h>
#include <easyx.h>
#include "ConfigManager.h"
#include "DrawingTool.h"
#include <algorithm>
#include <utility>
#include "WideError.h"
#include "Coordinate.h"
#include <cmath>

namespace NVisualSort {

	class Sketch {

	private:

		// 附加绘制函数，Sketch&参数为自身（不要捕获自身Sketch指针，避免移动后的悬空指针风险！）
		std::function<void(Sketch&)> m_additionalDrawFunction;
		std::wstring m_text;                                          // 文本内容
		std::wstring m_typeface = DefaultTypeface;                    // 文本字体
		RECT m_frameRect = {};                                        // 边框矩形区域
		RECT m_textRect = {};                                         // 文本矩形区域
		int m_frameThick = 2;                                         // 边框线粗细
		int m_frameStyle = PS_SOLID;                                  // 边框线样式（默认为实线）
		COLORREF m_frameColor = WHITE;                                // 边框颜色（默认为白色）
		int m_frameRoundSize = 10;                                    // 边框圆角大小
		COLORREF m_backgroundColor = DefaultCanvasColor;              // 背景颜色（默认为默认画布颜色）
		int m_textSize = 0;                                           // 文本大小
		COLORREF m_textColor = WHITE;                                 // 文本颜色
		UINT m_textMode = DT_SINGLELINE | DT_VCENTER | DT_CENTER;     // 文本绘制格式（默认为禁止换行、水平居中与竖直居中）
		bool m_hasFrame = true;                                       // 是否存在边框
		bool m_hasBackground = true;                                  // 是否存在背景

		// 规范化坐标，确保左<右，上<下
		constexpr void NormalizeCoordinates() noexcept {
			if (this->m_frameRect.left > this->m_frameRect.right) std::swap(this->m_frameRect.left, this->m_frameRect.right);
			if (this->m_frameRect.top > this->m_frameRect.bottom) std::swap(this->m_frameRect.top, this->m_frameRect.bottom);
		}

		// 自动设置文本区域（文本区域略小于边框区域）
		void SetTextRectAuto() {
			this->NormalizeCoordinates();

			int width = this->m_frameRect.right - this->m_frameRect.left;
			int height = this->m_frameRect.bottom - this->m_frameRect.top;
			int margin = (std::min)(width, height) / 20; // 使用相对边距

			this->m_textRect = {
				this->m_frameRect.left + margin,
				this->m_frameRect.top + margin,
				this->m_frameRect.right - margin,
				this->m_frameRect.bottom - margin
			};

			this->SetTextSizeAuto();
		}

		// 自动设置合适文本大小
		void SetTextSizeAuto() {

			int textWidth = this->m_textRect.right - this->m_textRect.left;
			int textHeight = this->m_textRect.bottom - this->m_textRect.top;

			if (!this->m_text.empty() && (textWidth <= 0 || textHeight <= 0)) {
				throw WideError(L"Sketch 的文本矩形区域无效！文本内容为：" + (this->m_text.empty() ? L"<空>" : this->m_text));
			}

			GetDrawingTool().ExecuteWithLock([this, textWidth, textHeight]() {

				// 使用二分查找确定最佳字体大小
				int lowSize = 1;
				int highSize = textHeight;
				int bestSize = 1;

				while (lowSize <= highSize) {
					int middleSize = (lowSize + highSize) / 2;
					::settextstyle(middleSize, 0, this->m_typeface.c_str());
					int currentWidth = ::textwidth(this->m_text.c_str());
					int currentHeight = ::textheight(L"Hg"); // 使用包含上下延伸的字符

					if (currentWidth <= textWidth && currentHeight <= textHeight) {
						bestSize = middleSize;
						lowSize = middleSize + 1;
					}
					else {
						highSize = middleSize - 1;
					}
				}
				this->m_textSize = bestSize;
			});

		}

	public:

		Sketch() = default;
		Sketch(const Sketch&) = default;
		Sketch& operator=(const Sketch&) = default;
		Sketch(Sketch&&) = default;
		Sketch& operator=(Sketch&&) = default;

		Sketch(int left_, int top_, int right_, int bottom_, const std::wstring& text_ = L"") :
			m_frameRect({ left_,top_,right_,bottom_ }), m_text(text_) {
			this->SetTextRectAuto();
		}

		Sketch(RECT frame_rect_, const std::wstring& text_ = L"") :m_frameRect(frame_rect_), m_text(text_) {
			this->SetTextRectAuto();
		}

		Sketch& SetSketch(int left_, int top_, int right_, int bottom_, const std::wstring& text_ = L"") {
			this->m_frameRect = { left_,top_,right_,bottom_ };
			this->m_text = text_;
			this->SetTextRectAuto();
			return *this;
		}

		Sketch& SetSketch(RECT frame_rect_, const std::wstring& text_ = L"") {
			this->m_frameRect = frame_rect_;
			this->m_text = text_;
			this->SetTextRectAuto();
			return *this;
		}

		Sketch& SetFrameRect(RECT frame_rect_) {
			this->m_frameRect = frame_rect_;
			this->SetTextRectAuto();
			return *this;
		}
		constexpr RECT GetFrameRect() const noexcept {
			return this->m_frameRect;
		}

		Sketch& SetLeft(int left_) {
			this->m_frameRect.left = left_;
			this->SetTextRectAuto();
			return *this;
		}
		constexpr int GetLeft() const noexcept {
			return this->m_frameRect.left;
		}

		Sketch& SetRight(int right_) {
			this->m_frameRect.right = right_;
			this->SetTextRectAuto();
			return *this;
		}
		Sketch& SetRightWithoutResize(int right_) noexcept {
			this->m_frameRect.right = right_;
			this->NormalizeCoordinates();
			return *this;
		}
		constexpr int GetRight() const noexcept {
			return this->m_frameRect.right;
		}

		Sketch& SetTop(int top_) {
			this->m_frameRect.top = top_;
			this->SetTextRectAuto();
			return *this;
		}
		constexpr int GetTop() const noexcept {
			return this->m_frameRect.top;
		}

		Sketch& SetBottom(int bottom_) {
			this->m_frameRect.bottom = bottom_;
			this->SetTextRectAuto();
			return *this;
		}
		constexpr int GetBottom() const noexcept {
			return this->m_frameRect.bottom;
		}

		Sketch& SetFrameThick(int frame_thick_) noexcept {
			this->m_frameThick = frame_thick_;
			return *this;
		}
		constexpr int GetFrameThick() const noexcept {
			return this->m_frameThick;
		}

		Sketch& SetFrameStyle(int frame_style_) noexcept {
			this->m_frameStyle = frame_style_;
			return *this;
		}
		constexpr int GetFrameStyle() const noexcept {
			return this->m_frameStyle;
		}

		Sketch& SetFrameColor(COLORREF frame_color_) noexcept {
			this->m_frameColor = frame_color_;
			return *this;
		}
		constexpr COLORREF GetFrameColor() const noexcept {
			return this->m_frameColor;
		}

		Sketch& SetFrameRoundSize(int round_size_) noexcept {
			this->m_frameRoundSize = round_size_;
			return *this;
		}
		constexpr int GetFrameRoundSize() const noexcept {
			return this->m_frameRoundSize;
		}

		Sketch& SetHasFrame(bool has_frame_) noexcept {
			this->m_hasFrame = has_frame_;
			return *this;
		}
		constexpr bool GetHasFrame() const noexcept {
			return this->m_hasFrame;
		}

		Sketch& SetBackgroundColor(COLORREF background_color_) noexcept {
			this->m_backgroundColor = background_color_;
			return *this;
		}
		constexpr COLORREF GetBackgroundColor() const noexcept {
			return this->m_backgroundColor;
		}

		Sketch& SetHasBackground(bool has_background_) noexcept {
			this->m_hasBackground = has_background_;
			return *this;
		}
		constexpr bool GetHasBackground() const noexcept {
			return this->m_hasBackground;
		}

		Sketch& SetTextRectWithoutResize(RECT text_rect_) noexcept {
			this->m_textRect = text_rect_;
			// 确保文本矩形在边框矩形内
			this->m_textRect.left = (std::max)(this->m_textRect.left, this->m_frameRect.left);
			this->m_textRect.top = (std::max)(this->m_textRect.top, this->m_frameRect.top);
			this->m_textRect.right = (std::min)(this->m_textRect.right, this->m_frameRect.right);
			this->m_textRect.bottom = (std::min)(this->m_textRect.bottom, this->m_frameRect.bottom);
			return *this;
		}
		Sketch& SetTextRectWithoutResize(int left_, int top_, int right_, int bottom_) noexcept {
			return this->SetTextRectWithoutResize({ left_,top_,right_,bottom_ });
		}
		Sketch& SetTextRect(RECT text_rect_) {
			this->SetTextRectWithoutResize(text_rect_);
			this->SetTextSizeAuto();
			return *this;
		}
		Sketch& SetTextRect(int left_, int top_, int right_, int bottom_) {
			return this->SetTextRect({ left_,top_,right_,bottom_ });
		}
		constexpr RECT GetTextRect() const noexcept {
			return this->m_textRect;
		}

		Sketch& SetTextSize(int text_size_) noexcept {
			this->m_textSize = text_size_;
			return *this;
		}
		constexpr int GetTextSize() const noexcept {
			return this->m_textSize;
		}

		Sketch& SetTextColor(COLORREF text_color_) noexcept {
			this->m_textColor = text_color_;
			return *this;
		}
		constexpr COLORREF GetTextColor() const noexcept {
			return this->m_textColor;
		}

		Sketch& SetText(const std::wstring& text_) {
			this->m_text = text_;
			this->SetTextRectAuto();
			return *this;
		}
		Sketch& SetTextWithoutResize(const std::wstring& text_) {
			this->m_text = text_;
			return *this;
		}
		std::wstring& GetText() noexcept {
			return this->m_text;
		}
		const std::wstring& GetText() const noexcept {
			return this->m_text;
		}

		Sketch& SetTypeface(const std::wstring& typeface_) {
			this->m_typeface = typeface_;
			return *this;
		}
		const std::wstring& GetTypeface() const noexcept {
			return this->m_typeface;
		}

		Sketch& SetTextMode(UINT text_mode_) noexcept {
			this->m_textMode = text_mode_;
			return *this;
		}
		constexpr UINT GetTextMode() const noexcept {
			return this->m_textMode;
		}

		Sketch& SetAdditionalDrawFunction(const std::function<void(Sketch&)>& additional_draw_function_) {
			this->m_additionalDrawFunction = additional_draw_function_;
			return *this;
		}
		std::function<void(Sketch&)>& GetAdditionalDrawFunction()noexcept {
			return this->m_additionalDrawFunction;
		}
		const std::function<void(Sketch&)>& GetAdditionalDrawFunction() const noexcept {
			return this->m_additionalDrawFunction;
		}

		constexpr int GetWidth() const noexcept {
			return this->GetRight() - this->GetLeft();
		}
		constexpr int GetHeight() const noexcept {
			return this->GetBottom() - this->GetTop();
		}

		int GetCenterX() const {
			return static_cast<int>(std::round(static_cast<double>(this->GetLeft() + this->GetRight()) / 2));
		}
		int GetCenterY() const {
			return static_cast<int>(std::round(static_cast<double>(this->GetTop() + this->GetBottom()) / 2));
		}
		Coordinate GetCenterXY() const {
			return {
				static_cast<Coordinate::type_x>(std::round(static_cast<double>(this->GetLeft() + this->GetRight()) / 2)),
				static_cast<Coordinate::type_y>(std::round(static_cast<double>(this->GetTop() + this->GetBottom()) / 2))
			};
		}

		void Flush() const noexcept {
			GetDrawingTool().FlushBatchDraw(this->m_frameRect);
		}

		void DrawSketch(bool is_flush_ = true) {
			// 绘制背景
			if (this->m_hasBackground) {
				if (this->m_hasFrame) {
					GetDrawingTool().FillRoundRect(
						this->m_frameRect, this->m_frameRoundSize, this->m_frameRoundSize,
						this->m_frameThick, this->m_frameStyle,
						this->m_frameColor, this->m_backgroundColor
					);
				}
				else {
					GetDrawingTool().SolidRoundRect(
						this->m_frameRect, this->m_frameRoundSize,
						this->m_frameRoundSize, this->m_backgroundColor
					);
				}
			}
			else if (this->m_hasFrame) {
				GetDrawingTool().RoundRect(
					this->m_frameRect, this->m_frameRoundSize, this->m_frameRoundSize,
					this->m_frameThick, this->m_frameStyle, this->m_frameColor
				);
			}

			// 绘制文本
			if (!this->m_text.empty()) {
				GetDrawingTool().DrawText_(
					this->m_text, this->m_textRect, this->m_textSize,
					this->m_textColor, this->m_textMode, this->m_typeface
				);
			}

			// 执行额外的绘制函数
			if (this->m_additionalDrawFunction) {
				this->m_additionalDrawFunction(*this);
			}

			// 如果需要刷新
			if (is_flush_) {
				this->Flush();
			}
		}

	};

}