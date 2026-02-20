#pragma once
#include <utility>
#include <Windows.h>
#include <cstddef>

namespace NVisualSort {

	// 坐标类，表示二维坐标
	class Coordinate {

	public:

		using type_x = decltype(POINT::x);
		using type_y = decltype(POINT::y);

		type_x x = 0;
		type_y y = 0;

		constexpr Coordinate(type_x x_ = 0, type_y y_ = 0) noexcept : x(x_), y(y_) {}
		constexpr Coordinate(const Coordinate&) noexcept = default;
		constexpr Coordinate& operator=(const Coordinate&) noexcept = default;
		constexpr Coordinate(Coordinate&&) noexcept = default;
		constexpr Coordinate& operator=(Coordinate&&) noexcept = default;

		constexpr operator std::pair<type_x, type_y>() const noexcept {
			return { this->x,this->y };
		}

		constexpr Coordinate& operator=(const std::pair<type_x, type_y>& pair_) noexcept {
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