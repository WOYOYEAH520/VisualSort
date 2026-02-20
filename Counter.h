#pragma once
#include <atomic>
#include <utility>
#include <vector>

namespace NVisualSort {

	inline std::atomic<size_t> ActualStepNum{0};

	class Counter {

	private:

		int m_value = 0;
		bool m_notTemp = false;

		inline void IncrementIfNeeded() const {
			if (this->m_notTemp) {
				++ActualStepNum;
			}
		}

		inline void IncrementIfNeeded(const Counter& other_) const {
			if (this->m_notTemp || other_.m_notTemp) {
				++ActualStepNum;
			}
		}

	public:

		Counter(int value_ = 0, bool not_temp_ = false) :m_value(value_), m_notTemp(not_temp_) {}

		Counter(const Counter& other_) {
			this->m_value = other_.m_value;
			this->m_notTemp = false;
			other_.IncrementIfNeeded();
		}

		void SetCounter(int value_, bool not_temp_) {
			this->m_value = value_;
			this->m_notTemp = not_temp_;
		}

		static void SetCounters(const std::vector<int>& data_, std::vector<Counter>& counters_) {
			counters_.resize(data_.size());
			for (size_t counterIndex = 0; counterIndex < data_.size(); ++counterIndex) {
				counters_[counterIndex].m_value = data_[counterIndex];
				counters_[counterIndex].m_notTemp = true;
			}
		}

		bool operator>(int value_) const {
			IncrementIfNeeded();
			return m_value > value_;
		}

		bool operator>(const Counter& counter_) const {
			IncrementIfNeeded(counter_);
			return m_value > counter_.m_value;
		}

		bool operator<(int value_) const {
			IncrementIfNeeded();
			return m_value < value_;
		}

		bool operator<(const Counter& counter_) const {
			IncrementIfNeeded(counter_);
			return m_value < counter_.m_value;
		}

		bool operator==(int value_) const {
			IncrementIfNeeded();
			return m_value == value_;
		}

		bool operator==(const Counter& counter_) const {
			IncrementIfNeeded(counter_);
			return m_value == counter_.m_value;
		}

		bool operator>=(int value_) const {
			IncrementIfNeeded();
			return m_value >= value_;
		}

		bool operator>=(const Counter& counter_) const {
			IncrementIfNeeded(counter_);
			return m_value >= counter_.m_value;
		}

		bool operator<=(int value_) const {
			IncrementIfNeeded();
			return m_value <= value_;
		}

		bool operator<=(const Counter& counter_) const {
			IncrementIfNeeded(counter_);
			return m_value <= counter_.m_value;
		}

		bool operator!=(int value_) const {
			IncrementIfNeeded();
			return m_value != value_;
		}

		bool operator!=(const Counter& counter_) const {
			IncrementIfNeeded(counter_);
			return m_value != counter_.m_value;
		}

		Counter& operator=(int value_) {
			IncrementIfNeeded();
			m_value = value_;
			return *this;
		}

		Counter& operator=(const Counter& counter_) {
			IncrementIfNeeded(counter_);
			m_value = counter_.m_value;
			return *this;
		}

		operator int() const {
			IncrementIfNeeded();
			return m_value;
		}

		Counter& operator+=(int value_) {
			IncrementIfNeeded();
			m_value += value_;
			return *this;
		}

		Counter& operator+=(const Counter& counter_) {
			IncrementIfNeeded(counter_);
			m_value += counter_.m_value;
			return *this;
		}

		Counter& operator-=(int value_) {
			IncrementIfNeeded();
			m_value -= value_;
			return *this;
		}

		Counter& operator-=(const Counter& counter_) {
			IncrementIfNeeded(counter_);
			m_value -= counter_.m_value;
			return *this;
		}

		Counter& operator*=(int value_) {
			IncrementIfNeeded();
			m_value *= value_;
			return *this;
		}

		Counter& operator*=(const Counter& counter_) {
			IncrementIfNeeded(counter_);
			m_value *= counter_.m_value;
			return *this;
		}

		Counter& operator/=(int value_) {
			IncrementIfNeeded();
			m_value /= value_;
			return *this;
		}

		Counter& operator/=(const Counter& counter_) {
			IncrementIfNeeded(counter_);
			m_value /= counter_.m_value;
			return *this;
		}

		Counter& operator%=(int value_) {
			IncrementIfNeeded();
			m_value %= value_;
			return *this;
		}

		Counter& operator%=(const Counter& counter_) {
			IncrementIfNeeded(counter_);
			m_value %= counter_.m_value;
			return *this;
		}

		// µÝÔöµÝ¼õÔËËã·û
		Counter& operator++() {
			IncrementIfNeeded();
			++m_value;
			return *this;
		}

		int operator++(int) {
			IncrementIfNeeded();
			return m_value++;
		}

		Counter& operator--() {
			IncrementIfNeeded();
			--m_value;
			return *this;
		}

		int operator--(int) {
			IncrementIfNeeded();
			return m_value--;
		}

		// ËãÊõÔËËã·û
		int operator+(int value_) const {
			IncrementIfNeeded();
			return m_value + value_;
		}

		int operator-(int value_) const {
			IncrementIfNeeded();
			return m_value - value_;
		}

		int operator*(int value_) const {
			IncrementIfNeeded();
			return m_value * value_;
		}

		int operator/(int value_) const {
			IncrementIfNeeded();
			return m_value / value_;
		}

		int operator%(int value_) const {
			IncrementIfNeeded();
			return m_value % value_;
		}

		int operator+(const Counter& counter_) const {
			IncrementIfNeeded(counter_);
			return m_value + counter_.m_value;
		}

		int operator-(const Counter& counter_) const {
			IncrementIfNeeded(counter_);
			return m_value - counter_.m_value;
		}

		int operator*(const Counter& counter_) const {
			IncrementIfNeeded(counter_);
			return m_value * counter_.m_value;
		}

		int operator/(const Counter& counter_) const {
			IncrementIfNeeded(counter_);
			return m_value / counter_.m_value;
		}

		int operator%(const Counter& counter_) const {
			IncrementIfNeeded(counter_);
			return m_value % counter_.m_value;
		}

		// ÓÑÔªº¯ÊýÉùÃ÷
		friend bool operator>(int, const Counter&);
		friend bool operator<(int, const Counter&);
		friend bool operator==(int, const Counter&);
		friend bool operator>=(int, const Counter&);
		friend bool operator<=(int, const Counter&);
		friend bool operator!=(int, const Counter&);
		friend void swap(Counter&, Counter&);
		friend void swap(Counter&, int&);
		friend void swap(int&, Counter&);

	};

	bool operator>(int value_, const Counter& counter_) {
		if (counter_.m_notTemp) {
			++ActualStepNum;
		}
		return value_ > counter_.m_value;
	}

	bool operator<(int value_, const Counter& counter_) {
		if (counter_.m_notTemp) {
			++ActualStepNum;
		}
		return value_ < counter_.m_value;
	}

	bool operator==(int value_, const Counter& counter_) {
		if (counter_.m_notTemp) {
			++ActualStepNum;
		}
		return value_ == counter_.m_value;
	}

	bool operator>=(int value_, const Counter& counter_) {
		if (counter_.m_notTemp) {
			++ActualStepNum;
		}
		return value_ >= counter_.m_value;
	}

	bool operator<=(int value_, const Counter& counter_) {
		if (counter_.m_notTemp) {
			++ActualStepNum;
		}
		return value_ <= counter_.m_value;
	}

	bool operator!=(int value_, const Counter& counter_) {
		if (counter_.m_notTemp) {
			++ActualStepNum;
		}
		return value_ != counter_.m_value;
	}

	void swap(Counter& counter1_, Counter& counter2_) {
		if (counter1_.m_notTemp || counter2_.m_notTemp) {
			++ActualStepNum;
		}
		std::swap(counter1_.m_value, counter2_.m_value);
	}

	void swap(Counter& counter_, int& value_) {
		if (counter_.m_notTemp) {
			++ActualStepNum;
		}
		std::swap(counter_.m_value, value_);
	}

	void swap(int& value_, Counter& counter_) {
		if (counter_.m_notTemp) {
			++ActualStepNum;
		}
		std::swap(value_, counter_.m_value);
	}

}
