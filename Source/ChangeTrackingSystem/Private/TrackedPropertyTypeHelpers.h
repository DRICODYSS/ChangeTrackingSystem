
#pragma once

#include <type_traits>

namespace ChangeTrackingSystem::Helper
{
    template<typename T, typename = void>
    struct TIsUStruct : std::false_type {};

    template<typename T>
    struct TIsUStruct<T, std::void_t<decltype(T::StaticStruct())>> : std::true_type {};
}
