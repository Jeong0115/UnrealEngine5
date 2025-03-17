#pragma once

#include "CoreMinimal.h"

template <typename T>
constexpr typename std::enable_if<std::is_enum<T>::value, int32>::type GetTypeRaw(const T enumType) { return static_cast<int32>(enumType); }


template <typename T> int32         GetEnumSize() { return -1; }
template <typename T> int32         ConvertStringToEnum(const char*) { return -1; }
template <typename T> const char*   ConvertEnumToString(int32) { return ""; }

#define DeclConvertEnumAndString(EnumClass) template<> constexpr int32 GetEnumSize<EnumClass>(); \
    template<> const char* ConvertEnumToString<EnumClass>(int32 raw); \
    template<> int32 ConvertStringToEnum<EnumClass>(const char* str)


#define DefineConvertEnumAndString(EnumClass, ...) const char* EnumClass##String[] = { __VA_ARGS__ }; \
    template<> const char* ConvertEnumToString<EnumClass>(int32 raw) \
    { \
        return EnumClass##String[raw]; \
    } \
    template<> int32 ConvertStringToEnum<EnumClass>(const char* str) \
    { \
        uint32 size = GetEnumSize<EnumClass>() + 1; \
        auto it = std::find_if(EnumClass##String, EnumClass##String + size, [str](const char* elem) { return std::strcmp(elem, str) == 0; }); \
        if (it != EnumClass##String + size) \
        { \
            return static_cast<int32>(it - EnumClass##String); \
        } \
        return -1; \
    }