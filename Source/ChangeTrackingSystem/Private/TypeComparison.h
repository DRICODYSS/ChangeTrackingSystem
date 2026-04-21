// Copyright DRICODYSS. All Rights Reserved.

#pragma once

#include "TypeComparisonTraits.h"

namespace ChangeTrackingSystem
{
    template<typename T>
    [[deprecated("No comparison available for this type. Fallback = true")]]
    constexpr bool TNoCompareFallback(){ return true; }

    template<typename T>
    FORCEINLINE bool IsEqual(const T& A, const T& B)
    {
        if constexpr (THasCustomComparator<T>::value)
        {
            return TComparator<T>::IsEqual(A, B);
        }
        else if constexpr (THasBoolEqual<T>::value)
        {
            return A == B;
        }
        else
        {
            return TNoCompareFallback<T>();
        }
    }
}
