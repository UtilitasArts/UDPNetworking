#pragma once

#include <iostream>
#include <vector>
#include <cassert>
#include "FormatSpecifiers.h" 
#include "TemplateTraits.h"

inline constexpr bool isLittleEndian() {
	int n = 1;
	if (*(char*)&n == 1) { return true; }
	return false;
}

class BytePack {
public:
	inline BytePack(size_t MaxSize = 0, size_t ElemCount = 0) :
	ArraySize(MaxSize),
	MaxIndex(0),
	ElementCount(0),
	bCRCisValid(false) {
		ByteArray.reserve(MaxSize);
		ByteIndexArray.reserve(ElemCount);
		ByteSizesArray.reserve(ElemCount);
		CreateCRCTable();
	}

	inline void Clear(size_t MaxSize = 0, size_t ElemCount = 0) {
		ArraySize	 = MaxSize;
		ElementCount = ElemCount;
		MaxIndex	 = 0;
		bCRCisValid  = false;

		ByteArray.clear();
		ByteIndexArray.clear();
		ByteSizesArray.clear();

		ByteArray.reserve(MaxSize);
		ByteIndexArray.reserve(ElemCount);
		ByteSizesArray.reserve(ElemCount);
	}

	// Enum
	template <typename T>
	std::enable_if_t<std::is_enum_v<T>, size_t>
		AddBytes(T Element) {
		using type = typename std::underlying_type<T>::type;
		size_t Size = sizeof(type);
		type value = static_cast<type>(Element);
		AddBytes(value);
		return Size;
	}

	// SINGLE
	template <typename T>
	std::enable_if_t<is_regular_v<T>, size_t>
		AddBytes(T& Element) {
		size_t Size = sizeof(T);
		if (isLittleEndian()) {
			using type = uint_convert_t<T>;
			type Converted = *reinterpret_cast<type*>(&Element);
			uint_convert<T>::HostToNet(Converted);
			type* Ptr = &Converted;
			AddToArray(Ptr, Size);
		}
		else {
			T* Ptr = &Element;
			AddToArray(Ptr, Size);
		}
		return Size;
	}
	
// Bools 
// 	template <typename T>
// 	std::enable_if_t<std::is_same_v<T, bool>, size_t>
// 		AddBytes(T Element) {
// 		size_t Size = sizeof(T);
// 		T* Ptr = &Element;
// 		AddToArray(Ptr, Size);	
// 		return Size;
// 	}

	//
	// SINGLE PTR
	template <typename T>
	std::enable_if_t<std::is_pointer_v<T> && !is_char_ptr_v<T>, size_t>
		AddBytes(T& Element) {
		size_t Size = AddBytes(*Element);
		return Size;
	}

	// Arrays
	// 
	// CONST ARRAY
	template <typename T>
	std::enable_if_t<std::is_array_v<T> && !std::is_same_v<std::remove_all_extents_t<T>, char>, size_t>
		AddBytes(T& Element) {
		using type = typename std::remove_all_extents_t<T>;
		size_t ElementCount = std::size(Element);
		size_t Size = ElementCount * sizeof(Element[0]);
		type* Ptr = &Element[0];

		HostToNetArray(Ptr, ElementCount, Size);

		return Size;
	}
	//
	// VECTOR
	template <typename T>
	std::enable_if_t<is_std_vector<T>::value, size_t>
		AddBytes(T& Element) {
		using type = typename is_std_vector<T>::type;
		size_t ElementCount = Element.size();
		size_t Size = ElementCount * sizeof(Element[0]);

		type* Ptr = &Element[0];
		HostToNetArray(Ptr, ElementCount, Size);

		return Size;
	}

	//
	//PTR_ARRAY
	template <typename T>
	std::enable_if_t<std::is_pointer_v<T>, size_t>
		AddBytes(T& Element, size_t ElementCount) {
		size_t Size = ElementCount * sizeof(Element[0]);

		HostToNetArray(Element, ElementCount, Size);
		return Size;
	}

	// TEXTS
	// 
	// CHAR_ARRAY
	template <typename T>
	std::enable_if_t<std::is_array_v<T>&& std::is_same_v<std::remove_all_extents_t<T>, char>, size_t>
		AddBytes(T& Element) {
		using type = typename std::remove_all_extents_t<T>;
		size_t Size = strlen(Element);
		type* Ptr = &Element[0];

		AddToArray(Ptr, Size);
		return Size;
	}
	//
	// CHAR_PTR
	template <typename T>
	std::enable_if_t<is_char_ptr_v<T>, size_t>
		AddBytes(T& Element) {
		size_t Size = strlen(Element);

		AddToArray(Element, Size);
		return Size;
	}
	//
	// STRING
	template <typename T>
	std::enable_if_t<is_std_string_v<T>, size_t>
		AddBytes(T& Element) {
		size_t Size = Element.size();
		char* Ptr = Element.data();
		AddToArray(Ptr, Size);
		return Size;
	}

private:

	template<typename T>
	void HostToNetArray(T* Ptr, size_t ElementCount, size_t Size) {
		if (is_regular_v<T> && isLittleEndian()) {
			using ConvertedType = uint_convert_t<T>;
			ConvertedType* ConvertedArray = new ConvertedType[ElementCount];
			for (size_t i = 0; i < ElementCount; i++) {
				ConvertedType Converted = *reinterpret_cast<ConvertedType*>(&Ptr[i]);
				uint_convert<T>::HostToNet(Converted);
				ConvertedArray[i] = Converted;
			}
			AddToArray(ConvertedArray, Size);
			delete[] ConvertedArray;
		}
		else {
			AddToArray(Ptr, Size);
		}
	}

	template <typename T>
	void AddToArray(T& Element, size_t Size) {
 		ByteArray.push_back(static_cast<uint8_t>(Size));
 		AddIndex(sizeof(uint8_t));
		ByteArray.resize(MaxIndex+ Size);
 		memcpy(ByteArray.data() + MaxIndex, Element, Size);
 		ByteIndexArray.push_back(static_cast<uint8_t>(MaxIndex));
 		ByteSizesArray.push_back(static_cast<uint8_t>(Size));
 		AddIndex(Size);
 		ElementCount++;
	}
 public:

	 inline static void CreateCRCTable() {

		 if (CRCTable[0] > 0) { printf("Already Defined"); return; }

		 uint32_t Poly = 0x04C11DB7;
		 for (uint32_t i = 0; i < 256; i++) {
			 uint32_t crc = i << 24;
			 for (size_t j = 0; j < 8; j++) {
				 if (crc & 0x80000000) { crc = (crc << 1) ^ Poly; }
				 else { crc <<= 1; }
			 }
			 CRCTable[i] = crc;
		 }
	 }
	 inline void AddCRC() {
		 if (!bCRCisValid) {
			 uint32_t crc = 0;
			 AddBytes(crc);
			 for (size_t i = 0; i < MaxIndex; i++) {
				 byte t = (crc >> 24) & 0xFF;
				 crc = (crc << 8) | ByteArray.data()[i];
				 crc ^= CRCTable[t];
			 }
			 ReplaceCRC(crc);
			 bCRCisValid = true;
		 }
	 }

	 inline void ReplaceCRC(uint32_t crc) {
		 uint_convert<uint32_t>::HostToNet(crc);
		 *reinterpret_cast<uint32_t*>(ByteArray.data() + (MaxIndex - sizeof(uint32_t))) = crc;
	 }

	 inline void PrintBytes() {
		 size_t BytesToPrint = 0;
		 size_t DataCount = 0;
		 for (size_t i = 0; i < MaxIndex; i++) {
			 if (BytesToPrint == 0) {
				 BytesToPrint = ByteArray[i];
				 printf("|Data#%02zd[%02db] = ", DataCount, ByteArray[i]);
				 DataCount++;
			 }
			 else {
				 printf("%02X ", ByteArray[i]);
				 BytesToPrint--;
				 if (BytesToPrint == 0) { printf("\n"); }
			 }
		 } printf("- Total ByteCount = %zd - \n\n", MaxIndex);
	 }

	 inline		  uint8_t*  GetByteArray() { ByteArray.shrink_to_fit(); return ByteArray.data(); }
	 inline const size_t    GetArraySize() { return ByteArray.size(); }
	 inline const size_t	GetElementCount()  { return ElementCount;}

	 inline void SetByteArray(const uint8_t* OtherByteArray, const size_t Size, bool bPrint = false) {
		Clear(Size);
		if (CRCValid(OtherByteArray, Size, bPrint)) {
			ByteArray.clear();
			ByteArray.resize(Size);
			std::memcpy(ByteArray.data(), OtherByteArray, Size);
			MaxIndex = Size;
			IndexBytes(false);
			if(bPrint){	PrintBytes(); }
		}
 	 }
	 inline void SetByteArray(const char* OtherByteArrayAsChar, const size_t Size, bool bPrint = false) {
		SetByteArray(reinterpret_cast<const uint8_t*>(ByteArray.data()), Size, bPrint);
	 }

	 inline const char* GetByteArrayAsChar() { ByteArray.shrink_to_fit(); return reinterpret_cast<const char*>(ByteArray.data()); }

	 inline bool CRCValid(const uint8_t* ArrayToCheck, const size_t Size, bool bPrint) {
 		 uint32_t crc = 0;
 		 for (size_t i = 0; i < Size; i++) {
 			 byte t = (crc >> 24) & 0xFF;
 			 crc = (crc << 8) | ArrayToCheck[i];
 			 crc ^= CRCTable[t];
 		 }
 
 		 if (crc == 0) {
 			 if (bPrint) { printf("\n-CRC is valid-\n"); }
 			 bCRCisValid = true;
 			 return true;
 		 }
 		 else {
 			 if (bPrint) { printf("\n-CRC is invalid-\n"); }
 			 bCRCisValid = false;
 			 return false;
 		 }
 	 }

	 inline bool GetCRCValid() {
		return bCRCisValid;
	 }

	 inline void IndexBytes(bool bPrint = false) {
		 ByteIndexArray.clear();
		 ByteSizesArray.clear();

 		 size_t element_index = 0;
 		 size_t array_index   = 0;
 		 size_t element_count = 0; 	
 		 size_t i = 0;
		 uint8_t ByteCount;
 
 		 while (array_index < MaxIndex) {
 			 ByteCount = ByteArray.data()[array_index]; 
 			 array_index++;
 			 ElementCount = i;
 			 ByteIndexArray.push_back(static_cast<uint8_t>(array_index));
 			 ByteSizesArray.push_back(static_cast<uint8_t>(ByteCount));
 			 array_index += (ByteCount);
 
 			 if (bPrint) { printf("%03d, %02d \n", ByteIndexArray.data()[i], ByteSizesArray.data()[i]); }
 			 i++;
 		 } 		
 	 }

	 template <typename T>
	 std::enable_if_t<std::is_enum_v<T>, void>
		 ReturnBytes(T& Element, size_t ElementIndex, bool bPrint = false) {
		 uint8_t ByteIndex = ByteIndexArray.data()[ElementIndex];
		 Element = *reinterpret_cast<T*>(ByteArray.data() + ByteIndex);		

		 if (bPrint) { FormPrint(" ",static_cast<uint8_t>(Element), "") }
	 }

 	template <typename T>
 	std::enable_if_t<is_regular_v<T>, void>
 		ReturnBytes(T& Element, size_t ElementIndex, bool bPrint = false) {
 		uint8_t ByteIndex = ByteIndexArray.data()[ElementIndex];
 		uint8_t ByteSize  = ByteSizesArray.data()[ElementIndex];
 		size_t ElemCount  = ByteSize / sizeof(T);
 		assert(ElemCount <= 1 && "To many elements for this type");
 
 		if (isLittleEndian()) {
 			uint_convert_t<T> ConvertT;
 			memcpy(&ConvertT, ByteArray.data() + ByteIndex, ByteSize);
 			uint_convert<T>::NetToHost(ConvertT);
 			Element = *reinterpret_cast<T*>(&ConvertT);
 		}
 		else{
 			Element = *reinterpret_cast<T*>(ByteArray.data() + ByteIndex);
 		}
 
 		if (bPrint) { FormPrint(" ", Element, "") }
 	}
 	
 	template <typename T>
 	std::enable_if_t<std::is_pointer_v<T>, void>
 		ReturnBytes(T& Element, size_t ElementIndex, bool bPrint = false) {
 		using type = std::remove_pointer_t<T>;
 		uint8_t ByteIndex = ByteIndexArray.data()[ElementIndex];
 		uint8_t ByteSize  = ByteSizesArray.data()[ElementIndex];
 		size_t ElemCount  = ByteSize / sizeof(type);
 
 		if (ElemCount > 1) { Element = new type;  }
 		else { Element = new type[ElemCount + 1]; }
 		ReturnArrayTypes(ElemCount, ByteIndex, Element, bPrint, ByteSize);
 	}
 
 	template <typename T>
 	std::enable_if_t<std::is_array_v<T>, void>
 		ReturnBytes(T& Element, size_t ElementIndex = 0, bool bPrint = false) {
 		using type = typename std::remove_all_extents_t<T>;
 		uint8_t ByteIndex = ByteIndexArray.data()[ElementIndex];
 		uint8_t ByteSize  = ByteSizesArray.data()[ElementIndex];
 		size_t ElemCount  = std::size(Element);		
 		ReturnArrayTypes(ElemCount, ByteIndex, Element, bPrint, ByteSize);	
 	}
 
  	template <typename T>
  	std::enable_if_t<is_std_vector_v<T>, void>
  		ReturnBytes(T& Element, size_t ElementIndex = 0, bool bPrint = false) {
  		using type = vector_type<T>;
 		uint8_t ByteIndex = ByteIndexArray.data()[ElementIndex];
 		uint8_t ByteSize  = ByteSizesArray.data()[ElementIndex];
 		size_t ElemCount  = ByteSize / sizeof(type);
 		Element.clear();Element.resize(ElemCount);
 		type* ElementPtr = Element.data();
 		ReturnArrayTypes(ElemCount, ByteIndex, ElementPtr, bPrint, ByteSize);
  	}
 
 	template <typename T>
 	std::enable_if_t<is_std_string_v<T>, void>
 		ReturnBytes(T& Element, size_t ElementIndex = 0, bool bPrint = false) {
 		using type = char;
 		uint8_t ByteIndex = ByteIndexArray.data()[ElementIndex];
 		uint8_t ByteSize  = ByteSizesArray.data()[ElementIndex];
 		size_t ElemCount  = ByteSize;
 		Element.clear(); Element.resize(ElemCount);
 		type* ElementPtr = Element.data();
 		ReturnArrayTypes(ElemCount, ByteIndex, ElementPtr, bPrint, ByteSize);
 	}

 private: 
 	template<typename T>
 	void ReturnArrayTypes(size_t ElemCount, uint8_t ByteIndex, T& Element, bool bPrint, uint8_t& ByteSize)
 	{
 		using type = std::remove_pointer_t<std::remove_all_extents_t<T>>;	
 
  		if (isLittleEndian() && is_regular_v<type> && !is_char_v<T>) {
  			using ConvType = uint_convert_t<type>;
  			for (size_t i = 0; i < ElemCount; i++) {
  				ConvType ConvertT = *reinterpret_cast<ConvType*>(ByteArray.data() + ByteIndex + (i * sizeof(type)));
  				uint_convert<type>::NetToHost(ConvertT);
  				type TempReturnValue = *reinterpret_cast<type*>(&ConvertT);
  				Element[i] = TempReturnValue;
  				if (bPrint) { FormPrint(" ", Element[i], "") }
  			} return;
  		}
  		else { 
  			memcpy(Element, ByteArray.data() + ByteIndex, ByteSize);
  			if constexpr (is_char_v<T>) {
  				Element[ByteSize] = '\0';
  				if (bPrint) { FormPrint(" ", Element, "") }
  				return;
  			}
  			else {
  				if (bPrint) { FormPrint(" ", *Element, "") }
  				return;
  			}
  		}
 	}

	inline void AddIndex(size_t Size) { MaxIndex += Size; }
	size_t   ArraySize;
	size_t   ElementCount;
	size_t   MaxIndex;
	static uint32_t CRCTable[256];

	std::vector<uint8_t> ByteArray;
	std::vector<uint8_t> ByteIndexArray;
	std::vector<uint8_t> ByteSizesArray;

	bool   bCRCisValid;

public:

	void operator=(BytePack& obj) {
		SetByteArray(obj.GetByteArray(), obj.GetArraySize());
	}
};

inline uint32_t BytePack::CRCTable[256];

template<typename T>
void PrintBit(T& value, size_t size = sizeof(T)){
	uint8_t* Bytes = reinterpret_cast<uint8_t*>(&value);
	for (size_t i = 0; i < size; i++){
		for (uint8_t j = 0; j < 8; j++)
		{
			Bytes[(size - 1) - i] >> (7 - j) & 0x1 ? printf("1") : printf("0");
		}
		printf(" ");
	} printf("\n");
}
