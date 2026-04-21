// Copyright DRICODYSS. All Rights Reserved.

#pragma once

#include "ChangeTrackingSystem/Private/TypeComparison.h"
#include "ChangeTrackingSystem/Private/ChangeTracker.h"
#include "ChangeTrackingSystem/Private/ChangeTrackerBase.h"

template<typename ContainerType>
struct TChangeTracker<ContainerType, ETrackedPropertyType::Array> : UE::Core::Private::TChangeTrackerBase<ContainerType>
{
private:
    using Super = UE::Core::Private::TChangeTrackerBase<ContainerType>;
    using ElementType = typename ContainerType::ElementType;
    using SizeType = typename ContainerType::SizeType;

protected:
    TSet<SizeType> AddedIndices{};
    TSet<SizeType> RemovedIndices{};
    TSet<SizeType> ChangedIndices{};

public:
    TChangeTracker() = default;

    [[nodiscard]] const ContainerType* operator->() const { return &Super::GetValue(); }

    [[nodiscard]] const ContainerType& operator*() const { return Super::GetValue(); }

    [[nodiscard]] const ContainerType& GetValue() const { return Super::GetValue(); }
    
    [[nodiscard]] const ElementType& operator[](SizeType Index) const UE_LIFETIMEBOUND
    {
        return this->TypedValue[Index];
    }

    void Reserve(SizeType Number)
    {
        AddedIndices.Reserve(Number);
        RemovedIndices.Reserve(Number);
        ChangedIndices.Reserve(Number);
        this->TypedValue.Reserve(Number);
    }

    void Init(const ElementType& Element, SizeType Number)
    {
        this->TypedValue.Init(Element, Number);
    }

    UE_FORCEINLINE_HINT SizeType Add(ElementType&& Item)
    {
        const SizeType Index = this->TypedValue.Add(MoveTempIfPossible(Item));
        AddItem(Index);
        return Index;
    }

    UE_FORCEINLINE_HINT SizeType Add(const ElementType& Item)
    {
        const SizeType Index = this->TypedValue.Add(Item);
        AddItem(Index);
        return Index;
    }
    
    void Append(const ContainerType& Other)
    {
        if (Other.Num() == 0) return;

        const SizeType StartIndex = this->TypedValue.Num();
        const SizeType Count = Other.Num();
        this->TypedValue.Append(Other);

        for (SizeType i = 0; i < Count; ++i)
        {
            const SizeType Index = StartIndex + i;
            AddItem(Index);
        }
    }

    void Append(ContainerType&& Other)
    {
        if (Other.Num() == 0) return;

        const SizeType StartIndex = this->TypedValue.Num();
        const SizeType Count = Other.Num();
        this->TypedValue.Append(MoveTempIfPossible(Other));

        for (SizeType i = 0; i < Count; ++i)
        {
            const SizeType Index = StartIndex + i;
            AddItem(Index);
        }
    }

    void Append(const ElementType* Ptr, SizeType Count)
    {
        if (Ptr != nullptr || Count == 0) return;

        const SizeType StartIndex = this->TypedValue.Num();
        this->TypedValue.Append(Ptr, Count);

        for (SizeType i = 0; i < Count; ++i)
        {
            const SizeType Index = StartIndex + i;
            AddItem(Index);
        }
    }

    void ReplaceAt(SizeType Index, const ElementType& NewValue)
    {
        if (!this->TypedValue.IsValidIndex(Index)) return;

        if (!ChangeTrackingSystem::IsEqual(this->TypedValue[Index], NewValue))
        {
            this->TypedValue[Index] = NewValue;
            ChangeItem(Index);
        }
    }

    void ReplaceItems(const ContainerType& Other)
    {
        const SizeType Count = Other.Num();
        for (SizeType i = 0; i < Count; ++i)
        {
            if (this->TypedValue.IsValidIndex(i) && !ChangeTrackingSystem::IsEqual(this->TypedValue[i], Other[i]))
            {
                this->TypedValue[i] = Other[i];
                ChangeItem(i);
            }
            else if (!this->TypedValue.IsValidIndex(i))
            {
                this->TypedValue.Add(Other[i]);
                AddItem(i);
            }
        }
    }

    void ReplaceItems(ContainerType&& Other)
    {
        const SizeType Count = Other.Num();
        for (SizeType i = 0; i < Count; ++i)
        {
            if (this->TypedValue.IsValidIndex(i) && !ChangeTrackingSystem::IsEqual(this->TypedValue[i], Other[i]))
            {
                this->TypedValue[i] = Other[i];
                ChangeItem(i);
            }
            else if (!this->TypedValue.IsValidIndex(i))
            {
                this->TypedValue.Add(MoveTempIfPossible(Other[i]));
                AddItem(i);
            }
        }
    }
    
    void ReplaceItems(const ElementType* Ptr, SizeType Count)
    {
        if (!Ptr || Count <= 0) return;

        for (SizeType i = 0; i < Count; ++i)
        {
            if (this->TypedValue.IsValidIndex(i) && !ChangeTrackingSystem::IsEqual(this->TypedValue[i], *(Ptr + i)))
            {
                this->TypedValue[i] = *(Ptr + i);
                ChangeItem(i);
            }
            else if (!this->TypedValue.IsValidIndex(i))
            {
                this->TypedValue.Add(MoveTempIfPossible(*(Ptr + i)));
                AddItem(i);
            }
        }
    }

    SizeType Remove(const ElementType& Item)
    {
        for (SizeType i = 0; i < this->TypedValue.Num(); ++i)
        {
            if (this->TypedValue[i] != Item) continue;
            RemoveItem(i);
        }
        return this->TypedValue.Remove(Item);
    }

    void RemoveAt(SizeType Index)
    {
        if (!this->TypedValue.IsValidIndex(Index)) return;

        RemoveItem(Index);
        this->TypedValue.RemoveAt(Index);
    }

    void Reset(SizeType NewSize = 0)
    {
        const SizeType Count = this->TypedValue.Num();
        for (SizeType i = 0; i < Count; ++i)
        {
            RemoveItem(i);
        }
        this->TypedValue.Reset(NewSize);
    }

    [[nodiscard]] const TSet<SizeType>& GetAddedElements() const { return AddedIndices; }
    [[nodiscard]] const TSet<SizeType>& GetChangedElements() const { return ChangedIndices; }
    [[nodiscard]] const TSet<SizeType>& GetRemovedElements() const { return RemovedIndices; }

    virtual void ChangeValue(const ContainerType& NewValue) override
    {
        ReplaceItems(NewValue);
    }
    virtual void ChangeValue(ContainerType&& NewValue) override
    {
        ReplaceItems(NewValue);
    }

    virtual void ResetChangedState() override
    {
        AddedIndices.Empty();
        ChangedIndices.Empty();
        RemovedIndices.Empty();

        Super::ResetChangedState();
    }

private:
    void AddItem(const SizeType Index)
    {
        if (RemovedIndices.Contains(Index))
        {
            RemovedIndices.Remove(Index);
            ChangedIndices.Add(Index);
            this->MarkChanged();
            return;
        }

        AddedIndices.Add(Index);
        this->MarkChanged();
    }
    void ChangeItem(const SizeType Index)
    {
        if (AddedIndices.Contains(Index)) return;

        ChangedIndices.Add(Index);
        this->MarkChanged();
    }
    void RemoveItem(const SizeType Index)
    {
        if (AddedIndices.Contains(Index))
        {
            AddedIndices.Remove(Index);
            if (AddedIndices.IsEmpty() && ChangedIndices.IsEmpty() && ChangedIndices.IsEmpty())
            {
                this->ResetChangedState();
            }
            return;
        }
        ChangedIndices.Remove(Index);
        RemovedIndices.Add(Index);
        this->MarkChanged();
    }
};
