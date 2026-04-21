// Copyright DRICODYSS. All Rights Reserved.

#pragma once

#include <type_traits>
#include <utility>

#include "TypeComparators.h"

namespace ChangeTrackingSystem
{
    template<typename, typename = void>
    struct THasBoolEqual : std::false_type {};

    template<typename T>
    struct THasBoolEqual<T,std::void_t<decltype(std::declval<T>() == std::declval<T>())>> : 
        std::bool_constant<std::is_convertible_v<decltype(std::declval<T>() == std::declval<T>()), bool>> {};

    template<typename, typename = void>
    struct THasCustomComparator : std::false_type {};

    template<typename T>
    struct THasCustomComparator<T,std::void_t<decltype(TComparator<T>::IsEqual(std::declval<T>(),std::declval<T>()))>> 
        : std::true_type {};
}
