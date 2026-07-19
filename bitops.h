#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#endif
//_MSC_VER is a built-in preprocessor macro in the Microsoft C/C++ compiler (MSVC) that evaluates to an integer representing the compiler's version number

#include <cstdint>
#include <cstring>

//msb and lsb perform differently based on compiler due to hardware and optimisations made by compielr

inline std::uint8_t lsbIndex(std::uint64_t x)
{
#if defined(_MSC_VER)
	unsigned long index;
	_BitScanForward64(&index, x);
	return index;
#elif defined(__GNUC__) || defined(__CLANG__) //clang gcc version
	return __builtin_ctzll(x);
#else //fallback calculation of msb via the debrujin whihc has randomly repeating sequence of 6 to access the lsb at any point
	static const int index[64] =
	{
		 0,  1, 48,  2, 57, 49, 28,  3,
		61, 58, 50, 42, 38, 29, 17,  4,
		62, 55, 59, 36, 53, 51, 43, 22,
		45, 39, 33, 30, 24, 18, 12,  5, 
		63, 47, 56, 27, 60, 41, 37, 16,
		54, 35, 52, 21, 44, 32, 23, 11,
		46, 26, 40, 15, 34, 20, 31, 10,
		25, 14, 19,  9, 13,  8,  7,  6,
	};
	static const std::uint64_t debrujin = 0x03f79d7b4cb0a89;
	return index[((x & -(std::int64_t)x) * debrujin) >> 58];
#endif
}

inline std::uint8_t msbIndex(std::uint64_t x)
{
#if defined(_MSC_VER)
	unsigned long index;
	_BitScanReverse64(&index, x);
	return index;
#elif defined(__GNUC__) || defined(__CLANG__) //clang gcc version
	return 63 - __builtin_ctzll(x);
#else
	double d = (double)x;
	std::uint64_t bits;
	memcpy(&bits, &d, sizeof(bits));
	return ((bits >> 52) & 0x7ff) - 1023; //exposes mantissa to find the MSB (shift by 52)	
#endif

}
