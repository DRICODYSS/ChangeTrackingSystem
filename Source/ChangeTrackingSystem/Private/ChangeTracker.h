// Copyright DRICODYSS. All Rights Reserved.

#pragma once

#include "TrackedPropertyTypeTraits.h"
#include "TrackedPropertyTypes.h"

template<typename T, ETrackedPropertyType Type = TTrackedPropertyTypeTraits<T>::Type>
struct TChangeTracker;
