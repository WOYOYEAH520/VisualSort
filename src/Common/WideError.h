#pragma once
#include <string>
#include <string_view>
#include <concepts>
#include <type_traits>
#include <utility>

namespace NVisualSort {

	class WideError {

	private:

		std::wstring m_wideMessage;

	public:

		WideError() = default;

		template<typename T>
			requires (std::constructible_from<std::wstring, T> &&
		!std::same_as<std::remove_cvref_t<T>, WideError>)
			explicit WideError(T&& msg)
			noexcept(std::is_nothrow_constructible_v<std::wstring, T>)
			: m_wideMessage(std::forward<T>(msg)) {}

		WideError(WideError&&) noexcept = default;
		WideError(const WideError&) = default;
		WideError& operator=(WideError&&) noexcept = default;
		WideError& operator=(const WideError&) = default;

		const std::wstring& What() const& noexcept { return this->m_wideMessage; }
		std::wstring What() && noexcept { return std::move(this->m_wideMessage); }
		std::wstring_view WhatView() const noexcept { return this->m_wideMessage; }

	};

}