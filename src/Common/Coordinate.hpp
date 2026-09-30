#pragma once
#include <utility>
#include <Windows.h>
#include <cstddef>

namespace NVisualSort {

	// 坐标类，表示二维坐标
	class Coordinate {

	public:

		using TypeX = decltype(POINT::x);
		using TypeY = decltype(POINT::y);

		TypeX x = 0;
		TypeY y = 0;

		constexpr Coordinate(TypeX x_ = 0, TypeY y_ = 0) noexcept : x(x_), y(y_) {}
		constexpr Coordinate(const Coordinate&) noexcept = default;
		constexpr Coordinate& operator=(const Coordinate&) noexcept = default;
		constexpr Coordinate(Coordinate&&) noexcept = default;
		constexpr Coordinate& operator=(Coordinate&&) noexcept = default;

		constexpr operator std::pair<TypeX, TypeY>() const noexcept {
			return { this->x,this->y };
		}

		constexpr Coordinate& operator=(const std::pair<TypeX, TypeY>& pair_) noexcept {
			this->x = pair_.first;
			this->y = pair_.second;
			return *this;
		}

		constexpr bool operator==(const Coordinate& other) const noexcept {
			return this->x == other.x && this->y == other.y;
		}
		constexpr bool operator!=(const Coordinate& other) const noexcept {
			return this->x != other.x || this->y != other.y;
		}

		constexpr Coordinate operator+(const Coordinate& other) const noexcept {
			return Coordinate(this->x + other.x, this->y + other.y);
		}
		constexpr Coordinate operator-(const Coordinate& other) const noexcept {
			return Coordinate(this->x - other.x, this->y - other.y);
		}

		constexpr operator POINT() const noexcept {
			return { this->x, this->y };
		}

		POINT* AsPointPtr() noexcept {
			return reinterpret_cast<POINT*>(this);
		}

		const POINT* AsPointPtr() const noexcept {
			return reinterpret_cast<const POINT*>(this);
		}

	};

	static_assert(sizeof(Coordinate) == sizeof(POINT),
		"Coordinate 的大小必须与 POINT 的大小相同");
	static_assert(offsetof(Coordinate, x) == offsetof(POINT, x),
		"Coordinate::x 偏移量必须与 POINT::x 匹配");
	static_assert(offsetof(Coordinate, y) == offsetof(POINT, y),
		"Coordinate::y 偏移量必须与 POINT::y 匹配");

}