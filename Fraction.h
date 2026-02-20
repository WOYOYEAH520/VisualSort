#pragma once
#include "WideError.h"
#include <cstdlib>
#include <limits>
#include <utility>
#include <Windows.h>
#include <numeric>
#include <concepts>
#include <type_traits>

namespace NVisualSort {

	class Fraction {

	private:

		long long m_numerator = 0;
		long long m_denominator = 1;

		// 约分函数
		constexpr void Reduce() noexcept {
			if (this->m_numerator == 0) {
				this->m_denominator = 1;
				return;
			}

			if (this->m_denominator < 0) {
				this->m_numerator = -this->m_numerator;
				this->m_denominator = -this->m_denominator;
			}

			long long divisor = std::gcd(this->m_numerator, this->m_denominator);
			if (divisor != 0) {
				this->m_numerator /= divisor;
				this->m_denominator /= divisor;
			}
		}

	public:

		template<std::integral T>
		constexpr Fraction(T num_ = 0) noexcept :
			m_numerator(static_cast<long long>(num_)),
			m_denominator(1) {
		}

		template<std::integral T1, std::integral T2>
		constexpr Fraction(T1 num_, T2 denom_) {
			if (denom_ == 0) {
				throw WideError(L"分母不能为0");
			}
			this->m_numerator = static_cast<long long>(num_);
			this->m_denominator = static_cast<long long>(denom_);
			this->Reduce();
		}

		// 从浮点数构造（近似转换）
		template<std::floating_point T>
		explicit Fraction(T value_) {
			if (!std::isfinite(value_)) {
				throw WideError(L"无法将非有限值转换为分数");
			}

			constexpr long long MAX_DEN = 1'000'000;
			constexpr double EPS = 1e-12;

			if (std::abs(value_) < EPS) {
				this->m_numerator = 0;
				this->m_denominator = 1;
				return;
			}

			const bool negative = value_ < 0;
			double x = std::abs(value_);

			long long h0 = 0, k0 = 1; // 前一收敛分数
			long long h1 = 1, k1 = 0; // 当前收敛分数
			long long h2 = 0, k2 = 0; // 下一收敛分数

			while (true) {
				long long a = static_cast<long long>(x);
				h2 = a * h1 + h0;
				k2 = a * k1 + k0;

				// 分母超限：选择上一个收敛（k1）并退出
				if (k2 > MAX_DEN) {
					// 此处直接采用上一个收敛，误差通常已极小
					h2 = h1;
					k2 = k1;
					break;
				}

				// 更新收敛序列
				h0 = h1; k0 = k1;
				h1 = h2; k1 = k2;

				// 剩余部分 = x - a
				x -= static_cast<double>(a);
				if (x < EPS) break;   // 精确匹配
				x = 1.0 / x;          // 倒置
			}

			this->m_numerator = h2;
			this->m_denominator = k2;

			if (negative) {
				this->m_numerator = -this->m_numerator;
			}
			this->Reduce();
		}

		Fraction(const std::pair<long long, long long>& pair_)
			: Fraction(pair_.first, pair_.second) {}

		constexpr Fraction(const Fraction&) noexcept = default;
		constexpr Fraction& operator=(const Fraction&) noexcept = default;
		constexpr Fraction(Fraction&&) noexcept = default;
		constexpr Fraction& operator=(Fraction&&) noexcept = default;

		constexpr long long GetNumerator() const noexcept {
			return this->m_numerator;
		}
		template<std::integral T>
		Fraction& SetNumerator(T num_) noexcept {
			this->m_numerator = static_cast<long long>(num_);
			this->Reduce();
			return *this;
		}

		constexpr long long GetDenominator() const noexcept {
			return this->m_denominator;
		}
		template<std::integral T>
		Fraction& SetDenominator(T denom_) {
			if (denom_ == 0) {
				throw WideError(L"分母不能为0");
			}
			this->m_denominator = static_cast<long long>(denom_);
			this->Reduce();
			return *this;
		}

		// 转换为浮点数
		template<std::floating_point T>
		constexpr operator T() const noexcept {
			return static_cast<T>(m_numerator) / static_cast<T>(m_denominator);
		}
		
		// 转换为整数（取整）
		template<std::integral T>
		constexpr operator T() const noexcept {
			return static_cast<T>(m_numerator / m_denominator);
		}

		// 转换为 std::pair 的显式函数
		constexpr std::pair<long long, long long> ToPair() const noexcept {
			return std::make_pair(this->m_numerator, this->m_denominator);
		}

		// 一元运算符
		Fraction operator+() const noexcept {
			return *this;
		}

		Fraction operator-() const noexcept {
			return Fraction(-this->m_numerator, this->m_denominator);
		}

		// 算术运算符（成员函数形式）
		Fraction& operator+=(const Fraction& other_) noexcept {
			long long lcm = std::lcm(this->m_denominator, other_.m_denominator);
			this->m_numerator = this->m_numerator * (lcm / this->m_denominator) +
				other_.m_numerator * (lcm / other_.m_denominator);
			this->m_denominator = lcm;
			this->Reduce();
			return *this;
		}

		Fraction& operator-=(const Fraction& other_) noexcept {
			long long lcm = std::lcm(this->m_denominator, other_.m_denominator);
			this->m_numerator = this->m_numerator * (lcm / this->m_denominator) -
				other_.m_numerator * (lcm / other_.m_denominator);
			this->m_denominator = lcm;
			this->Reduce();
			return *this;
		}

		Fraction& operator*=(const Fraction& other_) noexcept {
			this->m_numerator *= other_.m_numerator;
			this->m_denominator *= other_.m_denominator;
			this->Reduce();
			return *this;
		}

		Fraction& operator/=(const Fraction& other_) {
			if (other_.m_numerator == 0) {
				throw WideError(L"除以0");
			}
			this->m_numerator *= other_.m_denominator;
			this->m_denominator *= other_.m_numerator;
			this->Reduce();
			return *this;
		}

		constexpr Fraction Reciprocal() const {
			return Fraction(this->m_denominator, this->m_numerator);
		}

		// 比较运算符
		constexpr bool operator==(const Fraction& other_) const noexcept {
			return this->m_numerator == other_.m_numerator &&
				this->m_denominator == other_.m_denominator;
		}

		constexpr bool operator!=(const Fraction& other_) const noexcept {
			return !(*this == other_);
		}

		constexpr bool operator<(const Fraction& other_) const noexcept {
			return this->m_numerator * other_.m_denominator <
				other_.m_numerator * this->m_denominator;
		}

		constexpr bool operator<=(const Fraction& other_) const noexcept {
			return this->m_numerator * other_.m_denominator <=
				other_.m_numerator * this->m_denominator;
		}

		constexpr bool operator>(const Fraction& other_) const noexcept {
			return !(*this <= other_);
		}

		constexpr bool operator>=(const Fraction& other_) const noexcept {
			return !(*this < other_);
		}

		// 与浮点数的比较
		bool operator==(double value_) const noexcept {
			return std::abs(static_cast<double>(*this) - value_) <
				std::numeric_limits<double>::epsilon();
		}

		bool operator!=(double value_) const noexcept {
			return !(*this == value_);
		}

		constexpr bool operator<(double value_) const noexcept {
			return static_cast<double>(*this) < value_;
		}

		constexpr bool operator<=(double value_) const noexcept {
			return static_cast<double>(*this) <= value_;
		}

		constexpr bool operator>(double value_) const noexcept {
			return static_cast<double>(*this) > value_;
		}

		constexpr bool operator>=(double value_) const noexcept {
			return static_cast<double>(*this) >= value_;
		}

		template<std::integral T>
		constexpr bool operator==(T value_) const noexcept {
			return this->m_numerator == this->m_denominator * value_;
		}

		template<std::integral T>
		constexpr bool operator!=(T value_) const noexcept {
			return this->m_numerator != this->m_denominator * value_;
		}

		template<std::integral T>
		constexpr bool operator>(T value_) const noexcept {
			return this->m_numerator > this->m_denominator * value_;
		}

		template<std::integral T>
		constexpr bool operator>=(T value_) const noexcept {
			return this->m_numerator >= this->m_denominator * value_;
		}

		template<std::integral T>
		constexpr bool operator<(T value_) const noexcept {
			return this->m_numerator < this->m_denominator * value_;
		}

		template<std::integral T>
		constexpr bool operator<=(T value_) const noexcept {
			return this->m_numerator <= this->m_denominator * value_;
		}

	};

	// 非成员算术运算符
	inline Fraction operator+(const Fraction& lhs_, const Fraction& rhs_) noexcept {
		Fraction result = lhs_;
		result += rhs_;
		return result;
	}

	inline Fraction operator-(const Fraction& lhs_, const Fraction& rhs_) noexcept {
		Fraction result = lhs_;
		result -= rhs_;
		return result;
	}

	inline Fraction operator*(const Fraction& lhs_, const Fraction& rhs_) noexcept {
		Fraction result = lhs_;
		result *= rhs_;
		return result;
	}

	inline Fraction operator/(const Fraction& lhs_, const Fraction& rhs_) {
		Fraction result = lhs_;
		result /= rhs_;
		return result;
	}

	template<typename T> concept Arithmetic = std::integral<T> || std::floating_point<T>;

	template<Arithmetic T>
	inline Fraction operator+(const Fraction& lhs_, T rhs_) noexcept(noexcept(Fraction(rhs_))) {
		return lhs_ + Fraction(rhs_);
	}

	template <Arithmetic T>
	inline Fraction operator+(T lhs_, const Fraction& rhs_) noexcept(noexcept(Fraction(lhs_))) {
		return Fraction(lhs_) + rhs_;
	}

	template<Arithmetic T>
	inline Fraction operator-(const Fraction& lhs_, T rhs_) noexcept(noexcept(Fraction(rhs_))) {
		return lhs_ - Fraction(rhs_);
	}

	template <Arithmetic T>
	inline Fraction operator-(T lhs_, const Fraction& rhs_) noexcept(noexcept(Fraction(lhs_))) {
		return Fraction(lhs_) - rhs_;
	}

	template<Arithmetic T>
	inline Fraction operator*(const Fraction& lhs_, T rhs_) noexcept(noexcept(Fraction(rhs_))) {
		return lhs_ * Fraction(rhs_);
	}

	template <Arithmetic T>
	inline Fraction operator*(T lhs_, const Fraction& rhs_) noexcept(noexcept(Fraction(lhs_))) {
		return Fraction(lhs_) * rhs_;
	}

	template<Arithmetic T>
	inline Fraction operator/(const Fraction& lhs_, T rhs_) {
		return lhs_ / Fraction(rhs_);
	}

	template <Arithmetic T>
	inline Fraction operator/(T lhs_, const Fraction& rhs_) {
		return Fraction(lhs_) / rhs_;
	}

	/// <summary>
	/// 根据基矩形和四个边对应的分数计算并返回一个新的矩形。
	/// </summary>
	/// <param name="base_rect_">参考的基矩形（RECT），用于按其位置和尺寸计算新矩形的坐标。</param>
	/// <param name="left_fraction_">表示左边位置的分数（Fraction），按基矩形宽度和分数计算 newRect.left。</param>
	/// <param name="top_fraction_">表示上边位置的分数（Fraction），按基矩形高度和分数计算 newRect.top。</param>
	/// <param name="right_fraction_">表示右边位置的分数（Fraction），按基矩形宽度和分数计算 newRect.right。</param>
	/// <param name="bottom_fraction_">表示下边位置的分数（Fraction），按基矩形高度和分数计算 newRect.bottom。</param>
	/// <returns>按给定分数从基矩形计算得到的新 RECT。返回值在需要时会交换坐标以确保矩形有效（确保 left ≤ right 且 top ≤ bottom）。</returns>
	constexpr RECT ComputeRect(const RECT& base_rect_,
		const Fraction& left_fraction_,
		const Fraction& top_fraction_,
		const Fraction& right_fraction_,
		const Fraction& bottom_fraction_) noexcept
	{
		RECT newRect = {};

		newRect.left = static_cast<decltype(newRect.left)>((base_rect_.left * left_fraction_.GetDenominator() +
			left_fraction_.GetNumerator() * (static_cast<long long>(base_rect_.right) - base_rect_.left)) /
			left_fraction_.GetDenominator());

		newRect.top = static_cast<decltype(newRect.top)>((base_rect_.top * top_fraction_.GetDenominator() +
			top_fraction_.GetNumerator() * (static_cast<long long>(base_rect_.bottom) - base_rect_.top)) /
			top_fraction_.GetDenominator());

		newRect.right = static_cast<decltype(newRect.right)>((base_rect_.left * right_fraction_.GetDenominator() +
			right_fraction_.GetNumerator() * (static_cast<long long>(base_rect_.right) - base_rect_.left)) /
			right_fraction_.GetDenominator());

		newRect.bottom = static_cast<decltype(newRect.bottom)>((base_rect_.top * bottom_fraction_.GetDenominator() +
			bottom_fraction_.GetNumerator() * (static_cast<long long>(base_rect_.bottom) - base_rect_.top)) /
			bottom_fraction_.GetDenominator());

		// 确保矩形有效
		if (newRect.left > newRect.right) std::swap(newRect.left, newRect.right);
		if (newRect.top > newRect.bottom) std::swap(newRect.top, newRect.bottom);

		return newRect;
	}

}

static_assert(std::is_nothrow_copy_constructible_v<NVisualSort::Fraction>);