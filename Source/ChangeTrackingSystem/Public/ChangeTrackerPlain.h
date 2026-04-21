// Copyright DRICODYSS. All Rights Reserved.

#pragma once

#include "ChangeTrackingSystem/Private/TypeComparison.h"
#include "ChangeTrackingSystem/Private/ChangeTracker.h"
#include "ChangeTrackingSystem/Private/ChangeTrackerBase.h"

template<typename PropertyType>
struct TChangeTracker<PropertyType, ETrackedPropertyType::Plain> : UE::Core::Private::TChangeTrackerBase<PropertyType>
{
    using Super = UE::Core::Private::TChangeTrackerBase<PropertyType>;
    using Super::Super;
    using Super::operator=;

public:
    virtual void ChangeValue(const PropertyType& NewValue) override
    {
        if (!ChangeTrackingSystem::IsEqual(this->TypedValue, NewValue))
        {
            Super::ChangeValue(NewValue);
        }
    }
    virtual void ChangeValue(PropertyType&& NewValue) override
    {
        if (!ChangeTrackingSystem::IsEqual(this->TypedValue, NewValue))
        {
            Super::ChangeValue(MoveTemp(NewValue));
        }
    }
};
