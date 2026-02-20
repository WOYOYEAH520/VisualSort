#pragma once
#include "Fraction.h"
#include "Sketch.h"
#include <string>
#include "Button.h"
#include <algorithm>
#include "ConfigManager.h"
#include <Windows.h>
#include <vector>
#include <utility>
#include "Coordinate.h"
#include <functional>
#include "WideError.h"
#include <easyx.h>

namespace NVisualSort {

	class InputBox {

	private:

		inline static constexpr Fraction MinWidth{ 1,2 }; // 最小宽度为画布宽度的1/2
		inline static constexpr Fraction MinHeight{ 1,2 }; // 最小高度为画布高度的1/2
		inline static constexpr Fraction MaxWidth{ 8,9 }; // 最大宽度为画布宽度的8/9
		inline static constexpr Fraction MaxHeight{ 8,9 }; // 最大高度为画布高度的8/9
		inline static constexpr Fraction AspectRatio{ 1,2 }; // 高宽比为1:2

		ButtonSequence m_buttons; // 按钮序列
		Sketch m_mainBox;         // 主对话框
		Sketch m_titleBox;        // 标题区域
		Sketch m_contentBox;      // 内容区域
		Sketch m_inputBox;        // 输入区域
		std::wstring m_inputText; // 输入文本
		size_t m_maxNum = 99999;  // 输入最大值限制

		void ClampMainRect() {
			using F = Fraction;
			F adjustedWidth = std::clamp(
				F(this->m_mainBox.GetWidth()),
				InputBox::MinWidth * GetConfigManager().GetWidth(),
				InputBox::MaxWidth * GetConfigManager().GetWidth()
			);
			F adjustedHeight = std::clamp(
				F(this->m_mainBox.GetHeight()),
				InputBox::MinHeight * GetConfigManager().GetHeight(),
				InputBox::MaxHeight * GetConfigManager().GetHeight()
			);
			if (adjustedHeight > adjustedWidth * InputBox::AspectRatio) {
				adjustedHeight = adjustedWidth * InputBox::AspectRatio;
			} else {
				adjustedWidth = adjustedHeight / InputBox::AspectRatio;
			}
			this->m_mainBox.SetFrameRect(RECT(
				GetConfigManager().GetCenterX() - adjustedWidth / 2,
				GetConfigManager().GetCenterY() - adjustedHeight / 2,
				GetConfigManager().GetCenterX() + adjustedWidth / 2,
				GetConfigManager().GetCenterY() + adjustedHeight / 2)
			);
		}

		void SetButtonsAuto() {
			constexpr const wchar_t* buttonString[] = { L"1",L"2",L"3",L"4",L"5",L"6",L"7",L"8",L"9",L"确认",L"0",L"删除" };
			std::vector<Button> buttons(13);
			using F = Fraction;
			F leftMargin(7, 13); // 左边距为 dialog 窗口宽度的7/13
			F bWidth(3, 26); // 按钮宽度为 dialog 窗口宽度的3/26
			F horiGap(1, 26); // 按钮水平间距为 dialog 窗口宽度的1/26
			F topMargin(1, 13); // 上边距为 dialog 窗口高度的1/13
			F bHeight(2, 13); // 按钮高度为 dialog 窗口高度的2/13
			F vertGap(1, 13); // 按钮垂直间距为 dialog 窗口高度的1/13
			size_t horiCount = 3; // 每行按钮数量
			for (size_t i = 0; i < 12; ++i) {
				RECT tempRect = ComputeRect(this->m_mainBox.GetFrameRect(),
					leftMargin + (bWidth + horiGap) * (i % horiCount),
					topMargin + (bHeight + vertGap) * (i / horiCount),
					leftMargin + (bWidth + horiGap) * (i % horiCount) + bWidth,
					topMargin + (bHeight + vertGap) * (i / horiCount) + bHeight
				);
				buttons[i].SetButton(tempRect, buttonString[i]);
				if (i != 9 && i != 11) { // 数字按钮
					buttons[i].SetReleaseFunc([this](Button& button_, ExMessage) {
						if (this->m_inputBox.GetText() == L"0") {
							this->m_inputBox.SetTextWithoutResize(button_.GetSketch().GetText());
						}
						else {
							this->m_inputBox.SetTextWithoutResize(this->m_inputBox.GetText() + button_.GetSketch().GetText());
							if (std::stoull(this->m_inputBox.GetText()) > this->m_maxNum) {
								this->m_inputBox.SetTextWithoutResize(std::to_wstring(this->m_maxNum));
							}
						}
						this->m_inputBox.DrawSketch();
						Button::GetDefaultHoverDrawFunction()(button_, {});
					});
				}
			}
			buttons[11].SetReleaseFunc([this](Button& button_, ExMessage) {
				if (!this->m_inputBox.GetText().empty()) {
					this->m_inputBox.GetText().pop_back();
					this->m_inputBox.DrawSketch();
				}
				Button::GetDefaultHoverDrawFunction()(button_, {});
			});
			this->m_buttons.SetButtons(std::move(buttons));
			this->SetCrossFunc();
		}

		void SetBoxesAuto() {
			using F = Fraction;
			F leftMargin(1, 26);
			F topMargin(1, 13);
			F boxRight(6, 13);
			F boxBottom(3, 13);
			this->m_titleBox.SetFrameRect(ComputeRect(this->m_mainBox.GetFrameRect(),
				leftMargin,topMargin,boxRight,boxBottom
			));
			this->m_titleBox.SetTextSize((boxBottom - topMargin) * this->m_mainBox.GetHeight());
			this->m_titleBox.SetHasFrame(false);
			this->m_titleBox.SetTextMode(DT_LEFT);
			topMargin = { 3,13 };
			boxBottom = { 9,13 };
			this->m_contentBox.SetFrameRect(ComputeRect(this->m_mainBox.GetFrameRect(),
				leftMargin, topMargin, boxRight, boxBottom
			));
			this->m_contentBox.SetTextSize((boxBottom - topMargin) / 7 * this->m_mainBox.GetHeight());
			this->m_contentBox.SetHasFrame(false);
			this->m_contentBox.SetTextMode(DT_LEFT | DT_WORDBREAK);
			topMargin = { 10,13 };
			boxBottom = { 12,13 };
			this->m_inputBox.SetFrameRect(ComputeRect(this->m_mainBox.GetFrameRect(),
				leftMargin, topMargin, boxRight, boxBottom
			));
			this->m_inputBox.SetTextSize((boxBottom - topMargin) * F(9, 10) * this->m_mainBox.GetHeight());
			this->m_inputBox.SetTextMode(DT_LEFT);
		}

	public:

		InputBox(RECT main_box_rect_ = { 0,0,MAXSHORT,MINSHORT }) :m_buttons(0) {
			this->SetMainBoxRect(main_box_rect_);
			this->SetExcutFunc(nullptr);
		}
		InputBox(const InputBox&) = delete;
		InputBox& operator=(const InputBox&) = delete;
		InputBox(InputBox&&) = default;
		InputBox& operator=(InputBox&&) = default;

		InputBox& SetMainBoxRect(RECT main_box_rect_) {
			this->m_mainBox.SetFrameRect(main_box_rect_);
			this->ClampMainRect();
			this->SetButtonsAuto();
			this->SetBoxesAuto();
			return *this;
		}

		InputBox& SetTitleText(const std::wstring& title_text_) {
			this->m_titleBox.SetText(title_text_);
			return *this;
		}

		InputBox& SetContentText(const std::wstring& content_text_) {
			this->m_contentBox.SetTextWithoutResize(content_text_);
			return *this;
		}

		void DrawInputBox(bool is_flush_ = true) {
			this->m_mainBox.DrawSketch(false);
			this->m_titleBox.DrawSketch(false);
			this->m_contentBox.DrawSketch(false);
			this->m_inputBox.DrawSketch(false);
			this->m_buttons.DrawButtons(is_flush_);
		}

		size_t GetInputNum() const {
			if (this->m_inputText == L"") {
				return 0;
			}
			return std::stoull(this->m_inputText);
		}

		InputBox& SetMaxNum(size_t max_num_) noexcept {
			this->m_maxNum = max_num_;
			return *this;
		}

		size_t GetMaxNum() const noexcept {
			return this->m_maxNum;
		}

		InputBox& SetExcutFunc(const std::function<void(Button&, ExMessage)>& execut_func_) {
			if (this->m_buttons.GetButtonNum() != 13) {
				throw WideError(L"InputBox 未初始化！");
			}
			if (execut_func_) {
				this->m_buttons.GetButtons()[9].SetReleaseFunc([this, execut_func_](Button& button_, ExMessage) {
					this->m_inputText = this->m_inputBox.GetText();
					execut_func_(button_, {});
				});
			}
			return *this;
		}

		InputBox& SetCrossFunc(const std::function<void()>& cross_func_ = nullptr) {
			if (cross_func_) {
				this->m_buttons.SetButtonAsCross(12,
					Coordinate(this->m_mainBox.GetRight(), this->m_mainBox.GetTop()),
					Fraction(2, 13) * this->m_mainBox.GetHeight() / 2,
					[this, cross_func_](Button&) {
						this->m_inputText = this->m_mainBox.GetText();
						cross_func_();
					}
				);
			}
			else {
				this->m_buttons.SetButtonAsCross(12,
					Coordinate(this->m_mainBox.GetRight(), this->m_mainBox.GetTop()),
					Fraction(2, 13) * this->m_mainBox.GetHeight() / 2,
					[this, cross_func_](Button&) {
						this->m_inputText = this->m_mainBox.GetText();
					}
				);
			}
			return *this;
		}

		void RunBlockInputLoop() {
			this->DrawInputBox(false);
			this->m_buttons.RunBlockButtonLoop();
		}

		void RunNonBlockInputLoop() {
			this->DrawInputBox(false);
			this->m_buttons.RunNonBlockButtonLoop();
		}

		InputBox& SetExitFlag(bool exit_flag_) {
			this->m_buttons.SetExitFlag(exit_flag_);
			return *this;
		}

	};

}