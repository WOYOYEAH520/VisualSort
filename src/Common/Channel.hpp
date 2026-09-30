#pragma once
#include <atomic>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>
#include <condition_variable>
#include <chrono>

namespace NVisualSort {

	template <typename T> class Channel;

	template <typename T> class Sender {

	private:

		Channel<T>* m_channel = nullptr;

		friend class Channel<T>;

		void BindToChannel(Channel<T>* channel_) noexcept {
			this->m_channel = channel_;
		}

	public:

		Sender() = default;
		Sender(const Sender&) = delete;
		Sender& operator=(const Sender&) = delete;
		Sender(Sender&&) = delete;
		Sender& operator=(Sender&&) = delete;

		virtual ~Sender() = default;

		bool Send(const T& message_);
		void Close();
		bool IsClosed() const noexcept;

	};

	template <typename T> class Receiver {

	private:

		std::queue<T> m_messages;
		mutable std::mutex m_mutex;
		std::condition_variable m_condition;
		std::atomic<bool> m_closed{ false };

		friend class Channel<T>;

		// 仅供 Channel 调用：入队一条消息
		void Push(const T& message_) {
			{
				std::lock_guard<std::mutex> lock(this->m_mutex);
				this->m_messages.push(message_);
			}
			this->m_condition.notify_one();
		}

		// 仅供 Channel 调用：标记关闭，唤醒所有等待者
		void MarkClosed() {
			this->m_closed.store(true, std::memory_order_release);
			this->m_condition.notify_all();
		}

	public:

		Receiver() = default;
		Receiver(const Receiver&) = delete;
		Receiver& operator=(const Receiver&) = delete;
		Receiver(Receiver&&) = delete;
		Receiver& operator=(Receiver&&) = delete;
		virtual ~Receiver() = default;

		/**
		 * @brief 阻塞式接收。
		 * @return 取到消息返回 true；Channel 已关闭且队列为空返回 false。
		 */
		bool Receive(T& out_) {
			std::unique_lock<std::mutex> lock(this->m_mutex);
			this->m_condition.wait(lock, [this] {
				return !this->m_messages.empty() ||
					this->m_closed.load(std::memory_order_acquire);
				});
			if (this->m_messages.empty()) {
				return false;
			}
			out_ = std::move(this->m_messages.front());
			this->m_messages.pop();
			return true;
		}

		/** 带超时的阻塞接收。 */
		template <typename Rep, typename Period>
		bool ReceiveFor(T& out_, const std::chrono::duration<Rep, Period>& timeout_) {
			std::unique_lock<std::mutex> lock(this->m_mutex);
			bool signaled = this->m_condition.wait_for(lock, timeout_, [this] {
				return !this->m_messages.empty() ||
					this->m_closed.load(std::memory_order_acquire);
				});
			if (!signaled || this->m_messages.empty()) {
				return false;
			}
			out_ = std::move(this->m_messages.front());
			this->m_messages.pop();
			return true;
		}

		/** 非阻塞式接收。 */
		bool TryReceive(T& out_) {
			std::lock_guard<std::mutex> lock(this->m_mutex);
			if (this->m_messages.empty()) {
				return false;
			}
			out_ = std::move(this->m_messages.front());
			this->m_messages.pop();
			return true;
		}

		bool Empty() const {
			std::lock_guard<std::mutex> lock(this->m_mutex);
			return this->m_messages.empty();
		}

		std::size_t Size() const {
			std::lock_guard<std::mutex> lock(this->m_mutex);
			return this->m_messages.size();
		}

		bool IsClosed() const noexcept {
			return this->m_closed.load(std::memory_order_acquire);
		}

		std::queue<T> DrainMessages() {
			std::lock_guard<std::mutex> lock(this->m_mutex);
			std::queue<T> drained;
			std::swap(drained, this->m_messages);
			return drained;
		}

		std::size_t ClearMessages() {
			std::lock_guard<std::mutex> lock(this->m_mutex);
			std::size_t count = this->m_messages.size();
			std::queue<T> empty;
			std::swap(this->m_messages, empty);
			return count;
		}

		bool TryTakeLatest(T& out_) {
			std::lock_guard<std::mutex> lock(this->m_mutex);
			if (this->m_messages.empty()) {
				return false;
			}
			while (this->m_messages.size() > 1) {
				this->m_messages.pop();
			}
			out_ = std::move(this->m_messages.front());
			this->m_messages.pop();
			return true;
		}

		bool WaitTakeLatest(T& out_) {
			std::unique_lock<std::mutex> lock(this->m_mutex);
			this->m_condition.wait(lock, [this] {
				return !this->m_messages.empty() ||
					this->m_closed.load(std::memory_order_acquire);
				});
			if (this->m_messages.empty()) {
				return false;
			}
			while (this->m_messages.size() > 1) {
				this->m_messages.pop();
			}
			out_ = std::move(this->m_messages.front());
			this->m_messages.pop();
			return true;
		}

	};

	template <typename T> class Channel {

	private:

		std::atomic<bool> m_closed{ false };
		std::unique_ptr<Sender<T>> m_sender;
		std::mutex m_changeSenderMutex;
		std::map<Receiver<T>*, std::unique_ptr<Receiver<T>>> m_receivers;
		std::mutex m_changeReceiversMutex;

	public:

		Channel() = default;
		Channel(const Channel&) = delete;
		Channel& operator=(const Channel&) = delete;
		Channel(Channel&&) = delete;
		Channel& operator=(Channel&&) = delete;

		~Channel() {
			this->Close();
		}

		Channel(std::unique_ptr<Sender<T>> sender_,
			std::unique_ptr<Receiver<T>> receiver_) {
			this->SetSender(std::move(sender_));
			this->AddReceiver(std::move(receiver_));
		}

		Channel(std::unique_ptr<Sender<T>> sender_,
			std::list<std::unique_ptr<Receiver<T>>> receivers_) {
			this->SetSender(std::move(sender_));
			for (auto& receiver : receivers_) {
				this->AddReceiver(std::move(receiver));
			}
		}

		void SetSender(std::unique_ptr<Sender<T>> sender_) {
			std::lock_guard<std::mutex> lock(this->m_changeSenderMutex);
			if (this->m_sender != nullptr) {
				this->m_sender->BindToChannel(nullptr);
			}
			this->m_sender = std::move(sender_);
			if (this->m_sender != nullptr) {
				this->m_sender->BindToChannel(this);
			}
		}

		/** 取回 Sender 的所有权（从 Channel 解绑）。 */
		std::unique_ptr<Sender<T>> ReleaseSender() {
			std::lock_guard<std::mutex> lock(this->m_changeSenderMutex);
			if (this->m_sender != nullptr) {
				this->m_sender->BindToChannel(nullptr);
			}
			return std::move(this->m_sender);
		}

		void AddReceiver(std::unique_ptr<Receiver<T>> receiver_) {
			if (!receiver_) {
				return;
			}
			std::lock_guard<std::mutex> lock(this->m_changeReceiversMutex);
			Receiver<T>* raw = receiver_.get();
			if (this->m_closed.load(std::memory_order_acquire)) {
				receiver_->MarkClosed();
			}
			this->m_receivers.emplace(raw, std::move(receiver_));
		}

		/**
		 * @brief 删除一个 Receiver（Channel 回收其所有权）。
		 * @return 找到并删除返回 true；未找到返回 false。
		 */
		bool DeleteReceiver(Receiver<T>* receiver_) {
			if (!receiver_) {
				return false;
			}
			std::lock_guard<std::mutex> lock(this->m_changeReceiversMutex);
			auto it = this->m_receivers.find(receiver_);
			if (it == this->m_receivers.end()) {
				return false;
			}
			it->second->MarkClosed();
			this->m_receivers.erase(it);
			return true;
		}

		/**
		 * @brief 从 Channel 解绑 Receiver 并把所有权交还给调用者。
		 *		由调用者负责后续销毁。
		 */
		std::unique_ptr<Receiver<T>> ReleaseReceiver(Receiver<T>* receiver_) {
			if (!receiver_) {
				return nullptr;
			}
			std::lock_guard<std::mutex> lock(this->m_changeReceiversMutex);
			auto it = this->m_receivers.find(receiver_);
			if (it == this->m_receivers.end()) {
				return nullptr;
			}
			it->second->MarkClosed();
			auto receiver = std::move(it->second);
			this->m_receivers.erase(it);
			return receiver;
		}

		bool Send(const T& message_) {
			std::lock_guard<std::mutex> lock(this->m_changeReceiversMutex);
			if (this->m_closed.load(std::memory_order_acquire)) {
				return false;
			}
			bool sent_any = false;
			for (auto& pair : this->m_receivers) {
				pair.second->Push(message_);
				sent_any = true;
			}
			return sent_any;
		}

		void Close() {
			bool expected = false;
			if (!this->m_closed.compare_exchange_strong(expected, true)) {
				return;
			}
			std::lock_guard<std::mutex> lock(this->m_changeReceiversMutex);
			for (auto& pair : this->m_receivers) {
				pair.second->MarkClosed();
			}
		}

		bool IsClosed() const noexcept {
			return this->m_closed.load(std::memory_order_acquire);
		}
	};

	template <typename T>
	bool Sender<T>::Send(const T& message_) {
		if (this->m_channel == nullptr) {
			return false;
		}
		return this->m_channel->Send(message_);
	}

	template <typename T>
	void Sender<T>::Close() {
		if (this->m_channel != nullptr) {
			this->m_channel->Close();
		}
	}

	template <typename T>
	bool Sender<T>::IsClosed() const noexcept {
		return this->m_channel == nullptr || this->m_channel->IsClosed();
	}

}