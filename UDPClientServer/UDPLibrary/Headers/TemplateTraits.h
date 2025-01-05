#pragma once

#include <iostream>
#include <vector>
#include <type_traits>
#include <cassert>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Ws2_32.lib" )


// check for std_vectors 
template <typename T>
struct is_std_vector : public std::false_type {
	using type = T;
};

template <typename T>
struct is_std_vector<std::vector<T>> : public std::true_type {
	using type = T;
};

template <typename T>
static constexpr bool is_std_vector_v = is_std_vector<T>::value;

template <typename T>
using vector_type = typename is_std_vector<T>::type;

//// check for vectors 
//template <typename T>
//struct pure_vector {
//	static constexpr bool value = is_std_vector<std::remove_cv_t<std::remove_reference_t<T>>>::value;
//	using type = typename is_std_vector<std::remove_cv_t<std::remove_reference_t<T>>>::type;
//};
//
//template <typename T>
//using pure_vector_t = typename pure_vector<T>::type;

// remove const from pointer types

template <typename T>
struct remove_const_ptr : public std::false_type {
	using type = T;
};

template <typename T>
struct remove_const_ptr<const T*> : public std::true_type {
	using type = T*;
};

template <typename T>
using remove_const_ptr_t = typename remove_const_ptr<T>::type;

template <typename T>
static constexpr bool remove_const_ptr_v = remove_const_ptr<T>::value;

// check if char pointer but I don't think I use this one.
// I created one to check for chars in general.
template <typename T, typename Enable = void>
struct is_char_ptr : public std::false_type {};

template <typename T>
struct is_char_ptr<T, std::enable_if_t<std::is_same_v<remove_const_ptr_t<T>, char*>, void>> : public std::true_type {
	using type = char*;
};

template<typename T>
using is_char_ptr_t = typename is_char_ptr<T>::type;

template<typename T>
static constexpr bool is_char_ptr_v = is_char_ptr<T>::value;


// check for regular chars

template <typename T, typename Enable = void>
struct is_char : public std::false_type {};

template <typename T>
struct is_char<T, std::enable_if_t<std::is_same_v<std::remove_all_extents_t<std::remove_pointer_t<T>>, char>, void>> : public std::true_type {
	using type = char;
};

template<typename T>
using is_char_t = typename is_char<T>::type;

template<typename T>
static constexpr bool is_char_v = is_char<T>::value;


// check for strings

template <typename T, typename Enable = void>
struct is_std_string : public std::false_type {};

template <typename T>
struct is_std_string<T, std::enable_if_t<std::is_same_v<T, std::string>, void>> : public std::true_type {
	using type = T;
};

template<typename T>
using is_std_string_t = typename is_std_string<T>::type;

template<typename T>
static constexpr bool is_std_string_v = is_std_string<T>::value;


// check for regular objects no pointers no arrays kinda deal
template <typename T, typename Enable = void>
struct is_regular : std::false_type {};

template <typename T>
struct is_regular<T, std::enable_if_t< std::is_integral_v<T> || std::is_floating_point_v<T>,void>> : std::true_type {
using type = typename T;
};

template <typename T>
static constexpr bool is_regular_v = is_regular<T>::value;

template <typename T>
using is_regular_t = typename is_regular<T>::type;

// convert types to uint variation, this is for bit-reversing later on//

template <typename T, typename Enable = void>
struct uint_convert : std::false_type {};

template <typename T>
struct uint_convert<T, std::enable_if_t<sizeof(std::remove_cv_t<T>) == 1, void>> : std::true_type{
	using type = uint8_t;
	static void HostToNet(type& Value){}
	static void NetToHost(type& Value){}
};

template <typename T>
struct uint_convert<T,std::enable_if_t<sizeof(std::remove_cv_t<T>) == 2, void>> : std::true_type{
	using type = uint16_t;
	static void HostToNet(type& Value) { Value = htons(Value);}
	static void NetToHost(type& Value) { Value = ntohs(Value); }
};

template <typename T>
struct uint_convert<T, std::enable_if_t<sizeof(std::remove_cv_t<T>) == 4, void>> : std::true_type{
	using type = uint32_t;
	static void HostToNet(type& Value) { Value = htonl(Value); }
	static void NetToHost(type& Value) { Value = ntohl(Value); }
};

template <typename T>
struct uint_convert<T, std::enable_if_t<sizeof(std::remove_cv_t<T>) == 8, void>> : std::true_type{
	using type = uint64_t;
	static void HostToNet(type& Value) { Value = htonll(Value);}
	static void NetToHost(type& Value) { Value = ntohll(Value);}
};

template <typename T>
using uint_convert_t = typename uint_convert<T>::type;

template <typename T>
static constexpr bool uint_convert_v = uint_convert<T>::value;

