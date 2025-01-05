 #pragma once

#include <iostream> 

 #define FormPrint(pre,x,post) printf(pre); printf(Formatting::GetFormat(x),x); printf(post);
 
 namespace Formatting {
 
 	template<typename T>
 	static const char* GetFormat(T) {
 		return "%s";  // Default format specifier
 	}
 
 	// Specialization for int8_t
 	template<>
 	static const char* GetFormat<int8_t>(int8_t) {
 		return "%hhd";  // Format specifier for int8_t
 	}
 
 	// Specialization for uint8_t
 	template<>
 	static const char* GetFormat<uint8_t>(uint8_t) {
 		return "%hhu";  // Format specifier for uint8_t
 	}
 
 	// Specialization for int16_t
 	template<>
 	static const char* GetFormat<int16_t>(int16_t) {
 		return "%hd";  // Format specifier for int16_t
 	}
 
 	// Specialization for uint16_t
 	template<>
 	static const char* GetFormat<uint16_t>(uint16_t) {
 		return "%hu";  // Format specifier for uint16_t
 	}
 
 	// Specialization for int32_t
 	template<>
 	static const char* GetFormat<int32_t>(int32_t) {
 		return "%d";  // Format specifier for int32_t
 	}
 
 	// Specialization for uint32_t
 	template<>
 	static const char* GetFormat<uint32_t>(uint32_t) {
 		return "%u";  // Format specifier for uint32_t
 	}
 
 	// Specialization for int64_t
 	template<>
 	static const char* GetFormat<int64_t>(int64_t) {
 		return "%lld";  // Format specifier for int64_t
 	}
 
 	// Specialization for uint64_t
 	template<>
 	static const char* GetFormat<uint64_t>(uint64_t) {
 		return "%llu";  // Format specifier for uint64_t
 	}
 
 	// Specialization for float
 	template<>
 	static const char* GetFormat<float>(float) {
 		return "%f";  // Format specifier for float
 	}
 
 	// Specialization for double
 	template<>
 	static const char* GetFormat<double>(double) {
 		return "%.15f";  // Format specifier for double
 	}
 
 	// Specialization for bool
 	template<>
 	static const char* GetFormat<bool>(bool) {
 		return "%d";  // Format specifier for bool
 	}
 
 }
