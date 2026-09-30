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
				if (this->m_active) {
					this->ExecuteNow();
				}
				this->m_cleanupFunctions = std::move(other_.m_cleanupFunctions);
				this->m_active = other_.m_active;
				other_.m_active = false;
			}
			return *this;
		}

		ScopeGuard(const ScopeGuard&) = delete;
		ScopeGuard& operator=(const ScopeGuard&) = delete;

		~ScopeGuard() noexcept {
			if (this->m_active) {
				ExecuteNow();
			}
		}

		template<typename Func>
		ScopeGuard& AddFront(Func&& func_) {
			if (this->m_active) {
				this->m_cleanupFunctions.emplace_front(std::forward<Func>(func_));
			}
			return *this;
		}

		template<typename Func>
		ScopeGuard& AddBack(Func&& func_) {
			if (this->m_active) {
				this->m_cleanupFunctions.emplace_back(std::forward<Func>(func_));
			}
			return *this;
		}

		template<typename Func>
		ScopeGuard& operator+=(Func&& func_) {
			return AddBack(std::forward<Func>(func_));
		}

		template<typename Func>
		ScopeGuard& Add(Func&& func_) {
			return AddBack(std::forward<Func>(func_));
		}

		void ExecuteNow() noexcept {
			while (!this->m_cleanupFunctions.empty()) {
				try {
					auto& func = this->m_cleanupFunctions.back();
					if (func) {
						func();
					}
				}
				catch (...) {
				}
				this->m_cleanupFunctions.pop_back();
			}
		}

		void Dismiss() noexcept {
			this->m_active = false;
			this->m_cleanupFunctions.clear();
		}

		void Reactivate() noexcept {
			this->m_active = true;
		}

		bool IsActive() const noexcept {
			return this->m_active;
		}

		size_t Size() const noexcept {
			return this->m_cleanupFunctions.size();
		}

		bool Empty() const noexcept {
			return this->m_cleanupFunctions.empty();
		}

		template<typename Func>
		[[nodiscard("Create 的返回值不应该被忽略，否则清理会立即进行！")]] static ScopeGuard Create(Func&& func_) {
			ScopeGuard guard;
			guard.AddBack(std::forward<Func>(func_));
			return guard;
		}

	};

}