// Copyright DRICODYSS. All Rights Reserved.

#pragma once

#include "UObject/PropertyChangeTracker.h"

#include "ChangeTrackingSystem/Private/ChangeTracker.h"
#include "ChangeTrackingSystem/Private/ChangeTrackerBase.h"

template<typename PropertyType>
struct TChangeTracker<PropertyType, ETrackedPropertyType::Pointer> : UE::Core::Private::TChangeTrackerBase<PropertyType>
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
    TChangeTracker() = default;

    TChangeTracker(ENoInit) {}
    TChangeTracker(TYPE_OF_NULLPTR) {}

    TChangeTracker(const PropertyType& InValue) : Super(InValue) { Initialize(); }
    TChangeTracker(PropertyType&& InValue) : Super(MoveTempIfPossible(InValue)) { Initialize(); }

    virtual void ChangeValue(const PropertyType& NewValue) override
    {
        ResetFields();
        Super::ChangeValue(NewValue);
        Initialize();
    }
    virtual void ChangeValue(PropertyType&& NewValue) override
    {
        ResetFields();
        Super::ChangeValue(MoveTemp(NewValue));
        Initialize();
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
        if (!this->TypedValue) return;

        auto RawPtr = ToRawPtr(this->TypedValue);
        UObject* Object = reinterpret_cast<UObject*>(RawPtr);
        const UClass* Class = Object->GetClass();
        for (TFieldIterator<FProperty> It(Class); It; ++It)
        {
            FProperty* Prop = *It;
            if (CastField<FChangeTrackerProperty>(Prop))
            {
                FFieldTrackedInfo FieldInfo{};
                FieldInfo.Name = Prop->GetFName();
                void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Object);
                Core* FieldPtr = reinterpret_cast<Core*>(ValuePtr);
                FieldPtr->OnMarkChanged = [this]()
                {
                    if (this) this->MarkChanged();
                };

                FieldInfo.FieldPtr = FieldPtr;
                FieldsInfo.Add(FieldInfo);
            }
        }
    }

    void ResetFields()
    {
        for (auto& Field : FieldsInfo)
        {
            if (Field.FieldPtr)
            {
                Field.FieldPtr->OnMarkChanged = nullptr;
            }
        }
        FieldsInfo.Empty();
    }
};
