// Copyright DRICODYSS. All Rights Reserved.

#pragma once

#define DEFINE_COMPARATOR(Type)                     \
    template<>                                      \
    struct ChangeTrackingSystem::TComparator<Type>

namespace ChangeTrackingSystem
{
    template<typename T>
    struct TComparator;

    /* Example 
    *   template<typename T>
    *   struct TComparator
    *   {
    *       static bool IsEqual(const T& A, const T& B)
    *       {
    *           return ..Your implementation..;
    *       }
    *   };
    */
}
