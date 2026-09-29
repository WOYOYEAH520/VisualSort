#pragma once
#include "VisualSort.h"
#include "InputBox.h"
#include "Button.h"
#include "ConfigManager.h"
#include "DrawingTool.h"
#include "Dialog.h"
#include "Sketch.h"
#include <Windows.h>
#include <easyx.h>
#include <algorithm>
#include <optional>
#include <string>
#include <thread>
#include <vector>
#include "Fraction.h"
#include "WideError.h"
#include "Coordinate.h"
#include <memory>
#pragma comment(lib, "winmm.lib")

namespace NVisualSort {

	class MainMenu {

	private:

		bool m_fullScreen = true; // 是否全屏显示
		std::shared_ptr<std::thread> m_getMessageThreadPtr; // (获取鼠标消息)的线程指针

		MainMenu() {
			DEVMODE devMode = {};
			EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
			GetConfigManager().SetWidth(devMode.dmPelsWidth); // 设置窗口宽度
			GetConfigManager().SetHeight(devMode.dmPelsHeight); // 设置窗口高度
			::initgraph(GetConfigManager().GetWidth(), GetConfigManager().GetHeight(), EX_NOCLOSE | EX_NOMINIMIZE); // 创建窗口
			HWND hwnd = GetHWnd();
			LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
			style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
			SetWindowLongPtr(hwnd, GWL_STYLE, style); // 应用窗口风格（无边框、无标题栏等等）
			this->SetStyleAuto(); // 自动设置一些风格（比如文字抗锯齿等等）
			::timeBeginPeriod(1); // 向系统请求提高定时器的分辨率
			this->m_getMessageThreadPtr = std::make_shared<std::thread>(ButtonSequence::GetMessageLoop); // 开始获取鼠标消息
			this->RunMainMenu(); // 进入主菜单
		}

		MainMenu(const MainMenu&) = delete;
		MainMenu(MainMenu&&) = delete;
		MainMenu& operator = (const MainMenu&) = delete;
		MainMenu& operator = (MainMenu&&) = delete;

		~MainMenu() {
			::timeEndPeriod(1);
			::EndBatchDraw();
			::closegraph();
		}

		void FullScreen() const {
			if (this->m_getMessageThreadPtr->joinable()) {
				throw WideError(L"获取鼠标消息的线程处于工作状态，直接改变窗口大小会导致死锁");
			}
			::EndBatchDraw();
			::closegraph();
			DEVMODE devMode = {};
			EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
			GetConfigManager().SetWidth(devMode.dmPelsWidth);
			GetConfigManager().SetHeight(devMode.dmPelsHeight);
			::initgraph(GetConfigManager().GetWidth(), GetConfigManager().GetHeight(), EX_NOCLOSE | EX_NOMINIMIZE);
			HWND hwnd = GetHWnd();
			LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
			style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
			SetWindowLongPtr(hwnd, GWL_STYLE, style);
			this->SetStyleAuto();
		}

		void SetStyleAuto() const {
			::setbkcolor(GetConfigManager().GetCanvasColor());
			::setbkmode(TRANSPARENT);
			LOGFONT logFont = {};
			::gettextstyle(&logFont);
			logFont.lfQuality = ANTIALIASED_QUALITY;
			::settextstyle(&logFont);
			HWND hwnd = GetHWnd();
			SetWindowText(hwnd, L"你看，它们像柱子一样");
			BeginBatchDraw();
			GetDrawingTool().ClearDevice();
			GetDrawingTool().FlushBatchDraw();
		}

		void MaxWindow() const {
			if (this->m_getMessageThreadPtr->joinable()) {
				throw WideError(L"获取鼠标消息的线程处于工作状态，直接改变窗口大小会导致死锁");
			}
			
			GetConfigManager().SetDimensions(GetConfigManager().GetMaxWidth(), GetConfigManager().GetMaxClientHeight());

			::closegraph();

			::initgraph(GetConfigManager().GetMaxWidth(), GetConfigManager().GetHeight(), EX_NOCLOSE | EX_NOMINIMIZE);

			this->SetStyleAuto();
		}

		void ResizeWindow() const {
			if (this->m_getMessageThreadPtr->joinable()) {
				throw WideError(L"获取鼠标消息的线程处于工作状态，直接改变窗口大小会导致死锁");
			}
			::closegraph();
			::initgraph(GetConfigManager().GetWidth(), GetConfigManager().GetHeight(), EX_NOCLOSE | EX_NOMINIMIZE);
			this->SetStyleAuto();
		}
		
		void RunMainMenu() {
			using F = Fraction; // 分数类，保证整数除法时的精度
			while (true) {
				Sketch titleSketch(ComputeRect(GetConfigManager().GetCanvasRect(),
					F(1, 8), F(1, 10), F(7, 8), F(3, 10)), L"排序可视化"); // 标题设置
				titleSketch.SetHasFrame(false).SetHasBackground(false);
				Sketch WuYou(ComputeRect(GetConfigManager().GetCanvasRect(),
					F(281, 336), F(20, 21), F(1), F(1)), L"by 无忧yeah");
				WuYou.SetHasFrame(false).SetHasBackground(false).SetTextMode(DT_SINGLELINE | DT_BOTTOM | DT_RIGHT);
				static ButtonSequence buttons(3); // 三个按钮：开始、设置、退出
				constexpr F bWidth(3, 16); // 按钮宽度占窗口宽度的比例
				constexpr F bHegiht(3, 20); // 按钮高度占窗口高度的比例
				constexpr F vertiGap(1, 20); // 按钮竖直间隔占窗口高度的比例
				constexpr F lMargin(13, 32); // 第一个按钮离窗口左侧的距离占窗口宽度的比例
				constexpr F tMargin(7, 20); // 第一个按钮离窗口上方的距离占窗口高度的比例
				constexpr const wchar_t* texts[] = {L"开始",L"设置",L"退出"};
				static std::optional<size_t> choice; // 选择了哪个按钮
				choice = std::nullopt;
				for (size_t i = 0; i < buttons.GetButtonNum(); ++i) {
					buttons.GetButtons()[i].SetButton(ComputeRect(GetConfigManager().GetCanvasRect(),
						lMargin, (i + 1) * vertiGap + i * bHegiht + tMargin, lMargin + bWidth, (i + 1) * (vertiGap + bHegiht) + tMargin),
						texts[i], [i](Button&, ExMessage) {
							choice = i;
							buttons.SetExitFlag(true);
						}
					);
				} // 设置三个按钮的属性
				GetDrawingTool().ClearDevice();
				titleSketch.DrawSketch(false);
				WuYou.DrawSketch(false);
				buttons.RunBlockButtonLoop(); // 开始按钮检测（阻塞，直到选择了一个按钮）

				if (!choice.has_value()) {
					throw WideError(L"未选择任何选项");
				}
				else if (choice.value() == 0) { // 选择了下标为0的按钮（开始按钮）
					this->RunVisualSortMenu();
				}
				else if (choice.value() == 1) { // 选择了下标为1的按钮（设置按钮）
					this->RunSetMenu();
				}
				else if (choice.value() == 2) { // 选择了下标为2的按钮（退出按钮）
					ButtonSequence::s_exitGetMessage.store(true, std::memory_order_release); // 设置退出标志为真
					PostMessage(GetHWnd(), WM_MOUSEMOVE, 0, MAKELPARAM(100, 100)); // 发送一个无意义的鼠标消息，让获取消息的线程醒来，并检测一次退出标志
					if (this->m_getMessageThreadPtr->joinable()) {
						this->m_getMessageThreadPtr->join(); // 线程回归
					}
					else {
						throw WideError(L"获取鼠标消息的线程无法回归");
					}
					return;
				}
				else {
					throw WideError(L"未知选项");
				}
			}
		}

		void RunVisualSortMenu() const {
			const static size_t sortNum = GetVisualSort().GetSorts().size(); // 总共有几个排序
			const static size_t pageNum = sortNum / 21 + (sortNum % 21 != 0 ? 1 : 0); // 有几页排序（一页最多21个）
			static size_t nowPage = 0; // 现在是哪一页
			static ButtonSequence buttons; // 按钮们，包括返回按钮（红叉叉）、上下一页、排序按钮
			using F = Fraction; // 分数类
			static auto DrawPageInform = []() ->void { // 本地函数，绘制页数信息（比如 1 / 2 页）
				Sketch pageInform(ComputeRect(GetConfigManager().GetCanvasRect(),
					F(51, 112), F(20, 21), F(61, 112), F(1)),
					std::to_wstring(nowPage + 1) + L" / " + std::to_wstring(pageNum)
				);
				pageInform.SetHasBackground(false).SetHasFrame(false).DrawSketch(false);
			};
			while (true) {
				buttons.Clear(); // 重置按钮们
				std::optional<size_t> choice = std::nullopt; // 选择了哪个按钮
				int crossSize = (std::min)(GetConfigManager().GetWidth() / 38, GetConfigManager().GetHeight() / 24); // 红叉叉的大小
				buttons.AddButtonAsCross(Coordinate(GetConfigManager().GetWidth() - crossSize, crossSize), crossSize, [&choice](Button&) {choice = 0; });
				constexpr F bWidth(433, 1500);
				constexpr F bHeight(3, 28);
				constexpr F horiGap(1, 30);
				constexpr F vertiGap(1, 84);
				constexpr F lMargin(1, 30);
				constexpr F tMargin(2, 21);
				for (size_t i = 0; i < 21 && nowPage * 21 + i < sortNum; ++i) {
					size_t sortIndex = nowPage * 21 + i;
					buttons.AddButton(ComputeRect(GetConfigManager().GetCanvasRect(),
						lMargin + (i / 7) * (bWidth + horiGap),
						tMargin + (i % 7) * (bHeight + vertiGap),
						lMargin + (i / 7) * (bWidth + horiGap) + bWidth,
						tMargin + (i % 7) * (bHeight + vertiGap) + bHeight
					), GetVisualSort().GetSorts()[sortIndex].GetSortName(), [sortIndex](Button&, ExMessage) {
						InputBox inputBox;
						inputBox.SetTitleText(GetVisualSort().GetSorts()[sortIndex].GetSortName());
						inputBox.SetMaxNum(GetVisualSort().GetSorts()[sortIndex].GetMaxSize());
						std::wstring contentText = L"数值不超过" + std::to_wstring(GetVisualSort().GetSorts()[sortIndex].GetMaxSize());
						for (auto it = GetVisualSort().GetSorts()[sortIndex].GetNumRequires().begin();
							it != GetVisualSort().GetSorts()[sortIndex].GetNumRequires().end(); ++it) {
							contentText += L"\n" + it->GetRequireInform();
						}
						inputBox.SetContentText(contentText);
						inputBox.SetExcutFunc([&inputBox, sortIndex](Button& button_, ExMessage) {
							size_t resultNum = inputBox.GetInputNum();
							std::vector<std::wstring> errorMessages;
							if (resultNum > 1) {
								if (resultNum > GetVisualSort().GetSorts()[sortIndex].GetMaxSize()) {
									errorMessages.emplace_back(L"数据量超过允许最大值");
								}
								for (size_t i = 0; i < GetVisualSort().GetSorts()[sortIndex].GetNumRequires().size(); ++i) {
									if (!GetVisualSort().GetSorts()[sortIndex].GetNumRequires()[i].Check(resultNum)) {
										errorMessages.emplace_back(GetVisualSort().GetSorts()[sortIndex].GetNumRequires()[i].GetRequireInform());
									}
								}
								if (errorMessages.empty()) {
									GetVisualSort().SortPreparation(sortIndex, resultNum);
									inputBox.SetExitFlag(true);
								}
								else {
									Dialog prompt(errorMessages);
									prompt.RunBlockDialog();
									GetDrawingTool().ClearDevice();
									buttons.DrawButtons(false);
									DrawPageInform();
									inputBox.DrawInputBox();
								}
							}
							Button::GetDefaultHoverDrawFunction()(button_, {});
						});
						inputBox.RunBlockInputLoop();
						GetDrawingTool().ClearDevice();
						if (pageNum > 1) {
							DrawPageInform();
						}
						buttons.DrawButtons();
					});
				}
				if (pageNum > 1) {
					buttons.AddButton(ComputeRect(GetConfigManager().GetCanvasRect(),
						F(41, 112), F(20, 21), F(51, 112), F(1)), L"上一页", [&choice](Button& button_, ExMessage) {
							if(nowPage > 0) {
								choice = 1;
								buttons.SetExitFlag(true);
							}
							Button::GetDefaultHoverDrawFunction()(button_, {});
						}
					);
					buttons.AddButton(ComputeRect(GetConfigManager().GetCanvasRect(),
						F(61, 112), F(20, 21), F(71, 112), F(1)), L"下一页", [&choice](Button& button_, ExMessage) {
							if (nowPage + 1 < pageNum) {
								choice = 2;
								buttons.SetExitFlag(true);
							}
							Button::GetDefaultHoverDrawFunction()(button_, {});
						}
					);
				}

				GetDrawingTool().ClearDevice();
				if (pageNum > 1) {
					DrawPageInform();
				}
				buttons.RunBlockButtonLoop();
				if (!choice.has_value()) {
					throw WideError(L"未选择任何选项");
				}
				else if (choice.value() == 0) {
					return;
				}
				else if (choice.value() == 1) {
					if (nowPage > 0) {
						--nowPage;
					}
				}
				else if (choice.value() == 2) {
					if (nowPage + 1 < pageNum) {
						++nowPage;
					}
				}
				else {
					throw WideError(L"未知选项");
				}
			}
		}

		void RunSetMenu() {
			static bool isShowShuffle = GetVisualSort().GetShowShuffle();
			isShowShuffle = GetVisualSort().GetShowShuffle();
			static bool isFullScreen = this->m_fullScreen;
			isFullScreen = this->m_fullScreen;
			using F = Fraction;
			F mainHeight = 8 * GetConfigManager().GetWidth() > 11 * GetConfigManager().GetHeight() ?
				GetConfigManager().GetHeight() / 2 : GetConfigManager().GetWidth() / 2;
			F mainWidth = mainHeight * F(11, 8);
			static Sketch mainSketch;
			mainSketch.SetFrameRect(RECT(
				GetConfigManager().GetCenterX() - (mainWidth / 2),
				GetConfigManager().GetCenterY() - (mainHeight / 2),
				GetConfigManager().GetCenterX() + (mainWidth / 2),
				GetConfigManager().GetCenterY() + (mainHeight / 2)
			)).SetHasBackground(false);
			static std::vector<Sketch> sketches(4);
			constexpr F sWidth(6, 11);
			constexpr F sHeight(1, 8);
			constexpr F lMargin(1, 22);
			constexpr F tMargin(1, 16);
			constexpr F vertiGap(1, 16);
			constexpr const wchar_t* sketchTexts[] = { L"显示打乱过程",L"窗口全屏显示",L"调整窗口宽度",L"调整窗口高度" };
			for (size_t i = 0; i < 4; ++i) {
				sketches[i].SetSketch(ComputeRect(mainSketch.GetFrameRect(),
					lMargin, tMargin + i * (sHeight + vertiGap),
					lMargin + sWidth, tMargin + i * (sHeight + vertiGap) + sHeight),
					sketchTexts[i]
				).SetHasBackground(false).SetHasFrame(false);
			}
			static ButtonSequence buttons(6);
			static auto DrawSetMenuFunc = []() {
				mainSketch.DrawSketch(false);
				for (size_t i = 0; i < sketches.size(); ++i) {
					sketches[i].DrawSketch(false);
				}
				buttons.DrawButtons(false);
			};
			buttons.SetButtonAsSwitch(0, ComputeRect(mainSketch.GetFrameRect(),
				F(15, 22), F(1, 16), F(21, 22), F(3, 16)), isShowShuffle
			);
			buttons.SetButtonAsSwitch(1, ComputeRect(mainSketch.GetFrameRect(),
				F(15, 22), F(1, 4), F(21, 22), F(3, 8)), isFullScreen, []() {
					if (isFullScreen) {
						buttons.GetButtons()[2].GetSketch().SetTextWithoutResize(std::to_wstring(static_cast<int>(GetConfigManager().GetMaxWidth())));
						buttons.GetButtons()[3].GetSketch().SetTextWithoutResize(std::to_wstring(static_cast<int>(GetConfigManager().GetMaxHeight())));
						buttons.GetButtons()[2].GetSketch().DrawSketch();
						buttons.GetButtons()[3].GetSketch().DrawSketch();
					}
					else {
						buttons.GetButtons()[3].GetSketch().SetTextWithoutResize(std::to_wstring(static_cast<int>(GetConfigManager().GetMaxClientHeight())));
						buttons.GetButtons()[3].GetSketch().DrawSketch();
					}
				}
			);

			buttons.GetButtons()[2].SetButton(ComputeRect(mainSketch.GetFrameRect(),
				F(15, 22), F(7, 16), F(21, 22), F(9, 16)),
				std::to_wstring(static_cast<int>(GetConfigManager().GetWidth())), [](Button& button_, ExMessage) {
					if (!isFullScreen) {
						InputBox inputBox;
						inputBox.SetTitleText(L"输入窗口宽度").
							SetContentText(L"数值不超过" + std::to_wstring(static_cast<int>(GetConfigManager().GetMaxWidth())) + L"\n" +
							L"数值不小于" + std::to_wstring(static_cast<int>(GetConfigManager().GetMinWidth()))).
							SetMaxNum(GetConfigManager().GetMaxWidth()).SetCrossFunc([]() {
								GetDrawingTool().ClearDevice();
								DrawSetMenuFunc();
								GetDrawingTool().FlushBatchDraw();
							}).
							SetExcutFunc([&button_, &inputBox](Button&, ExMessage) {
							size_t result = inputBox.GetInputNum();
							if (result < static_cast<size_t>(GetConfigManager().GetMinWidth())) {
								Dialog prompt(L"数值过小");
								prompt.RunBlockDialog();
							}
							else if (result > static_cast<size_t>(GetConfigManager().GetMaxWidth())) {
								Dialog prompt(L"数值过大");
								prompt.RunBlockDialog();
							}
							else {
								inputBox.SetExitFlag(true);
								button_.GetSketch().SetTextWithoutResize(std::to_wstring(result));
								GetDrawingTool().ClearDevice();
								DrawSetMenuFunc();
								GetDrawingTool().FlushBatchDraw();
								return;
							}
							GetDrawingTool().ClearDevice();
							inputBox.DrawInputBox();
						});
						inputBox.RunBlockInputLoop();
					}
					else {
						Dialog prompt(L"全屏状态下，不能修改窗口宽度");
						prompt.RunBlockDialog();
						GetDrawingTool().ClearDevice();
						DrawSetMenuFunc();
						GetDrawingTool().FlushBatchDraw();
					}
				}
			).GetSketch().SetTextMode(DT_LEFT).SetFrameRoundSize(0);

			buttons.GetButtons()[3].SetButton(ComputeRect(mainSketch.GetFrameRect(),
				F(15, 22), F(5, 8), F(21, 22), F(3, 4)),
				std::to_wstring(static_cast<int>(GetConfigManager().GetHeight())), [](Button& button_, ExMessage) {
					if (!isFullScreen) {
						InputBox inputBox;
						inputBox.SetTitleText(L"输入窗口高度").
							SetContentText(L"数值不超过" + std::to_wstring(static_cast<int>(GetConfigManager().GetMaxClientHeight())) + L"\n" +
								L"数值不小于" + std::to_wstring(static_cast<int>(GetConfigManager().GetMinHeight()))).
							SetMaxNum(GetConfigManager().GetMaxClientHeight()).SetCrossFunc([]() {
								GetDrawingTool().ClearDevice();
								DrawSetMenuFunc();
								GetDrawingTool().FlushBatchDraw();
							}).
							SetExcutFunc([&button_, &inputBox](Button&, ExMessage) {
							size_t result = inputBox.GetInputNum();
							if (result < static_cast<size_t>(GetConfigManager().GetMinHeight())) {
								Dialog prompt(L"数值过小");
								prompt.RunBlockDialog();
							}
							else if (result > static_cast<size_t>(GetConfigManager().GetMaxHeight())) {
								Dialog prompt(L"数值过大");
								prompt.RunBlockDialog();
							}
							else {
								inputBox.SetExitFlag(true);
								button_.GetSketch().SetTextWithoutResize(std::to_wstring(result));
								GetDrawingTool().ClearDevice();
								DrawSetMenuFunc();
								GetDrawingTool().FlushBatchDraw();
								return;
							}
							GetDrawingTool().ClearDevice();
							inputBox.DrawInputBox();
						});
						inputBox.RunBlockInputLoop();
					}
					else {
						Dialog prompt(L"全屏状态下，不能修改窗口高度");
						prompt.RunBlockDialog();
						GetDrawingTool().ClearDevice();
						DrawSetMenuFunc();
						GetDrawingTool().FlushBatchDraw();
					}
				}
			).GetSketch().SetTextMode(DT_LEFT).SetFrameRoundSize(0);

			static std::optional<bool> confirm = std::nullopt;
			confirm = std::nullopt;
			buttons.GetButtons()[4].SetButton(ComputeRect(mainSketch.GetFrameRect(),
				F(2, 11), F(13, 16), F(4, 11), F(15, 16)), L"确认", [](Button&, ExMessage) {
					confirm = true;
					buttons.SetExitFlag(true);
				}
			).GetSketch().SetFrameRoundSize(5);
			buttons.GetButtons()[5].SetButton(ComputeRect(mainSketch.GetFrameRect(),
				F(7, 11), F(13, 16), F(9, 11), F(15, 16)), L"取消", [](Button&, ExMessage) {
					confirm = false;
					buttons.SetExitFlag(true);
				}
			).GetSketch().SetFrameRoundSize(5);

			GetDrawingTool().ClearDevice();
			DrawSetMenuFunc();
			buttons.RunBlockButtonLoop();

			ButtonSequence::s_exitGetMessage.store(true, std::memory_order_release);
			PostMessage(GetHWnd(), WM_MOUSEMOVE, 0, MAKELPARAM(0, 0));
			if (this->m_getMessageThreadPtr->joinable()) {
				this->m_getMessageThreadPtr->join();
			}
			else {
				throw WideError(L"获取鼠标消息的线程无法回归");
			}

			if (!confirm.has_value()) {
				throw WideError(L"未知的选项！");
			}
			else if (confirm.value()) {
				GetVisualSort().SetShowShuffle(isShowShuffle);
				if (isFullScreen) {
					if (!this->m_fullScreen) {
						this->m_fullScreen = true;
						this->FullScreen();
					}
				}
				else {
					if (this->m_fullScreen) {
						this->m_fullScreen = false;
						GetConfigManager().SetWidth(std::stoi(buttons.GetButtons()[2].GetSketch().GetText()));
						GetConfigManager().SetHeight(std::stoi(buttons.GetButtons()[3].GetSketch().GetText()));
						this->ResizeWindow();
					}
					else {
						int tempWidth = std::stoi(buttons.GetButtons()[2].GetSketch().GetText());
						int tempHeight = std::stoi(buttons.GetButtons()[3].GetSketch().GetText());
						if (F(tempWidth) != GetConfigManager().GetWidth() || F(tempHeight) != GetConfigManager().GetHeight()) {
							GetConfigManager().SetWidth(tempWidth);
							GetConfigManager().SetHeight(tempHeight);
							this->ResizeWindow();
						}
					}
				}
			}
			this->m_getMessageThreadPtr = std::make_shared<std::thread>(ButtonSequence::GetMessageLoop);
		}

	public:

		friend inline MainMenu& GetMainMenu();

	};

	inline MainMenu& GetMainMenu() {
		static MainMenu instance;
		return instance;
	}

}