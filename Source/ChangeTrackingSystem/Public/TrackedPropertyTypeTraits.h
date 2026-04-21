// Copyright DRICODYSS. All Rights Reserved.

#pragma once

#include "TrackedPropertyTypes.h"
#include "ChangeTrackingSystem/Private/TrackedPropertyTypeHelpers.h"

template<typename T, typename Enable = void>
struct TTrackedPropertyTypeTraits
{
    static constexpr ETrackedPropertyType Type = ETrackedPropertyType::Plain;
};

// --- UStruct ---
template<typename T>
struct TTrackedPropertyTypeTraits<T, std::enable_if_t<ChangeTrackingSystem::Helper::TIsUStruct<T>::value>>
{
    static constexpr ETrackedPropertyType Type = ETrackedPropertyType::Struct;
};

// --- TArray ---
template<typename T>
struct TTrackedPropertyTypeTraits<T, std::enable_if_t<TIsTArray<T>::Value>>
{
    static constexpr ETrackedPropertyType Type = ETrackedPropertyType::Array;
};

// --- TSet ---
template<typename T>
struct TTrackedPropertyTypeTraits<T, std::enable_if_t<TIsTSet<T>::Value>>
{
    static constexpr ETrackedPropertyType Type = ETrackedPropertyType::Set;
};

// --- TMap ---
template<typename T>
struct TTrackedPropertyTypeTraits<T, std::enable_if_t<TIsTMap<T>::Value>>
{
    static constexpr ETrackedPropertyType Type = ETrackedPropertyType::Map;
};

// --- Pointer ---
template<typename T>
struct TTrackedPropertyTypeTraits<T, std::enable_if_t<std::is_pointer_v<T> || TIsTObjectPtr<T>::Value>>
{
    static constexpr ETrackedPropertyType Type = ETrackedPropertyType::Pointer;
};
