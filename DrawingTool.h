#pragma once
#include <string>
#include <mutex>
#include <functional>
#include <easyx.h>
#include <utility>
#include <vector>
#include <Windows.h>
#include "Coordinate.h"
#include "WideError.h"
#include "ConfigManager.h"

namespace NVisualSort {

	class DrawingTool {

	private:

		std::recursive_mutex m_drawMutex;

		// 单例模式下，禁止拷贝和移动
		DrawingTool() noexcept = default;
		DrawingTool(const DrawingTool&) = delete;
		DrawingTool& operator=(const DrawingTool&) = delete;
		DrawingTool(DrawingTool&&) = delete;
		DrawingTool& operator=(DrawingTool&&) = delete;

	public:

		// 支持泛化绘制函数
		void ExecuteWithLock(const std::function<void()>& func_) {
			if (func_) {
				std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
				func_();
			}
		}

		// 清空窗口
		void ClearDevice() noexcept {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::cleardevice();
		}

		// 清除矩形区域
		void ClearRectangle(RECT rect_) noexcept {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::clearrectangle(rect_.left, rect_.top, rect_.right, rect_.bottom);
		}

		void ClearRectangle(int left_, int top_, int right_, int bottom_) noexcept {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::clearrectangle(left_, top_, right_, bottom_);
		}

		// 绘制线
		void Line(std::pair<Coordinate, Coordinate> start_end_points_, int line_thick_, int line_style_, COLORREF line_color_) {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::setlinestyle(line_style_, line_thick_);
			::setlinecolor(line_color_);
			::line(start_end_points_.first.x, start_end_points_.first.y, start_end_points_.second.x, start_end_points_.second.y);
		}

		// 填充矩形区域（包含边框）
		void FillRectangle(RECT rect_, int frame_thick_, int frame_style_, COLORREF frame_color_, COLORREF background_color_) {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::setlinestyle(frame_style_, frame_thick_);
			::setlinecolor(frame_color_);
			::setfillcolor(background_color_);
			::fillrectangle(rect_.left, rect_.top, rect_.right, rect_.bottom);
		}

		// 填充多边形（包含边框）
		void FillPolygon(const std::vector<Coordinate>& points_, int frame_thick_, int frame_style_, COLORREF frame_color_, COLORREF background_color_) {
			if (points_.size() < 3) {
				throw WideError(L"多边形顶点数量过少！数量为：" + std::to_wstring(points_.size()));
			}
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::setlinestyle(frame_style_, frame_thick_);
			::setlinecolor(frame_color_);
			::setfillcolor(background_color_);
			::fillpolygon(points_.front().AsPointPtr(), static_cast<int>(points_.size()));
		}

		// 填充圆角矩形区域（包含边框）
		void FillRoundRect(RECT rect_, int ellipse_width_, int ellipse_height_, int frame_thick_, int frame_style_, COLORREF frame_color_, COLORREF background_color_) {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::setlinestyle(frame_style_, frame_thick_);
			::setlinecolor(frame_color_);
			::setfillcolor(background_color_);
			::fillroundrect(rect_.left, rect_.top, rect_.right, rect_.bottom, ellipse_width_, ellipse_height_);
		}

		void SolidCircle(Coordinate center_, int radius_, COLORREF background_color_) {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::setfillcolor(background_color_);
			::solidcircle(center_.x, center_.y, radius_);
		}

		// 填充矩形区域（不包含边框）
		void SolidRectangle(RECT rect_, COLORREF background_color_) {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::setfillcolor(background_color_);
			::solidrectangle(rect_.left, rect_.top, rect_.right, rect_.bottom);
		}

		// 填充多边形（不包含边框）
		void SolidPolygon(const std::vector<Coordinate>& points_, COLORREF background_color_) {
			if (points_.size() < 3) {
				throw WideError(L"多边形顶点数量过少！数量为：" + std::to_wstring(points_.size()));
			}
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::setfillcolor(background_color_);
			::solidpolygon(points_.front().AsPointPtr(), static_cast<int>(points_.size()));
		}

		// 填充圆角矩形区域（不包含边框）
		void SolidRoundRect(RECT rect_, int ellipse_width_, int ellipse_height_, COLORREF background_color_) {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::setfillcolor(background_color_);
			::solidroundrect(rect_.left, rect_.top, rect_.right, rect_.bottom, ellipse_width_, ellipse_height_);
		}

		// 绘制圆角矩形边框
		void RoundRect(RECT rect_, int ellipse_width_, int ellipse_height_, int frame_thick_, int frame_style_, COLORREF frame_color_) {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::setlinestyle(frame_style_, frame_thick_);
			::setlinecolor(frame_color_);
			::roundrect(rect_.left, rect_.top, rect_.right, rect_.bottom, ellipse_width_, ellipse_height_);
		}

		// 刷新区域
		void FlushBatchDraw() noexcept {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::FlushBatchDraw();
		}

		// 刷新区域
		void FlushBatchDraw(int left_, int top_, int right_, int bottom_) noexcept {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::FlushBatchDraw(left_, top_, right_, bottom_);
		}

		// 刷新区域
		void FlushBatchDraw(RECT rect_) noexcept {
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::FlushBatchDraw(rect_.left, rect_.top, rect_.right, rect_.bottom);
		}

		// 绘制文本
		void DrawText_(const std::wstring& text_, RECT rect_, int text_size_, COLORREF text_color_, UINT text_mode_ = DT_CENTER | DT_VCENTER, const std::wstring& font_ = DefaultTypeface) {
			if(text_.empty()) {
				return;
			}
			std::lock_guard<std::recursive_mutex> lock(this->m_drawMutex);
			::settextcolor(text_color_);
			::settextstyle(text_size_, 0, font_.c_str());
			::drawtext(text_.c_str(), &rect_, text_mode_);
		}

		friend inline DrawingTool& GetDrawingTool() noexcept;

	};

	// 获取绘制工具实例
	inline DrawingTool& GetDrawingTool() noexcept {
		static DrawingTool instance;
		return instance;
	}

}