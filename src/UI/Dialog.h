#pragma once
#include "Sketch.h"
#include "Button.h"
#include <optional>
#include "Fraction.h"
#include "ConfigManager.h"
#include "DrawingTool.h"
#include <easyx.h>
#include <Windows.h>
#include <algorithm>
#include <string>
#include <vector>
#include <functional>
#include "WideError.h"
#include "Coordinate.h"
#include <concepts>
#include <type_traits>

namespace NVisualSort {

	class Dialog {

	private:

		inline static constexpr Fraction MinWidth{ 1,3 }; // 最小宽度为画布宽度的1/3
		inline static constexpr Fraction MinHeight{ 1,5 }; // 最小高度为画布高度的1/5
		inline static constexpr Fraction MaxWidth{ 8,9 }; // 最大宽度为画布宽度的8/9
		inline static constexpr Fraction MaxHeight{ 8,9 }; // 最大高度为画布高度的8/9
		inline static constexpr Fraction Margin{ 1,20 }; // 文本上下区域和边框上下区域的边距为画布高度的1/20
		inline static constexpr Fraction TextSize{ 1,20 }; // 文本大小为画布高度的1/20

		Sketch m_mainBox;
		ButtonSequence m_buttons;
		std::optional<size_t> m_crossIndex;

		static Fraction GetMinWidth() noexcept {
			return Dialog::MinWidth * GetConfigManager().GetWidth();
		}

		static Fraction GetMinHeight() noexcept {
			return Dialog::MinHeight * GetConfigManager().GetHeight();
		}

		static Fraction GetMaxWidth() noexcept {
			return Dialog::MaxWidth * GetConfigManager().GetWidth();
		}

		static Fraction GetMaxHeight() noexcept {
			return Dialog::MaxHeight * GetConfigManager().GetHeight();
		}

		static Fraction GetTextSize() noexcept {
			return Dialog::TextSize * GetConfigManager().GetHeight();
		}

		static Fraction GetMargin() noexcept {
			return Dialog::Margin * GetConfigManager().GetHeight();
		}

		void SetWidthAuto() {
			GetDrawingTool().ExecuteWithLock([this]() {
				::settextstyle(Dialog::GetTextSize(), 0, this->m_mainBox.GetTypeface().c_str());
				int textWidth = ::textwidth(this->m_mainBox.GetText().c_str());
				textWidth = std::clamp(textWidth, static_cast<int>(Dialog::GetMinWidth()), static_cast<int>(Dialog::GetMaxWidth()));
				RECT mainRect = {};
				using F = Fraction;
				mainRect = ComputeRect(
					GetConfigManager().GetCanvasRect(),
					F(1, 2) - F(textWidth) / GetConfigManager().GetWidth() / 2,
					F(1, 2) - F(this->m_mainBox.GetHeight()) / GetConfigManager().GetHeight() / 2,
					F(1, 2) + F(textWidth) / GetConfigManager().GetWidth() / 2,
					F(1, 2) + F(this->m_mainBox.GetHeight()) / GetConfigManager().GetHeight() / 2
				); // 使用文本宽度并水平居中
				this->m_mainBox.SetFrameRect(mainRect);
			});
		}

		void SetHeightAuto() {
			GetDrawingTool().ExecuteWithLock([this]() {
				::settextstyle(Dialog::GetTextSize(), 0, this->m_mainBox.GetTypeface().c_str());
				RECT tempRect = this->m_mainBox.GetFrameRect();
				int textHeight = ::drawtext(this->m_mainBox.GetText().c_str(), &tempRect, DT_CALCRECT | DT_WORDBREAK);
				textHeight = textHeight + Dialog::GetMargin() * 2; // 添加边距
				textHeight = std::clamp(textHeight, static_cast<int>(Dialog::GetMinHeight()), static_cast<int>(Dialog::GetMaxHeight()));
				using F = Fraction;
				RECT mainRect = ComputeRect(
					GetConfigManager().GetCanvasRect(),
					F(1, 2) - F(this->m_mainBox.GetWidth()) / GetConfigManager().GetWidth() / 2,
					F(1, 2) - F(textHeight) / GetConfigManager().GetHeight() / 2,
					F(1, 2) + F(this->m_mainBox.GetWidth()) / GetConfigManager().GetWidth() / 2,
					F(1, 2) + F(textHeight) / GetConfigManager().GetHeight() / 2
				); // 使用文本高度并垂直居中
				this->m_mainBox.SetFrameRect(mainRect);
			});
		}

	public:

		template<typename T>
			requires (std::constructible_from<std::wstring, T> &&
		!std::same_as<std::remove_cvref_t<T>, WideError>)
			explicit Dialog(T&& msg)
			noexcept(std::is_nothrow_constructible_v<std::wstring, T>):m_buttons(1) {
			this->SetText({ msg });
			this->SetCrossAuto();
		}

		Dialog(const std::vector<std::wstring>& messages_ = {}):m_buttons(1) {
			this->SetText(messages_);
			this->SetCrossAuto();
		}

		void SetText(const std::vector<std::wstring>& messages_) {
			this->m_mainBox.SetFrameRect(GetConfigManager().GetCanvasRect());
			std::wstring allMessage;
			for (const auto& line : messages_) {
				allMessage += line + L"\n";
			}
			this->m_mainBox.SetTextWithoutResize(allMessage);
			this->SetWidthAuto();
			this->SetHeightAuto();
			this->m_mainBox.SetTextMode(DT_CENTER | DT_WORDBREAK).
				SetTextSize(Dialog::GetTextSize()).SetTextRectWithoutResize(
					this->m_mainBox.GetLeft(),
					this->m_mainBox.GetTop() + Dialog::GetMargin(),
					this->m_mainBox.GetRight(),
					this->m_mainBox.GetBottom() - Dialog::GetMargin()
				);
		}

		Sketch& GetMainBox() noexcept {
			return this->m_mainBox;
		}

		const Sketch& GetMainBox() const noexcept {
			return this->m_mainBox;
		}

		ButtonSequence& GetButtons() noexcept {
			return this->m_buttons;
		}

		const ButtonSequence& GetButtons() const noexcept {
			return this->m_buttons;
		}

		size_t SetCrossAuto(const std::function<void()>& callback_ = nullptr) {
			if (this->m_crossIndex.has_value()) {
				this->m_buttons.SetButtonAsCross(this->m_crossIndex.value(),
					Coordinate(this->m_mainBox.GetFrameRect().right, this->m_mainBox.GetFrameRect().top),
					Dialog::GetMargin()
				);
			}
			else {
				this->m_buttons.AddButtonAsCross(
					Coordinate(this->m_mainBox.GetFrameRect().right, this->m_mainBox.GetFrameRect().top),
					Dialog::GetMargin()
				);
				this->m_crossIndex = this->m_buttons.GetButtonNum() - 1;
			}
			return this->m_crossIndex.value();
		}

		template<typename T>
		Dialog& AddButton(T&& button_) {
			this->m_buttons.AddButton(std::forward<T>(button_));
			return *this;
		}

		void DrawDialog(bool is_flush_ = true) {
			this->m_mainBox.DrawSketch(false);
			this->m_buttons.DrawButtons(is_flush_);
		}

		void RunBlockDialog() {
			this->m_mainBox.DrawSketch(false);
			this->m_buttons.RunBlockButtonLoop();
		}

		void RunNonBlockDialog() {
			this->m_mainBox.DrawSketch(false); // 先绘制对话框
			this->m_buttons.RunNonBlockButtonLoop(); // 循环事件自动绘制按钮
		}

	};

}