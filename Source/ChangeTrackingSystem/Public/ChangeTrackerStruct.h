// Copyright DRICODYSS. All Rights Reserved.

#pragma once

#include "UObject/PropertyChangeTracker.h"

#include "ChangeTrackingSystem/Private/ChangeTracker.h"
#include "ChangeTrackingSystem/Private/ChangeTrackerBase.h"

template<typename PropertyType>
struct TChangeTracker<PropertyType, ETrackedPropertyType::Struct> : UE::Core::Private::TChangeTrackerBase<PropertyType>
{
    using Core = UE::Core::Private::FChangeTrackerBase;
    using Super = UE::Core::Private::TChangeTrackerBase<PropertyType>;
    using Super::operator=;

private:
    struct FFieldTrackedInfo
    {
        FName Name;
        int32 Offset;
        Core* FieldPtr = nullptr;
    };
    TArray<FFieldTrackedInfo> FieldsInfo;

public:
    TChangeTracker() { Initialize(); }
    explicit TChangeTracker(const PropertyType& InValue)
    {
        Initialize();
        Super::ChangeValue(InValue);
    }
    explicit TChangeTracker(PropertyType&& InValue)
    {
        Initialize();
        Super::ChangeValue(MoveTemp(InValue));
    }

    virtual void ResetChangedState() override
    {
        for (FFieldTrackedInfo& FieldInfo : FieldsInfo)
        {
            if (FieldInfo.FieldPtr)
            {
                FieldInfo.FieldPtr->ResetChangedState();
            }
        }
        Super::ResetChangedState();
    }

private:
    void Initialize()
    {
        FieldsInfo.Empty();

        const UStruct* StructType = PropertyType::StaticStruct();
        for (TFieldIterator<FProperty> It(StructType); It; ++It)
        {
            FProperty* Prop = *It;
            if (CastField<FChangeTrackerProperty>(Prop))
            {
                FFieldTrackedInfo FieldInfo{};
                FieldInfo.Name = Prop->GetFName();
                FieldInfo.Offset = Prop->GetOffset_ForInternal();

                const auto FieldPtr = reinterpret_cast<Core*>(
                    reinterpret_cast<uint8*>(&this->TypedValue) + FieldInfo.Offset
                );

                FieldPtr->OnMarkChanged = [this]()
                {
                    if (this) this->MarkChanged();
                };

                FieldInfo.FieldPtr = FieldPtr;
                FieldsInfo.Add(FieldInfo);
            }
        }
    }
};
