#pragma once

#include <functional>

namespace lve {
	  // from: https://stackoverflow.com/a/57595105
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
	  std::hash<T> hash = std::hash<T>{};
	  std::size_t valueHashed = hash(v);

	  seed ^= valueHashed + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	  (hashCombine(seed, rest), ...);
	}

}  // namespace lve
