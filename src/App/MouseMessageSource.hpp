#pragma once
#include "Channel.hpp"
#include <Windows.h>
#include <easyx.h>
#include <atomic>
#include <thread>

namespace NVisualSort {

	/**
	 * @brief 全局鼠标消息源。单例，内部持有一个 Channel<ExMessage> 和采集线程。
	 *
	 * 采集线程从 EasyX 拉鼠标消息并广播到 Channel。
	 * 由于 ::getmessage 是阻塞的，Stop() 会通过 PostMessage 发送一个假鼠标消息唤醒它。
	 * 注意：PostMessage 需要有效的 HWND，因此必须在 initgraph 之后调用 Start()。
	 *
	 * 单例故意用 new 泄漏，避免静态析构顺序问题（ButtonSequence 析构时可能仍需要它）。
	 */
	class MouseMessageSource {

	private:

		Channel<ExMessage> m_channel;
		std::atomic<bool> m_running{ false };
		std::thread m_thread;

		MouseMessageSource() = default;
		MouseMessageSource(const MouseMessageSource&) = delete;
		MouseMessageSource& operator=(const MouseMessageSource&) = delete;
		MouseMessageSource(MouseMessageSource&&) = delete;
		MouseMessageSource& operator=(MouseMessageSource&&) = delete;

		static void GetMessageLoop(MouseMessageSource* self_) {
			ExMessage msg;
			while (self_->m_running.load(std::memory_order_acquire)) {
				::getmessage(&msg, EX_MOUSE);
				if (!self_->m_running.load(std::memory_order_acquire)) {
					return;
				}
				self_->m_channel.Send(msg);
			}
		}

	public:

		static MouseMessageSource& GetInstance() noexcept {
			static MouseMessageSource* instance = new MouseMessageSource();
			return *instance;
		}

		/** 启动采集线程。重复调用是幂等的。 */
		void Start() {
			if (this->m_running.exchange(true, std::memory_order_acq_rel)) {
				return;
			}
			this->m_thread = std::thread(MouseMessageSource::GetMessageLoop, this);
		}

		/**
		 * @brief 停止采集线程。通过 PostMessage 唤醒卡在 getmessage 的线程。
		 *		Channel 保持打开，后续可再次 Start() 重启。
		 */
		void Stop() {
			if (!this->m_running.exchange(false, std::memory_order_acq_rel)) {
				return;
			}
			::PostMessage(::GetHWnd(), WM_MOUSEMOVE, 0, MAKELPARAM(0, 0));
			if (this->m_thread.joinable()) {
				this->m_thread.join();
			}
		}

		bool IsRunning() const noexcept {
			return this->m_running.load(std::memory_order_acquire);
		}

		Channel<ExMessage>& GetChannel() noexcept {
			return this->m_channel;
		}

	};

}