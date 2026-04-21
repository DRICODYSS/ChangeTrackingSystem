// Copyright DRICODYSS. All Rights Reserved.

#pragma once

template<typename ContainerType>
struct TContainerTraits;

// -------- TSet --------
template<typename ElementType, typename KeyFuncs, typename Allocator>
struct TContainerTraits<TSet<ElementType, KeyFuncs, Allocator>>
{
    using KeyInitType = const ElementType&;
};
