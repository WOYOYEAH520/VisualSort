#pragma once
#include "Sketch.h"
#include <Windows.h>
#include <functional>
#include <easyx.h>
#include "ScopeGuard.h"
#include <vector>
#include <atomic>
#include <memory>
#include <string>
#include <optional>
#include "WideError.h"
#include "DrawingTool.h"
#include <list>
#include <mutex>
#include <utility>
#include "Coordinate.h"
#include "Fraction.h"
#include <condition_variable>
#include <algorithm>

namespace NVisualSort {

	constexpr COLORREF HoverOffset = RGB(0x66, 0x66, 0x66); // 鼠标悬停时的背景颜色偏移
	constexpr COLORREF PressOffset = RGB(0x33, 0x33, 0x33); // 鼠标按下时的背景颜色偏移

	class Button {

		friend class ButtonSequence; // 允许 ButtonSequence 访问 Button 的私有成员

	private:

		Sketch m_sketch; // 按钮的视觉表现由 Sketch 负责

		static void DefaultHoverDrawFunction(Button& button_, ExMessage) {
			if (!button_.m_sketch.GetHasBackground()) {
				return; // 如果没有背景，就不需要改变颜色
			}
			COLORREF originalColor = button_.m_sketch.GetBackgroundColor();
			ScopeGuard restoreColorGuard([&button_, originalColor]() {
				button_.m_sketch.SetBackgroundColor(originalColor);
			});
			button_.m_sketch.SetBackgroundColor(HoverOffset + originalColor);
			button_.m_sketch.DrawSketch();
		}

		static void DefaultPressDrawFunction(Button& button_, ExMessage) {
			if (!button_.m_sketch.GetHasBackground()) {
				return; // 如果没有背景，就不需要改变颜色
			}
			COLORREF originalColor = button_.m_sketch.GetBackgroundColor();
			ScopeGuard restoreColorGuard([&button_, originalColor]() {
				button_.m_sketch.SetBackgroundColor(originalColor);
			});
			button_.m_sketch.SetBackgroundColor(PressOffset + originalColor);
			button_.m_sketch.DrawSketch();
		}

		static void DefaultLeaveDrawFunction(Button& button_, ExMessage) {
			button_.m_sketch.DrawSketch();
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
				if ((!wasIn) || (lastDown &&!currDown)) {
					return Button::Hover;
				}
			}
			else if (wasIn) {
				return Button::Leave;
			}

			return std::nullopt;
		}

	public:

		constexpr inline static size_t Hover = 0; // 鼠标悬停事件索引
		constexpr inline static size_t Press = 1; // 鼠标按下事件索引
		constexpr inline static size_t Release = 2; // 鼠标释放事件索引
		constexpr inline static size_t Leave = 3; // 鼠标离开事件索引
		constexpr inline static size_t Drag = 4; // 鼠标拖拽事件索引

		Button() = default;
		Button(const Button&) = default;
		Button& operator=(const Button&) = default;
		Button(Button&&) = default;
		Button& operator=(Button&&) = default;
		
		Button(int left_, int top_, int right_, int bottom_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) :
			m_sketch(left_, top_, right_, bottom_, text_) {
			this->m_callbackFuncs[Button::Release] = release_func_;
		}
		
		Button(RECT rect_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) :
			m_sketch(rect_, text_) {
			this->m_callbackFuncs[Button::Release] = release_func_;
		}

		Button& SetButton(int left_, int top_, int right_, int bottom_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) {
			this->m_sketch.SetFrameRect({ left_, top_, right_, bottom_ }).SetText(text_);
			this->m_callbackFuncs[Button::Release] = release_func_;
			return *this;
		}

		Button& SetButton(RECT rect_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) {
			this->m_sketch.SetFrameRect(rect_).SetText(text_);
			this->m_callbackFuncs[Button::Release] = release_func_;
			return *this;
		}

		Button& SetSketch(Sketch&& sketch_) {
			this->m_sketch = std::move(sketch_);
			return *this;
		}
		Sketch& GetSketch() noexcept {
			return this->m_sketch;
		}
		const Sketch& GetSketch() const noexcept {
			return this->m_sketch;
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
			return mouse_message_.x >= this->m_sketch.GetLeft() &&
				mouse_message_.x <= this->m_sketch.GetRight() &&
				mouse_message_.y >= this->m_sketch.GetTop() &&
				mouse_message_.y <= this->m_sketch.GetBottom();
		}

		void SetCross(Coordinate center_, Fraction size_, std::shared_ptr<std::atomic<bool>> exit_flag_, const std::function<void(Button&)>& last_work_ = nullptr) {
			this->m_sketch.SetHasBackground(false).SetHasFrame(false).SetText(L"");
			this->m_sketch.SetFrameRect({
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
			this->m_sketch.SetAdditionalDrawFunction([crossPoints](Sketch& sketch_) {
				GetDrawingTool().SolidPolygon(crossPoints, RED);
				sketch_.Flush();
			});
			this->SetHoverFunc([crossPoints](Button& button_, ExMessage) {
				GetDrawingTool().SolidPolygon(crossPoints, HSVtoRGB(0, 1, 1));
				button_.m_sketch.Flush();
			}).SetPressFunc([crossPoints](Button& button_, ExMessage) {
				GetDrawingTool().SolidPolygon(crossPoints, HSVtoRGB(0, 1, static_cast<float>(0.8)));
				button_.m_sketch.Flush();
			}).SetLeaveFunc([](Button& button_, ExMessage) {
				button_.m_sketch.DrawSketch();
			}).SetDragFunc(nullptr);
			if (last_work_) {
				this->SetReleaseFunc([exit_flag_, last_work_](Button& button_, ExMessage) {
					exit_flag_->store(true, std::memory_order_release);
					button_.m_sketch.Flush();
					last_work_(button_);
				});
			} else {
				this->SetReleaseFunc([exit_flag_](Button& button_, ExMessage) {
					exit_flag_->store(true, std::memory_order_release);
					button_.m_sketch.Flush();
				});
			}
		}

		void SetSwitch(RECT rect_, std::shared_ptr<bool> switch_ptr_, const std::function<void()> other_work_ = nullptr) {
			this->m_sketch.SetFrameRect(rect_).SetHasBackground(false).SetHasFrame(false).SetText(L"");
			this->m_sketch.SetAdditionalDrawFunction([switch_ptr_](Sketch& sketch_) {
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
					button_.m_sketch.DrawSketch();
					other_work_();
				});
			}
			else {
				this->SetReleaseFunc([switch_ptr_, other_work_](Button& button_, ExMessage) {
					*switch_ptr_ = !(*switch_ptr_);
					button_.m_sketch.DrawSketch();
				});
			}
		}

		void SetSwitch(RECT rect_, bool& switch_ptr_, const std::function<void()> other_work_ = nullptr) {
			this->m_sketch.SetFrameRect(rect_).SetHasBackground(false).SetHasFrame(false).SetText(L"");
			this->m_sketch.SetAdditionalDrawFunction([&switch_ptr_](Sketch& sketch_) {
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
					button_.m_sketch.DrawSketch();
					other_work_();
				});
			}
			else {
				this->SetReleaseFunc([&switch_ptr_](Button& button_, ExMessage) {
					switch_ptr_ = !(switch_ptr_);
					button_.m_sketch.DrawSketch();
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
			this->m_sketch.SetHasBackground(false).SetHasFrame(false).SetText(L"").
				SetFrameRect(RECT(messagePtr->GetRight(), rect_.top, rect_.right, rect_.bottom));
			using F = Fraction;
			std::shared_ptr<F> currValuePtr = std::make_shared<F>(default_value_);
			std::shared_ptr<F> lastValuePtr = std::make_shared<F>(default_value_);
			F leftBound = F(1, 50) * this->m_sketch.GetWidth() + this->m_sketch.GetLeft();
			F rightBound = F(49, 50) * this->m_sketch.GetWidth() + this->m_sketch.GetLeft();
			F topBound = F(1, 8) * this->m_sketch.GetHeight() + this->m_sketch.GetTop();
			F bottomBound = F(7, 8) * this->m_sketch.GetHeight() + this->m_sketch.GetTop();
			this->m_sketch.SetAdditionalDrawFunction([messagePtr, currValuePtr, set_value_func_, leftBound, rightBound, topBound, bottomBound](Sketch& sketch_) {
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
					((*lastValuePtr) * (rightBound - leftBound) + leftBound) - (F(1, 100) * button_.GetSketch().GetWidth() / 2) - 1,
					topBound - 1,
					((*lastValuePtr) * (rightBound - leftBound) + leftBound) + (F(1, 100) * button_.GetSketch().GetWidth() / 2) + 1,
					bottomBound + 1
				};
				GetDrawingTool().ClearRectangle(lastThumbRect);
				GetDrawingTool().Line({ Coordinate((std::max)(F(lastThumbRect.left - 2),leftBound),button_.GetSketch().GetCenterY()),
					Coordinate((std::min)(F(lastThumbRect.right + 2),rightBound),button_.GetSketch().GetCenterY()) }, 2, PS_SOLID, WHITE);
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
				GetDrawingTool().Line({ Coordinate(leftBound,button_.GetSketch().GetCenterY()),Coordinate(rightBound,button_.GetSketch().GetCenterY()) }, 2, PS_SOLID, WHITE);
				RECT thumbRect = {
					((*currValuePtr) * (rightBound - leftBound) + leftBound) - (F(1, 100) * button_.GetSketch().GetWidth() / 2),
					topBound,
					((*currValuePtr) * (rightBound - leftBound) + leftBound) + (F(1, 100) * button_.GetSketch().GetWidth() / 2),
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

		std::vector<Button> m_buttons; // 按钮列表
		std::shared_ptr<std::atomic<bool>> m_exitFlag; // 退出标志，供外部控制事件循环

		std::optional<size_t> LinearFindButtonIndex(const ExMessage& mouse_message_) const noexcept {
			for (size_t index = 0; index < this->m_buttons.size(); ++index) {
				if (this->m_buttons[index].IsMouseInButton(mouse_message_)) {
					return index;
				}
			}
			return std::nullopt; // 没有找到按钮
		}

		struct ButtonBlockItem {

			std::shared_ptr<std::atomic<bool>> m_exitLoopPtr;
			std::shared_ptr<std::atomic<bool>> m_notFalseWakeupPtr;
			std::shared_ptr<std::condition_variable> m_getMessageCVPtr;
			std::shared_ptr<std::atomic<bool>> m_isWaitingPtr;
			std::shared_ptr<ExMessage> m_currentMessagePtr;

			ButtonBlockItem(std::shared_ptr<std::atomic<bool>> exit_loop_ptr_, std::shared_ptr<std::atomic<bool>> not_false_wakeup_ptr_,
				std::shared_ptr<std::condition_variable> get_message_CV_ptr_, std::shared_ptr<std::atomic<bool>> is_waiting_ptr_,
				std::shared_ptr<ExMessage> current_message_ptr_) :
				m_exitLoopPtr(std::move(exit_loop_ptr_)), m_notFalseWakeupPtr(std::move(not_false_wakeup_ptr_)),
				m_getMessageCVPtr(std::move(get_message_CV_ptr_)), m_isWaitingPtr(std::move(is_waiting_ptr_)),
				m_currentMessagePtr(current_message_ptr_) {
			}

		};

		struct ButtonNonBlockItem {

			std::shared_ptr<std::function<void()>> m_nonBlockFunc;
			std::shared_ptr<std::atomic<bool>> m_exitLoopPtr;

			ButtonNonBlockItem(std::shared_ptr<std::atomic<bool>> exit_loop_ptr_, std::shared_ptr<std::function<void()>> func_ptr) noexcept :
				m_exitLoopPtr(std::move(exit_loop_ptr_)), m_nonBlockFunc(std::move(func_ptr)) {
			}

		};

		inline static std::list<ButtonBlockItem> s_blockTaskBuffer;
		inline static std::mutex s_blockTaskMutex;
		inline static std::list<ButtonNonBlockItem> s_nonBlockTaskBuffer;
		inline static std::mutex s_nonBlockTaskMutex;

		static void AddBlockTask(std::shared_ptr<std::atomic<bool>> exit_loop_ptr_, std::shared_ptr<std::atomic<bool>> not_false_wakeup_ptr_,
			std::shared_ptr<std::condition_variable> get_message_CV_ptr_, std::shared_ptr<std::atomic<bool>> is_waiting_ptr_,
			std::shared_ptr<ExMessage> current_message_ptr_) {
			std::lock_guard lock(ButtonSequence::s_blockTaskMutex);
			ButtonSequence::s_blockTaskBuffer.emplace_back(exit_loop_ptr_, not_false_wakeup_ptr_, get_message_CV_ptr_, is_waiting_ptr_, current_message_ptr_);
		}

		static void AddNonBlockTask(std::shared_ptr<std::atomic<bool>> exit_loop_ptr_, std::shared_ptr<std::function<void()>> func_ptr_) {
			std::lock_guard lock(ButtonSequence::s_nonBlockTaskMutex);
			ButtonSequence::s_nonBlockTaskBuffer.emplace_back(ButtonNonBlockItem{ exit_loop_ptr_,func_ptr_ });
		}

		inline static ExMessage s_mouseMessage = {};
		inline static std::atomic<bool> s_exitGetMessage = false;
		inline static std::mutex s_getMessageMutex;
		inline static std::atomic<bool> s_isGettingMessage = false;

		static void GetMessageLoop() {
			std::lock_guard getMessageLoopLock(ButtonSequence::s_getMessageMutex);
			ButtonSequence::s_exitGetMessage.store(false, std::memory_order_release); // 重置退出标志，准备进入事件循环
			ButtonSequence::s_isGettingMessage.store(true, std::memory_order_release);
			std::list<ButtonBlockItem> localBlockTasks;
			std::list<ButtonNonBlockItem> localNonBlockTasks; // 本地任务列表，避免在执行任务时持有全局锁
			ScopeGuard clearLocalTasksGuard([&localBlockTasks, &localNonBlockTasks]() {
				ButtonSequence::s_exitGetMessage.store(true, std::memory_order_release); // 在事件循环退出时设置退出标志
				ButtonSequence::s_isGettingMessage.store(false, std::memory_order_release);
				for (auto& item : localBlockTasks) {
					item.m_exitLoopPtr->store(true, std::memory_order_release);
					if (item.m_isWaitingPtr->load(std::memory_order_acquire)) {
						item.m_notFalseWakeupPtr->store(true, std::memory_order_release);
						item.m_getMessageCVPtr->notify_all();
					}
				}
				for (auto& item : localNonBlockTasks) {
					item.m_exitLoopPtr->store(true, std::memory_order_release); // 在事件循环退出时设置退出标志，通知任务退出
				}
			});

			while (!ButtonSequence::s_exitGetMessage.load(std::memory_order_acquire)) {

				::getmessage(&ButtonSequence::s_mouseMessage, EX_MOUSE); // 获取鼠标消息
				if (ButtonSequence::s_exitGetMessage.load(std::memory_order_acquire)) {
					return;
				}

				auto blockTaskIter = localBlockTasks.begin();
				while (blockTaskIter != localBlockTasks.end()) {
					if (blockTaskIter->m_exitLoopPtr->load(std::memory_order_acquire)) {
						if (blockTaskIter->m_isWaitingPtr->load(std::memory_order_acquire)) {
							blockTaskIter->m_notFalseWakeupPtr->store(true, std::memory_order_release);
							blockTaskIter->m_getMessageCVPtr->notify_all();
						}
						blockTaskIter = localBlockTasks.erase(blockTaskIter);
					}
					else {
						*blockTaskIter->m_currentMessagePtr = ButtonSequence::s_mouseMessage;
						blockTaskIter->m_notFalseWakeupPtr->store(true, std::memory_order_release);
						blockTaskIter->m_getMessageCVPtr->notify_all();
						++blockTaskIter;
					}
				}

				auto nonBlockTaskIter = localNonBlockTasks.begin();
				while (nonBlockTaskIter != localNonBlockTasks.end()) {
					if(nonBlockTaskIter->m_exitLoopPtr->load(std::memory_order_acquire)) {
						nonBlockTaskIter = localNonBlockTasks.erase(nonBlockTaskIter); // 任务退出，移除任务
					}
					else {
						if (*nonBlockTaskIter->m_nonBlockFunc) {
							nonBlockTaskIter->m_nonBlockFunc->operator()(); // 执行任务
						}
						++nonBlockTaskIter;
					}
				}

				if (!s_blockTaskBuffer.empty()) {
					std::unique_lock lock(ButtonSequence::s_blockTaskMutex);
					localBlockTasks.splice(localBlockTasks.end(), ButtonSequence::s_blockTaskBuffer);
				}

				if (!ButtonSequence::s_nonBlockTaskBuffer.empty()) {
					std::lock_guard lock(ButtonSequence::s_nonBlockTaskMutex);
					localNonBlockTasks.splice(localNonBlockTasks.end(), ButtonSequence::s_nonBlockTaskBuffer);
				}

			}
		}

	public:

		ButtonSequence(size_t button_num_ = 0) :m_buttons(button_num_), m_exitFlag(std::make_shared<std::atomic<bool>>(false)) {}
		ButtonSequence(const ButtonSequence&) = delete;
		ButtonSequence& operator=(const ButtonSequence&) = delete;
		ButtonSequence(ButtonSequence&& other_) noexcept {
			other_.m_exitFlag->store(true, std::memory_order_release);
			this->m_buttons = std::move(other_.m_buttons);
			this->m_exitFlag = std::move(other_.m_exitFlag);
		}
		ButtonSequence& operator=(ButtonSequence&& other_) noexcept {
			if (this != &other_) {
				other_.m_exitFlag->store(true, std::memory_order_release);
				this->m_buttons = std::move(other_.m_buttons);
				this->m_exitFlag = std::move(other_.m_exitFlag);
			}
		}
		~ButtonSequence() {
			this->m_exitFlag->store(true, std::memory_order_release); // 在销毁时设置退出标志，通知事件循环退出
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
			this->m_buttons.emplace_back(button_);
			return *this;
		}
		ButtonSequence& AddButton(int left_, int top_, int right_, int bottom_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) {
			this->m_buttons.emplace_back(left_, top_, right_, bottom_, text_, release_func_);
			return *this;
		}
		ButtonSequence& AddButton(RECT rect_, const std::wstring& text_ = L"",
			const std::function<void(Button&, ExMessage)>& release_func_ = nullptr) {
			this->AddButton(rect_.left, rect_.top, rect_.right, rect_.bottom, text_, release_func_);
			return *this;
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
				it->m_sketch.DrawSketch(false);
			}
			if (is_flush_) {
				GetDrawingTool().FlushBatchDraw();
			}
		}

		// 阻塞事件循环
		void RunBlockButtonLoop() {

			this->m_exitFlag->store(false, std::memory_order_release); // 重置退出标志，准备进入事件循环

			this->DrawButtons(); // 绘制按钮

			std::optional<size_t> lastButtonIndex; // 上一次鼠标所在的按钮索引
			ExMessage lastMouseMessage = {}; // 上一次的鼠标消息
			std::optional<size_t> currentButtonIndex; // 当前鼠标所在的按钮索引
			auto currentMouseMessagePtr = std::make_shared<ExMessage>(); // 当前的鼠标消息
			std::mutex waitMessageMutex;
			auto notFalseWakeupPtr = std::make_shared<std::atomic<bool>>(false);
			auto waitMessageCVPtr = std::make_shared<std::condition_variable>();
			auto isWaitingPtr = std::make_shared<std::atomic<bool>>(false);

			ButtonSequence::AddBlockTask(this->m_exitFlag, notFalseWakeupPtr, waitMessageCVPtr, isWaitingPtr, currentMouseMessagePtr);
			ScopeGuard guard([this, isWaitingPtr]() {
				this->m_exitFlag->store(true, std::memory_order_release);
				isWaitingPtr->store(false, std::memory_order_release);
			});

			while (!this->m_exitFlag->load(std::memory_order_acquire) &&
				!ButtonSequence::s_exitGetMessage.load(std::memory_order_acquire)) {

				std::unique_lock lock(waitMessageMutex);
				while (!notFalseWakeupPtr->load(std::memory_order_acquire)) {
					isWaitingPtr->store(true, std::memory_order_release);
					waitMessageCVPtr->wait(lock);
				}
				isWaitingPtr->store(false, std::memory_order_release);
				notFalseWakeupPtr->store(false, std::memory_order_release);

				if (ButtonSequence::s_exitGetMessage.load(std::memory_order_acquire)) {
					return;
				}
				*currentMouseMessagePtr = ButtonSequence::s_mouseMessage; // 获取当前鼠标消息
				currentButtonIndex = this->LinearFindButtonIndex(*currentMouseMessagePtr); // 查找当前鼠标所在的按钮索引

				if(currentButtonIndex.has_value()) {
					std::optional<size_t> eventType = this->m_buttons[currentButtonIndex.value()].GetMouseEventType(lastMouseMessage, *currentMouseMessagePtr); // 获取鼠标事件类型
					if (eventType.has_value()) {
						std::function<void(Button&, ExMessage)>& callbackFunc = this->m_buttons[currentButtonIndex.value()].m_callbackFuncs[eventType.value()]; // 获取回调函数
						if (callbackFunc) {
							callbackFunc(this->m_buttons[currentButtonIndex.value()], *currentMouseMessagePtr); // 执行回调函数
						}
					}
				}

				if(lastButtonIndex.has_value() && lastButtonIndex != currentButtonIndex) {
					const std::function<void(Button&, ExMessage)>& callbackFunc = this->m_buttons[lastButtonIndex.value()].m_callbackFuncs[Button::Leave]; // 获取回调函数
					if (callbackFunc) {
						callbackFunc(this->m_buttons[lastButtonIndex.value()], *currentMouseMessagePtr); // 执行回调函数
					}
				}

				lastButtonIndex = currentButtonIndex;
				lastMouseMessage = *currentMouseMessagePtr;
			}
		}

		// 非阻塞事件循环，禁止使用带有阻塞效果的回调函数，否则可能导致事件循环无法正常进行
		void RunNonBlockButtonLoop() {
			this->m_exitFlag->store(false, std::memory_order_release); // 重置退出标志，准备进入事件循环

			this->DrawButtons(); // 绘制按钮

			auto lastMouseMessagePtr = std::make_shared<ExMessage>(); // 上一次的鼠标消息
			auto lastButtonIndexPtr = std::make_shared<std::optional<size_t>>(); // 上一次鼠标所在的按钮索引

			ButtonSequence::AddNonBlockTask(this->m_exitFlag, std::make_shared<std::function<void()>>([this, lastMouseMessagePtr, lastButtonIndexPtr]() {
				ExMessage currentMouseMessage = ButtonSequence::s_mouseMessage; // 获取当前鼠标消息
				std::optional<size_t> currentButtonIndex = this->LinearFindButtonIndex(currentMouseMessage); // 当前鼠标所在的按钮索引

				if (currentButtonIndex.has_value()) {
					std::optional<size_t> eventType = this->m_buttons[currentButtonIndex.value()].GetMouseEventType(*lastMouseMessagePtr, currentMouseMessage); // 获取鼠标事件类型
					if (eventType.has_value()) {
						const std::function<void(Button&, ExMessage)>& callbackFunc = this->m_buttons[currentButtonIndex.value()].m_callbackFuncs[eventType.value()]; // 获取回调函数
						if (callbackFunc) {
							callbackFunc(this->m_buttons[currentButtonIndex.value()], currentMouseMessage); // 执行回调函数
						}
					}
				}

				if (lastButtonIndexPtr->has_value() && *lastButtonIndexPtr != currentButtonIndex) {
					const std::function<void(Button&, ExMessage)>& callbackFunc = this->m_buttons[lastButtonIndexPtr->value()].m_callbackFuncs[Button::Leave]; // 获取回调函数
					if (callbackFunc) {
						callbackFunc(this->m_buttons[lastButtonIndexPtr->value()], currentMouseMessage); // 执行回调函数
					}
				}

				*lastButtonIndexPtr = currentButtonIndex;
				*lastMouseMessagePtr = currentMouseMessage;
			}));
		}

	};

}