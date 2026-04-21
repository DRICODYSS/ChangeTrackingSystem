// Copyright DRICODYSS. All Rights Reserved.

#pragma once

#include "ContainerTraits.h"
#include "ChangeTrackingSystem/Private/ChangeTracker.h"
#include "ChangeTrackingSystem/Private/ChangeTrackerBase.h"

template<typename ContainerType>
struct TChangeTracker<ContainerType, ETrackedPropertyType::Set> : UE::Core::Private::TChangeTrackerBase<ContainerType>
{
private:
    using Super = UE::Core::Private::TChangeTrackerBase<ContainerType>;
    using Traits = TContainerTraits<ContainerType>;
    using ElementType = typename ContainerType::ElementType;

    using KeyInitType = typename Traits::KeyInitType;

protected:
    TSet<ElementType> AddedElements{};
    TSet<ElementType> RemovedElements{};

public:
    TChangeTracker() = default;

    [[nodiscard]] const ContainerType* operator->() const { return &Super::GetValue(); }

    [[nodiscard]] const ContainerType& operator*() const { return Super::GetValue(); }

    [[nodiscard]] const ContainerType& GetValue() const { return Super::GetValue(); }

    [[nodiscard]] const ElementType& operator[](FSetElementId Id) const
    {
        return this->TypedValue[Id.AsInteger()].Value;
    }

    void Reserve(int32 Number)
    {
        AddedElements.Reserve(Number);
        RemovedElements.Reserve(Number);
        this->TypedValue.Reserve(Number);
    }

    UE_FORCEINLINE_HINT FSetElementId Add(const ElementType& InElement, bool* bIsAlreadyInSetPtr = nullptr)
    {
        if (this->TypedValue.Contains(InElement)) return this->TypedValue.FindId(InElement);

        const FSetElementId Id = this->TypedValue.Emplace(InElement, bIsAlreadyInSetPtr);
        if (!bIsAlreadyInSetPtr || !(*bIsAlreadyInSetPtr))
        {
            AddElement(InElement);
        }
        return Id;
    }

    UE_FORCEINLINE_HINT FSetElementId Add(ElementType&& InElement, bool* bIsAlreadyInSetPtr = nullptr)
    {
        const ElementType& ElementRef = InElement;
        if (this->TypedValue.Contains(ElementRef)) return this->TypedValue.FindId(ElementRef);

        const FSetElementId Id = this->TypedValue.Emplace(MoveTempIfPossible(InElement), bIsAlreadyInSetPtr);
        if (!bIsAlreadyInSetPtr || !(*bIsAlreadyInSetPtr))
        {
            AddElement(ElementRef);
        }
        return Id;
    }

    void Append(const ContainerType& OtherSet)
    {
        for (const ElementType& Element : OtherSet)
        {
            Add(Element);
        }
    }

    void Append(TArrayView<const ElementType> Elements)
    {
        for (const ElementType& Etem : Elements)
        {
            Add(Etem);
        }
    }

    void Append(std::initializer_list<ElementType> InitList)
    {
        for (const ElementType& Etem : InitList)
        {
            Add(Etem);
        }
    }

    int32 Remove(KeyInitType Key)
    {
        const int32 NumRemoved = this->TypedValue.Remove(Key);
        if (NumRemoved > 0)
        {
            RemoveElement(Key);
        }
        return NumRemoved;
    }

    void Reset()
    {
        for (const ElementType& Etem : this->TypedValue)
        {
            RemoveElement(Etem);
        }
        this->TypedValue.Reset();
    }

    [[nodiscard]] const TSet<ElementType>& GetAddedElements() const { return AddedElements; }
    [[nodiscard]] const TSet<ElementType>& GetRemovedElements() const { return RemovedElements; }

    virtual void ChangeValue(const ContainerType& NewValue) override
    {
        Reset();
        Append(NewValue);
    }
    virtual void ChangeValue(ContainerType&& NewValue) override
    {
        Reset();
        Append(NewValue);
    }

    virtual void ResetChangedState() override
    {
        AddedElements.Empty();
        RemovedElements.Empty();

        Super::ResetChangedState();
    }

private:
    void AddElement(const ElementType& InElement)
    {
        if (RemovedElements.Contains(InElement))
        {
            RemovedElements.Remove(InElement);
            if (RemovedElements.IsEmpty() && AddedElements.IsEmpty())
            {
                this->ResetChangedState();
            }
            return;
        }

        AddedElements.Add(InElement);
        this->MarkChanged();
    }
    void RemoveElement(const ElementType& InElement)
    {
        if (AddedElements.Contains(InElement))
        {
            AddedElements.Remove(InElement);
            if (RemovedElements.IsEmpty() && AddedElements.IsEmpty())
            {
                this->ResetChangedState();
            }
            return;
        }
        RemovedElements.Add(InElement);
        this->MarkChanged();
    }
};
