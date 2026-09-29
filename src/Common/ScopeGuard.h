#pragma once
#include <deque>
#include <functional>
#include <utility>

namespace NVisualSort {

	class ScopeGuard {

	private:

		std::deque<std::function<void()>> m_cleanupFunctions;
		bool m_active = true;

	public:

		ScopeGuard() noexcept = default;

		template<typename Func>
		explicit ScopeGuard(Func&& func) {
			m_cleanupFunctions.emplace_back(std::forward<Func>(func));
		}

		ScopeGuard(ScopeGuard&& other_) noexcept
			: m_cleanupFunctions(std::move(other_.m_cleanupFunctions))
			, m_active(other_.m_active) {
			other_.m_active = false;
		}

		ScopeGuard& operator=(ScopeGuard&& other_) noexcept {
			if (this != &other_) {
				// 首先执行当前对象的清理函数
				if (this->m_active) {
					this->ExecuteNow();
				}
				this->m_cleanupFunctions = std::move(other_.m_cleanupFunctions);
				this->m_active = other_.m_active;
				other_.m_active = false;
			}
			return *this;
		}

		// 禁止拷贝（避免重复清理）
		ScopeGuard(const ScopeGuard&) = delete;
		ScopeGuard& operator=(const ScopeGuard&) = delete;

		// 析构函数 - 自动执行清理
		~ScopeGuard() noexcept {
			if (this->m_active) {
				ExecuteNow();
			}
		}

		// 添加清理函数到队列头部（最后添加的首先执行 - LIFO）
		template<typename Func>
		ScopeGuard& AddFront(Func&& func_) {
			if (this->m_active) {
				this->m_cleanupFunctions.emplace_front(std::forward<Func>(func_));
			}
			return *this;
		}

		// 添加清理函数到队列尾部（按添加顺序执行 - FIFO）
		template<typename Func>
		ScopeGuard& AddBack(Func&& func_) {
			if (this->m_active) {
				this->m_cleanupFunctions.emplace_back(std::forward<Func>(func_));
			}
			return *this;
		}

		// 添加清理函数（默认添加到尾部）
		template<typename Func>
		ScopeGuard& operator+=(Func&& func_) {
			return AddBack(std::forward<Func>(func_));
		}

		// 添加清理函数并返回自身的引用（链式调用）
		template<typename Func>
		ScopeGuard& Add(Func&& func_) {
			return AddBack(std::forward<Func>(func_));
		}

		// 立即执行所有清理函数（无论是否active）
		void ExecuteNow() noexcept {
			while (!this->m_cleanupFunctions.empty()) {
				try {
					auto& func = this->m_cleanupFunctions.back();
					if (func) {
						func();
					}
				}
				catch (...) {
					// 忽略清理函数中的异常，防止异常传播
				}
				this->m_cleanupFunctions.pop_back();
			}
		}

		// 取消ScopeGuard（析构时不执行清理）
		void Dismiss() noexcept {
			this->m_active = false;
			this->m_cleanupFunctions.clear();
		}

		// 重新激活ScopeGuard（如果之前被取消）
		void Reactivate() noexcept {
			this->m_active = true;
		}

		// 检查是否处于活动状态
		bool IsActive() const noexcept {
			return this->m_active;
		}

		// 获取当前待执行的清理函数数量
		size_t Size() const noexcept {
			return this->m_cleanupFunctions.size();
		}

		// 检查是否为空
		bool Empty() const noexcept {
			return this->m_cleanupFunctions.empty();
		}

		// 创建ScopeGuard并立即添加一个清理函数（工厂函数）
		template<typename Func>
		[[nodiscard("Create 的返回值不应该被忽略，否则清理会立即进行！")]] static ScopeGuard Create(Func&& func_) {
			ScopeGuard guard;
			guard.AddBack(std::forward<Func>(func_));
			return guard;
		}

	};

}