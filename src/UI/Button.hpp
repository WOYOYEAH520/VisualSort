#pragma once
#include "Sketch.hpp"
#include <Windows.h>
#include <functional>
#include <easyx.h>
#include "ScopeGuard.hpp"
#include <vector>
#include <atomic>
#include <memory>
#include <string>
#include <optional>
#include "WideError.hpp"
#include "DrawingTool.hpp"
#include <list>
#include <mutex>
#include <utility>
#include "Coordinate.hpp"
#include "Fraction.hpp"
#include <condition_variable>
#include <algorithm>
#include "Channel.hpp"
#include "MouseMessageSource.hpp"
#include <thread>

namespace NVisualSort {

	constexpr COLORREF HoverOffset = RGB(0x66, 0x66, 0x66); // 鼠标悬停时的背景颜色偏移
	constexpr COLORREF PressOffset = RGB(0x33, 0x33, 0x33); // 鼠标按下时的背景颜色偏移

	class Button : public Sketch {

		friend class ButtonSequence;

	public:

		constexpr inline static size_t Hover = 0; // 鼠标悬停事件索引
		constexpr inline static size_t Press = 1; // 鼠标按下事件索引
		constexpr inline static size_t Release = 2; // 鼠标释放事件索引
		constexpr inline static size_t Leave = 3; // 鼠标离开事件索引
		constexpr inline static size_t Drag = 4; // 鼠标拖拽事件索引

	private:

		static void DefaultHoverDrawFunction(Button& button_, ExMessage) {
			if (!button_.GetHasBackground()) {
				return; // 如果没有背景，就不需要改变颜色
			}
			COLORREF originalColor = button_.GetBackgroundColor();
			ScopeGuard restoreColorGuard([&button_, originalColor]() {
				button_.SetBackgroundColor(originalColor);
				});
			button_.SetBackgroundColor(HoverOffset + originalColor);
			button_.DrawSketch();
		}

		static void DefaultPressDrawFunction(Button& button_, ExMessage) {
			if (!button_.GetHasBackground()) {
				return; // 如果没有背景，就不需要改变颜色
			}
			COLORREF originalColor = button_.GetBackgroundColor();
			ScopeGuard restoreColorGuard([&button_, originalColor]() {
				button_.SetBackgroundColor(originalColor);
				});
			button_.SetBackgroundColor(PressOffset + originalColor);
			button_.DrawSketch();
		}

		static void DefaultLeaveDrawFunction(Button& button_, ExMessage) {
			button_.DrawSketch();
		}

		// 存储鼠标事件对应的回调函数，参数是 Button 自身和鼠标消息
		// 默认为悬停和按下事件提供默认绘制函数，释放事件默认为空（需要用户设置），离开事件默认为重绘按钮以恢复原状
		// 索引对应关系：0 - Hover，1 - Press，2 - Release，3 - Leave，4 - Drag
		// 不要捕获自身 Button 的 this 指针，避免移动后的悬空指针风险！回调函数会自动通过参数传入 Button 引用
		std::vector<std::function<void(Button&, ExMessage)>> m_callbackFuncs = {
			Button::DefaultHoverDrawFunction,
			Button::DefaultPressDrawFunction,
			nullptr,
			Button::DefaultLeaveDrawFunction,
			nullptr
		};

		// 根据上一次鼠标消息和这一次的鼠标消息，判断鼠标事件类型
		std::optional<size_t> GetMouseEventType(const ExMessage& last_message_, const ExMessage& current_message_) const noexcept {
			bool wasIn = this->IsMouseInButton(last_message_);
			bool isIn = this->IsMouseInButton(current_message_);

			// 只在按钮内时处理按键相关事件
			if (isIn) {
				bool lastDown = last_message_.lbutton;
				bool currDown = current_message_.lbutton;

				// 按下：左键从抬起→按下（或在按钮外按下后移入）
				if (currDown && (!lastDown || !wasIn)) {
					return Button::Press;
				}
				// 释放：左键从按下→抬起
				if (lastDown && !currDown) {
					return Button::Release;
				}
				// 拖拽：左键持续按下且移动
				if (lastDown && currDown && (last_message_.x != current_message_.x || last_message_.y != current_message_.y)) {
					return Button::Drag;
				}
				// 悬停：鼠标进入按钮区域
				if ((!wasIn) || (lastDown && !currDown)) {
					return Button::Hover;
				}
			}
			else if (wasIn) {
				return Button::Leave;
			}

			return std::nullopt;
		}

	public:

		Button() = default; // 因为添加了其他构造函数，必须显式保留默认构造

		Button(int left_, int top_, int right_, int bottom_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) {
			this->SetButton(left_, top_, right_, bottom_, text_, release_func_);
		}

		Button& SetButton(int left_, int top_, int right_, int bottom_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) {
			this->SetFrameRect({ left_, top_, right_, bottom_ }).SetText(text_);
			this->m_callbackFuncs[Button::Release] = release_func_;
			return *this;
		}

		Button& SetButton(RECT rect_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) {
			this->SetFrameRect(rect_).SetText(text_);
			this->m_callbackFuncs[Button::Release] = release_func_;
			return *this;
		}

		Button& SetHoverFunc(const std::function<void(Button&, ExMessage)>& hover_func_) {
			this->m_callbackFuncs[Button::Hover] = hover_func_;
			return *this;
		}
		const std::function<void(Button&, ExMessage)>& GetHoverFunc() const noexcept {
			return this->m_callbackFuncs[Button::Hover];
		}

		Button& SetPressFunc(const std::function<void(Button&, ExMessage)>& press_func_) {
			this->m_callbackFuncs[Button::Press] = press_func_;
			return *this;
		}
		const std::function<void(Button&, ExMessage)>& GetPressFunc() const noexcept {
			return this->m_callbackFuncs[Button::Press];
		}

		Button& SetReleaseFunc(const std::function<void(Button&, ExMessage)>& release_func_) {
			this->m_callbackFuncs[Button::Release] = release_func_;
			return *this;
		}
		const std::function<void(Button&, ExMessage)>& GetReleaseFunc() const noexcept {
			return this->m_callbackFuncs[Button::Release];
		}

		Button& SetLeaveFunc(const std::function<void(Button&, ExMessage)>& leave_func_) {
			this->m_callbackFuncs[Button::Leave] = leave_func_;
			return *this;
		}
		const std::function<void(Button&, ExMessage)>& GetLeaveFunc() const noexcept {
			return this->m_callbackFuncs[Button::Leave];
		}

		Button& SetDragFunc(const std::function<void(Button&, ExMessage)>& drag_func_) {
			this->m_callbackFuncs[Button::Drag] = drag_func_;
			return *this;
		}
		const std::function<void(Button&, ExMessage)>& GetDragFunc() const noexcept {
			return this->m_callbackFuncs[Button::Drag];
		}

		static auto GetDefaultHoverDrawFunction() noexcept -> void(*)(Button&, ExMessage) {
			return Button::DefaultHoverDrawFunction;
		}

		static auto GetDefaultPressDrawFunction() noexcept -> void(*)(Button&, ExMessage) {
			return Button::DefaultPressDrawFunction;
		}

		static auto GetDefaultLeaveDrawFunction() noexcept -> void(*)(Button&, ExMessage) {
			return Button::DefaultLeaveDrawFunction;
		}

		constexpr bool IsMouseInButton(const ExMessage& mouse_message_) const noexcept {
			return mouse_message_.x >= this->GetLeft() &&
				mouse_message_.x <= this->GetRight() &&
				mouse_message_.y >= this->GetTop() &&
				mouse_message_.y <= this->GetBottom();
		}

		void SetCross(Coordinate center_, Fraction size_, std::shared_ptr<std::atomic<bool>> exit_flag_, const std::function<void(Button&)>& last_work_ = nullptr) {
			this->SetHasBackground(false).SetHasFrame(false).SetText(L"");
			this->SetFrameRect({
				center_.x - size_,
				center_.y - size_,
				center_.x + size_,
				center_.y + size_
				});
			int x = center_.x;
			int y = center_.y;
			Fraction biSize = size_ / 2;
			Fraction triSize = size_ / 3;
			std::vector<Coordinate> crossPoints = {
				Coordinate(x, y - triSize),
				Coordinate(x + biSize, y - size_),
				Coordinate(x + size_, y - biSize),
				Coordinate(x + triSize, y),
				Coordinate(x + size_, y + biSize),
				Coordinate(x + biSize, y + size_),
				Coordinate(x, y + triSize),
				Coordinate(x - biSize, y + size_),
				Coordinate(x - size_, y + biSize),
				Coordinate(x - triSize, y),
				Coordinate(x - size_, y - biSize),
				Coordinate(x - biSize, y - size_)
			};
			this->SetAdditionalDrawFunction([crossPoints](Sketch& sketch_) {
				GetDrawingTool().SolidPolygon(crossPoints, RED);
				sketch_.Flush();
				});
			this->SetHoverFunc([crossPoints](Button& button_, ExMessage) {
				GetDrawingTool().SolidPolygon(crossPoints, HSVtoRGB(0, 1, 1));
				button_.Flush();
				}).SetPressFunc([crossPoints](Button& button_, ExMessage) {
					GetDrawingTool().SolidPolygon(crossPoints, HSVtoRGB(0, 1, static_cast<float>(0.8)));
					button_.Flush();
					}).SetLeaveFunc([](Button& button_, ExMessage) {
						button_.DrawSketch();
						}).SetDragFunc(nullptr);
					if (last_work_) {
						this->SetReleaseFunc([exit_flag_, last_work_](Button& button_, ExMessage) {
							exit_flag_->store(true, std::memory_order_release);
							button_.Flush();
							last_work_(button_);
							});
					}
					else {
						this->SetReleaseFunc([exit_flag_](Button& button_, ExMessage) {
							exit_flag_->store(true, std::memory_order_release);
							button_.Flush();
							});
					}
		}

		void SetSwitch(RECT rect_, std::shared_ptr<bool> switch_ptr_, const std::function<void()> other_work_ = nullptr) {
			this->SetFrameRect(rect_).SetHasBackground(false).SetHasFrame(false).SetText(L"");
			this->SetAdditionalDrawFunction([switch_ptr_](Sketch& sketch_) {
				int knobRadius = (sketch_.GetRight() - sketch_.GetLeft()) / 2;
				if (*switch_ptr_) {
					GetDrawingTool().SolidRoundRect(sketch_.GetFrameRect(), knobRadius, knobRadius, GREEN);
					GetDrawingTool().SolidCircle(Coordinate(sketch_.GetRight() - knobRadius, sketch_.GetCenterY()), knobRadius, WHITE);
				}
				else {
					GetDrawingTool().SolidRoundRect(sketch_.GetFrameRect(), knobRadius, knobRadius, RGB(0x1, 0x1, 0x1));
					GetDrawingTool().SolidCircle(Coordinate(sketch_.GetLeft() + knobRadius, sketch_.GetCenterY()), knobRadius, WHITE);
				}
				});
			this->SetHoverFunc(nullptr).SetPressFunc(nullptr).SetLeaveFunc(nullptr).SetDragFunc(nullptr);
			if (other_work_) {
				this->SetReleaseFunc([switch_ptr_, other_work_](Button& button_, ExMessage) {
					*switch_ptr_ = !(*switch_ptr_);
					button_.DrawSketch();
					other_work_();
					});
			}
			else {
				this->SetReleaseFunc([switch_ptr_](Button& button_, ExMessage) {
					*switch_ptr_ = !(*switch_ptr_);
					button_.DrawSketch();
					});
			}
		}

		void SetSwitch(RECT rect_, bool& switch_ptr_, const std::function<void()> other_work_ = nullptr) {
			this->SetFrameRect(rect_).SetHasBackground(false).SetHasFrame(false).SetText(L"");
			this->SetAdditionalDrawFunction([&switch_ptr_](Sketch& sketch_) {
				int knobRadius = (sketch_.GetHeight()) / 2;
				if (switch_ptr_) {
					GetDrawingTool().SolidRoundRect(sketch_.GetFrameRect(), knobRadius * 2, knobRadius * 2, GREEN);
					GetDrawingTool().SolidCircle(Coordinate(sketch_.GetRight() - knobRadius, sketch_.GetCenterY()), knobRadius, WHITE);
				}
				else {
					GetDrawingTool().SolidRoundRect(sketch_.GetFrameRect(), knobRadius * 2, knobRadius * 2, RGB(0x1, 0x1, 0x1));
					GetDrawingTool().SolidCircle(Coordinate(sketch_.GetLeft() + knobRadius, sketch_.GetCenterY()), knobRadius, WHITE);
				}
				});
			this->SetHoverFunc(nullptr).SetPressFunc(nullptr).SetLeaveFunc(nullptr).SetDragFunc(nullptr);
			if (other_work_) {
				this->SetReleaseFunc([&switch_ptr_, other_work_](Button& button_, ExMessage) {
					switch_ptr_ = !(switch_ptr_);
					button_.DrawSketch();
					other_work_();
					});
			}
			else {
				this->SetReleaseFunc([&switch_ptr_](Button& button_, ExMessage) {
					switch_ptr_ = !(switch_ptr_);
					button_.DrawSketch();
					});
			}
		}

		void SetThumb(RECT rect_, Fraction default_value_, const std::function<std::wstring(Fraction)>& set_value_func_) {
			if (rect_.left > rect_.right) {
				std::swap(rect_.left, rect_.right);
			}
			auto messagePtr = std::make_shared<Sketch>(rect_, set_value_func_(default_value_));
			messagePtr->SetHasFrame(false);
			int textWidth = 0;
			GetDrawingTool().ExecuteWithLock([&messagePtr, &textWidth]() {
				::settextstyle(messagePtr->GetTextSize(), 0, messagePtr->GetTypeface().c_str());
				textWidth = ::textwidth(messagePtr->GetText().c_str());
				});
			textWidth = textWidth * 10 / 9 * 20 / 19;
			if (textWidth > (rect_.right - rect_.left) / 2) {
				textWidth = (rect_.right - rect_.left) / 2;
			}
			messagePtr->SetFrameRect(RECT(rect_.left, rect_.top, rect_.left + textWidth, rect_.bottom)).
				SetTextMode(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			this->SetHasBackground(false).SetHasFrame(false).SetText(L"").
				SetFrameRect(RECT(messagePtr->GetRight(), rect_.top, rect_.right, rect_.bottom));
			using F = Fraction;
			std::shared_ptr<F> currValuePtr = std::make_shared<F>(default_value_);
			std::shared_ptr<F> lastValuePtr = std::make_shared<F>(default_value_);
			F leftBound = F(1, 50) * this->GetWidth() + this->GetLeft();
			F rightBound = F(49, 50) * this->GetWidth() + this->GetLeft();
			F topBound = F(1, 8) * this->GetHeight() + this->GetTop();
			F bottomBound = F(7, 8) * this->GetHeight() + this->GetTop();
			this->SetAdditionalDrawFunction([messagePtr, currValuePtr, set_value_func_, leftBound, rightBound, topBound, bottomBound](Sketch& sketch_) {
				GetDrawingTool().ClearRectangle(sketch_.GetFrameRect());
				GetDrawingTool().Line({ Coordinate(leftBound,topBound),Coordinate(leftBound,bottomBound) }, 2, PS_SOLID, WHITE);
				GetDrawingTool().Line({ Coordinate(rightBound,topBound),Coordinate(rightBound,bottomBound) }, 2, PS_SOLID, WHITE);
				GetDrawingTool().Line({ Coordinate(leftBound,sketch_.GetCenterY()),Coordinate(rightBound,sketch_.GetCenterY()) }, 2, PS_SOLID, WHITE);
				RECT thumbRect = {
					((*currValuePtr) * (rightBound - leftBound) + leftBound) - (F(1, 100) * sketch_.GetWidth() / 2),
					topBound,
					((*currValuePtr) * (rightBound - leftBound) + leftBound) + (F(1, 100) * sketch_.GetWidth() / 2),
					bottomBound
				};
				GetDrawingTool().FillRoundRect(thumbRect, 5, 5, 2, PS_SOLID, BLACK, WHITE);
				messagePtr->SetText(set_value_func_(*currValuePtr)).DrawSketch(false);
				GetDrawingTool().FlushBatchDraw(messagePtr->GetLeft(), messagePtr->GetTop(), sketch_.GetRight(), messagePtr->GetBottom());
				});
			this->SetHoverFunc(nullptr).SetLeaveFunc(nullptr).SetPressFunc(nullptr).SetReleaseFunc(nullptr).
				SetDragFunc([currValuePtr, lastValuePtr, leftBound, rightBound, topBound, bottomBound, messagePtr, set_value_func_](Button& button_, ExMessage msg) {
				RECT lastThumbRect = {
					((*lastValuePtr) * (rightBound - leftBound) + leftBound) - (F(1, 100) * button_.GetWidth() / 2) - 1,
					topBound - 1,
					((*lastValuePtr) * (rightBound - leftBound) + leftBound) + (F(1, 100) * button_.GetWidth() / 2) + 1,
					bottomBound + 1
				};
				GetDrawingTool().ClearRectangle(lastThumbRect);
				GetDrawingTool().Line({ Coordinate((std::max)(F(lastThumbRect.left - 2),leftBound),button_.GetCenterY()),
					Coordinate((std::min)(F(lastThumbRect.right + 2),rightBound),button_.GetCenterY()) }, 2, PS_SOLID, WHITE);
				if (*lastValuePtr < F(2, 100)) {
					GetDrawingTool().Line({ Coordinate(leftBound,topBound),Coordinate(leftBound,bottomBound) }, 2, PS_SOLID, WHITE);
				}
				else if (*lastValuePtr > F(98, 100)) {
					GetDrawingTool().Line({ Coordinate(rightBound,topBound),Coordinate(rightBound,bottomBound) }, 2, PS_SOLID, WHITE);
				}
				GetDrawingTool().FlushBatchDraw(lastThumbRect);
				F temp = (msg.x - leftBound) / (rightBound - leftBound);
				temp = std::clamp(temp, F(0), F(1));
				*currValuePtr = temp;
				GetDrawingTool().Line({ Coordinate(leftBound,button_.GetCenterY()),Coordinate(rightBound,button_.GetCenterY()) }, 2, PS_SOLID, WHITE);
				RECT thumbRect = {
					((*currValuePtr) * (rightBound - leftBound) + leftBound) - (F(1, 100) * button_.GetWidth() / 2),
					topBound,
					((*currValuePtr) * (rightBound - leftBound) + leftBound) + (F(1, 100) * button_.GetWidth() / 2),
					bottomBound
				};
				GetDrawingTool().FillRoundRect(thumbRect, 5, 5, 2, PS_SOLID, BLACK, WHITE);
				GetDrawingTool().FlushBatchDraw(thumbRect);
				messagePtr->SetText(set_value_func_(*currValuePtr)).DrawSketch();
				*lastValuePtr = *currValuePtr;
					});
		}

	};

	class ButtonSequence {

		friend class MainMenu;

	private:

		std::vector<Button> m_buttons;
		std::shared_ptr<std::atomic<bool>> m_exitFlag;
		Receiver<ExMessage>* m_receiver = nullptr;
		std::thread m_pumpThread;

		std::optional<size_t> LinearFindButtonIndex(const ExMessage& mouse_message_) const noexcept {
			for (size_t index = 0; index < this->m_buttons.size(); ++index) {
				if (this->m_buttons[index].IsMouseInButton(mouse_message_)) {
					return index;
				}
			}
			return std::nullopt;
		}

		// 处理一条鼠标消息，同时更新 last 状态
		void ProcessMouseMessage(std::optional<size_t>& last_button_index_,
			ExMessage& last_mouse_message_,
			const ExMessage& current_message_) {

			std::optional<size_t> currentButtonIndex = this->LinearFindButtonIndex(current_message_);

			if (currentButtonIndex.has_value()) {
				std::optional<size_t> eventType = this->m_buttons[currentButtonIndex.value()].GetMouseEventType(
					last_mouse_message_, current_message_);
				if (eventType.has_value()) {
					std::function<void(Button&, ExMessage)>& callbackFunc =
						this->m_buttons[currentButtonIndex.value()].m_callbackFuncs[eventType.value()];
					if (callbackFunc) {
						callbackFunc(this->m_buttons[currentButtonIndex.value()], current_message_);
					}
				}
			}

			if (last_button_index_.has_value() && last_button_index_ != currentButtonIndex) {
				const std::function<void(Button&, ExMessage)>& callbackFunc =
					this->m_buttons[last_button_index_.value()].m_callbackFuncs[Button::Leave];
				if (callbackFunc) {
					callbackFunc(this->m_buttons[last_button_index_.value()], current_message_);
				}
			}

			last_button_index_ = currentButtonIndex;
			last_mouse_message_ = current_message_;
		}

		// 退出非阻塞模式：先解绑（MarkClosed 唤醒 pump 线程），再 join，最后销毁 Receiver
		void UnsubscribeReceiver() noexcept {
			if (this->m_receiver != nullptr) {
				Receiver<ExMessage>* receiver = this->m_receiver;
				this->m_receiver = nullptr;
				// ReleaseReceiver 内部会 MarkClosed → pump 线程的 Receive 返回 false
				auto receiverOwner = MouseMessageSource::GetInstance().GetChannel().ReleaseReceiver(receiver);
				if (this->m_pumpThread.joinable()) {
					this->m_pumpThread.join();
				}
			}
			else if (this->m_pumpThread.joinable()) {
				this->m_pumpThread.join();
			}
		}

		// ---------- 阻塞事件循环 ----------
		/**
		* @brief 按钮事件循环的公共主体：阻塞等消息、取最新一条、派发回调。
		*        直到 m_exitFlag 为 true 或 Channel 被关闭才返回。
		*        这个函数会阻塞调用它的线程，因此阻塞模式直接调，非阻塞模式放到 pump 线程里调。
		*/
		void RunButtonEventLoop(Receiver<ExMessage>& receiver_) {
			std::optional<size_t> lastButtonIndex;
			ExMessage lastMouseMessage = {};
			ExMessage currentMouseMessage;

			while (!this->m_exitFlag->load(std::memory_order_acquire)) {
				if (!receiver_.WaitTakeLatest(currentMouseMessage)) {
					break;   // Channel 已关闭，或 WaitTakeLatest 失败
				}
				this->ProcessMouseMessage(lastButtonIndex, lastMouseMessage, currentMouseMessage);
			}
		}

		/**
		 * @brief 订阅鼠标消息，返回新建 Receiver 的裸指针。
		 *        所有权已交给 Channel（以 unique_ptr 形式），调用方只借指针。
		 *        用指针而不是 shared_ptr，因为 Channel 用的是裸指针索引 + unique_ptr 持有的模式。
		 */
		Receiver<ExMessage>* SubscribeReceiver() {
			Channel<ExMessage>& channel = MouseMessageSource::GetInstance().GetChannel();
			auto receiverOwner = std::make_unique<Receiver<ExMessage>>();
			Receiver<ExMessage>* receiver = receiverOwner.get();
			channel.AddReceiver(std::move(receiverOwner));
			return receiver;
		}

	public:

		ButtonSequence(size_t button_num_ = 0) :
			m_buttons(button_num_), m_exitFlag(std::make_shared<std::atomic<bool>>(false)) {
		}

		ButtonSequence(const ButtonSequence&) = delete;
		ButtonSequence& operator=(const ButtonSequence&) = delete;

		~ButtonSequence() {
			if (this->m_exitFlag) this->m_exitFlag->store(true, std::memory_order_release);
			this->UnsubscribeReceiver();
		}

		std::vector<Button>& GetButtons() noexcept {
			return this->m_buttons;
		}
		const std::vector<Button>& GetButtons() const noexcept {
			return this->m_buttons;
		}
		ButtonSequence& SetButtons(std::vector<Button>&& buttons_) {
			this->m_buttons = std::move(buttons_);
			return *this;
		}

		constexpr size_t GetButtonNum() const noexcept {
			return this->m_buttons.size();
		}

		template<typename T>
		ButtonSequence& AddButton(T&& button_) {
			this->m_buttons.emplace_back(std::forward<T>(button_));
			return *this;
		}
		ButtonSequence& AddButton(int left_, int top_, int right_, int bottom_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) {
			this->m_buttons.emplace_back(left_, top_, right_, bottom_, text_, release_func_);
			return *this;
		}
		ButtonSequence& AddButton(RECT rect_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) {
			return this->AddButton(rect_.left, rect_.top, rect_.right, rect_.bottom, text_, release_func_);
		}
		ButtonSequence& AddButtonAsCross(Coordinate center_, Fraction size_, const std::function<void(Button&)>& last_work_ = nullptr) {
			this->m_buttons.emplace_back();
			this->m_buttons.rbegin()->SetCross(center_, size_, this->m_exitFlag, last_work_);
			return *this;
		}
		ButtonSequence& AddButtonAsSwitch(RECT rect_, std::shared_ptr<bool> switch_ptr_, const std::function<void()> other_work_ = nullptr) {
			this->m_buttons.emplace_back();
			this->m_buttons.rbegin()->SetSwitch(rect_, switch_ptr_, other_work_);
			return *this;
		}
		ButtonSequence& AddButtonAsThumb(RECT rect_, Fraction default_value_, const std::function<std::wstring(Fraction)>& set_value_func_) {
			this->m_buttons.emplace_back();
			this->m_buttons.rbegin()->SetThumb(rect_, default_value_, set_value_func_);
			return *this;
		}

		ButtonSequence& SetButton(size_t index_, int left_, int top_, int right_, int bottom_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) {
			if (index_ >= this->m_buttons.size()) {
				throw WideError(L"Button 下标越界！");
			}
			this->m_buttons[index_].SetButton(left_, top_, right_, bottom_, text_, release_func_);
			return *this;
		}
		ButtonSequence& SetButton(size_t index_, RECT rect_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) {
			return this->SetButton(index_, rect_.left, rect_.top, rect_.right, rect_.bottom, text_, release_func_);
		}
		ButtonSequence& SetButtonAsCross(size_t index_, Coordinate center_, Fraction size_, const std::function<void(Button&)>& last_work_ = nullptr) {
			if (index_ >= this->m_buttons.size()) {
				throw WideError(L"Button 下标越界！");
			}
			this->m_buttons[index_].SetCross(center_, size_, this->m_exitFlag, last_work_);
			return *this;
		}
		ButtonSequence& SetButtonAsSwitch(size_t index_, RECT rect_, std::shared_ptr<bool> switch_ptr_, const std::function<void()> other_work_ = nullptr) {
			if (index_ >= this->m_buttons.size()) {
				throw WideError(L"Button 下标越界！");
			}
			this->m_buttons[index_].SetSwitch(rect_, switch_ptr_, other_work_);
			return *this;
		}
		ButtonSequence& SetButtonAsSwitch(size_t index_, RECT rect_, bool& switch_ptr_, std::function<void()> other_work_ = nullptr) {
			if (index_ >= this->m_buttons.size()) {
				throw WideError(L"Button 下标越界！");
			}
			this->m_buttons[index_].SetSwitch(rect_, switch_ptr_, other_work_);
			return *this;
		}
		ButtonSequence& SetButtonAsThumb(size_t index_, RECT rect_, Fraction default_value_, const std::function<std::wstring(Fraction)>& set_value_func_) {
			if (index_ >= this->m_buttons.size()) {
				throw WideError(L"Button 下标越界！");
			}
			this->m_buttons[index_].SetThumb(rect_, default_value_, set_value_func_);
			return *this;
		}

		ButtonSequence& Clear() {
			this->m_exitFlag->store(false, std::memory_order_release);
			this->m_buttons.clear();
			return *this;
		}

		ButtonSequence& Resize(size_t size_) {
			this->m_exitFlag->store(false, std::memory_order_release);
			this->m_buttons.resize(size_);
			return *this;
		}

		bool GetExitFlag() const noexcept {
			return this->m_exitFlag->load(std::memory_order_acquire);
		}
		ButtonSequence& SetExitFlag(bool exit_flag_ = true) noexcept {
			this->m_exitFlag->store(exit_flag_, std::memory_order_release);
			return *this;
		}

		void DrawButtons(bool is_flush_ = true) {
			for (auto it = this->m_buttons.begin(); it != this->m_buttons.end(); ++it) {
				it->DrawSketch(false);
			}
			if (is_flush_) {
				GetDrawingTool().FlushBatchDraw();
			}
		}

		void RunBlockButtonLoop() {

			this->m_exitFlag->store(false, std::memory_order_release);
			this->DrawButtons();

			Channel<ExMessage>& channel = MouseMessageSource::GetInstance().GetChannel();
			Receiver<ExMessage>* receiver = this->SubscribeReceiver();

			ScopeGuard unsubscribeGuard([&channel, receiver]() {
				channel.DeleteReceiver(receiver);
				});

			this->RunButtonEventLoop(*receiver);
		}

		void RunNonBlockButtonLoop() {
			this->UnsubscribeReceiver();
			this->m_exitFlag->store(false, std::memory_order_release);
			this->DrawButtons();

			Receiver<ExMessage>* receiver = this->SubscribeReceiver();
			this->m_receiver = receiver;

			this->m_pumpThread = std::thread([this, receiver]() {
				this->RunButtonEventLoop(*receiver);
				});
		}

	};

}