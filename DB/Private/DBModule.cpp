// Copyright Epic Games, Inc. All Rights Reserved.

#include "DB.h"
#include "Modules/ModuleManager.h"


extern "C" __declspec(dllexport) IModuleInterface* InitializeModule() {
    return new FDefaultModuleImpl();
} extern "C" void IMPLEMENT_MODULE_DB() { } uint8** GNameBlocksDebug = FNameDebugVisualizer::GetBlocks(); FChunkedFixedUObjectArray*& GObjectArrayForDebugVisualizers = GCoreObjectArrayForDebugVisualizers; UE::CoreUObject::Private::FStoredObjectPathDebug*& GComplexObjectPathDebug = GCoreComplexObjectPathDebug; UE::CoreUObject::Private::FObjectHandlePackageDebugData*& GObjectHandlePackageDebug = GCoreObjectHandlePackageDebug;  
//void* operator new (size_t Size) {
//    return FMemory::Malloc(Size ? Size : 1, 16ull);
//}  void* operator new[](size_t Size) {
//    return FMemory::Malloc(Size ? Size : 1, 16ull);
//}  void* operator new (size_t Size, const std::nothrow_t&) throw() {
//    return FMemory::Malloc(Size ? Size : 1, 16ull);
//}  void* operator new[](size_t Size, const std::nothrow_t&) throw() {
//    return FMemory::Malloc(Size ? Size : 1, 16ull);
//}  void* operator new (size_t Size, std::align_val_t Alignment) {
//    return FMemory::Malloc(Size ? Size : 1, (std::size_t)Alignment);
//}  void* operator new[](size_t Size, std::align_val_t Alignment) {
//    return FMemory::Malloc(Size ? Size : 1, (std::size_t)Alignment);
//}  void* operator new (size_t Size, std::align_val_t Alignment, const std::nothrow_t&) throw() {
//    return FMemory::Malloc(Size ? Size : 1, (std::size_t)Alignment);
//}  void* operator new[](size_t Size, std::align_val_t Alignment, const std::nothrow_t&) throw() {
//    return FMemory::Malloc(Size ? Size : 1, (std::size_t)Alignment);
//} void operator delete (void* Ptr) {
//    FMemory::Free(Ptr);
//} void operator delete[](void* Ptr) {
//    FMemory::Free(Ptr);
//} void operator delete (void* Ptr, const std::nothrow_t&) throw() {
//    FMemory::Free(Ptr);
//} void operator delete[](void* Ptr, const std::nothrow_t&) throw() {
//    FMemory::Free(Ptr);
//} void operator delete (void* Ptr, size_t Size) {
//    FMemory::Free(Ptr);
//} void operator delete[](void* Ptr, size_t Size) {
//    FMemory::Free(Ptr);
//} void operator delete (void* Ptr, size_t Size, const std::nothrow_t&) throw() {
//    FMemory::Free(Ptr);
//} void operator delete[](void* Ptr, size_t Size, const std::nothrow_t&) throw() {
//    FMemory::Free(Ptr);
//} void operator delete (void* Ptr, std::align_val_t Alignment) {
//    FMemory::Free(Ptr);
//} void operator delete[](void* Ptr, std::align_val_t Alignment) {
//    FMemory::Free(Ptr);
//} void operator delete (void* Ptr, std::align_val_t Alignment, const std::nothrow_t&) throw() {
//    FMemory::Free(Ptr);
//} void operator delete[](void* Ptr, std::align_val_t Alignment, const std::nothrow_t&) throw() {
//    FMemory::Free(Ptr);
//} void operator delete (void* Ptr, size_t Size, std::align_val_t Alignment) {
//    FMemory::Free(Ptr);
//} void operator delete[](void* Ptr, size_t Size, std::align_val_t Alignment) {
//    FMemory::Free(Ptr);
//} void operator delete (void* Ptr, size_t Size, std::align_val_t Alignment, const std::nothrow_t&) throw() {
//    FMemory::Free(Ptr);
//} void operator delete[](void* Ptr, size_t Size, std::align_val_t Alignment, const std::nothrow_t&) throw() {
//    FMemory::Free(Ptr);
//} void* StdMalloc(size_t Size, size_t Alignment) {
//    return FMemory::Malloc(Size ? Size : 1, Alignment);
//} void* StdRealloc(void* Original, size_t Size, size_t Alignment) {
//    return FMemory::Realloc(Original, Size ? Size : 1, Alignment);
//} void StdFree(void* Ptr) {
//    FMemory::Free(Ptr);
//}
