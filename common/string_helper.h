#pragma once
#include <string>

namespace pvesc {
	namespace common {
		template<size_t Len>
		constexpr inline size_t strlen(const char (&data)[Len]) noexcept {
			return Len - 1;
		}
		template<typename CharT, typename Traits, typename Alloc>
		inline size_t strlen(const std::basic_string<CharT, Traits, Alloc>& str) noexcept {
			return str.length();
		}
		template<typename T, typename TStart, typename TEnd>
		inline T find_section(const T& str, const TStart& start, const TEnd& end) {
			if(strlen(start) == 0)
				throw std::invalid_argument("Start can not be empty");
			auto pos = str.find(start);
			if(pos == T::npos)
				throw std::invalid_argument("Section not found");
			auto pend = strlen(end) == 0 ? str.size() : str.find(end, pos + strlen(start));
			if(pend == T::npos)
				throw std::invalid_argument("Section not found");
			return str.substr(pos + strlen(start), pend - pos - strlen(start));
		}
		template<typename T, typename TFind, typename TReplace>
		inline void replace(T& str, const TFind& find, const TReplace& replace) {
			auto flen = strlen(find);
			auto rlen = strlen(replace);
			if(flen == 0)
				throw std::invalid_argument("Find string can not be empty");
			auto pos = str.find(find);
			while(pos != T::npos) {
				str.replace(pos, flen, replace);
				pos = str.find(find, pos + rlen);
			}
		}
		template<typename T, typename TFind, typename TReplace>
		inline T replace_copy(const T& str, const TFind& find, const TReplace& replace) {
			T copy = str;
			common::replace(copy, find, replace);
			return copy;
		}
	}
}