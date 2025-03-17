#pragma once

#include "CoreMinimal.h"

#define KEY(Name, Type, Value) \
    typedef struct F##Name { \
        F##Name() : _key(Value) {} \
        F##Name(const F##Name& other) : _key(other._key) {} \
        F##Name(F##Name&& other) noexcept : _key(other._key) {} \
        F##Name(const Type rawValue) : _key(rawValue) {} \
        void SetKeyRawValue(const Type rawValue) { if (_key == Value) _key = rawValue;} \
        void Clear() { _key = Value; } \
        __forceinline const Type GetKey() const { return _key; } \
        bool IsValid() const { return _key > Value; } \
        bool operator==(const F##Name& other) const { return _key == other._key; } \
        bool operator!=(const F##Name& other) const { return !(*this == other); } \
        static constexpr Type InitValue = Value; \
        private:\
        Type _key;\
    } Name; \
    __forceinline uint32 GetTypeHash(const F##Name& other) { return ::GetTypeHash(other.GetKey()); }   


KEY(MonsterKey,     int32, -1)
KEY(StageKey,       int32, -1)
KEY(SpawnGroupKey,  int32, -1)
KEY(QuestKey,       int32, -1)
KEY(ItemKey,        int32, -1)
KEY(DropGroupKey,   int32, -1)
KEY(StoreKey,       int32, -1)