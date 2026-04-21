// Copyright DRICODYSS. All Rights Reserved.

#pragma once

#include "ChangeTrackingSystem/Private/TypeComparison.h"
#include "ChangeTrackingSystem/Private/ChangeTracker.h"
#include "ChangeTrackingSystem/Private/ChangeTrackerBase.h"

template<typename ContainerType>
struct TChangeTracker<ContainerType, ETrackedPropertyType::Map> : UE::Core::Private::TChangeTrackerBase<ContainerType>
{
    using Super = UE::Core::Private::TChangeTrackerBase<ContainerType>;
    using KeyType = typename ContainerType::KeyType;
    using ValueType = typename ContainerType::ValueType;
    using KeyConstPointerType = typename ContainerType::KeyConstPointerType;

protected:
    TSet<KeyType> AddedKeys{};
    TSet<KeyType> RemovedKeys{};
    TSet<KeyType> ChangedKeys{};

    struct FValueProxy
    {
    private:
        TChangeTracker* Owner;
        KeyConstPointerType Key;

    public:
        FValueProxy(TChangeTracker* InOwner, KeyConstPointerType InKey) : Owner(InOwner), Key(InKey) {}

        FValueProxy& operator=(const ValueType& NewValue)
        {
            Owner->Set(Key, NewValue);
            return *this;
        }

        FValueProxy& operator=(ValueType&& NewValue)
        {
            Owner->Set(Key, MoveTempIfPossible(NewValue));
            return *this;
        }

        explicit operator ValueType() const
        {
            return Owner->GetValue()[Key];
        }
    };

public:
    TChangeTracker() = default;

    [[nodiscard]] const ContainerType* operator->() const { return &Super::GetValue(); }

    [[nodiscard]] const ContainerType& operator*() const { return Super::GetValue(); }

    [[nodiscard]] const ContainerType& GetValue() const { return Super::GetValue(); }

    [[nodiscard]] UE_FORCEINLINE_HINT FValueProxy operator[](KeyConstPointerType Key)
    {
        return FValueProxy(this, Key);
    }
    [[nodiscard]] UE_FORCEINLINE_HINT const ValueType& operator[](KeyConstPointerType Key) const
    {
        return this->TypedValue[Key];
    }

    void Reserve(int32 Number)
    {
        AddedKeys.Reserve(Number);
        RemovedKeys.Reserve(Number);
        ChangedKeys.Reserve(Number);
        this->TypedValue.Reserve(Number);
    }
    
    template <typename InitKeyType, typename InitValueType>
    ValueType& Add(InitKeyType&& InKey, InitValueType&& InValue)
    {
        const KeyType& KeyRef = InKey;

        if (!this->TypedValue.Contains(KeyRef))
        {
            AddElement(KeyRef);
            return this->TypedValue.Emplace(Forward<InitKeyType>(InKey), Forward<InitValueType>(InValue));
        }

        ValueType& Existing = this->TypedValue[KeyRef];

        if (!ChangeTrackingSystem::IsEqual(Existing, InValue))
        {
            ChangeElement(KeyRef);
            Existing = Forward<InitValueType>(InValue);
        }

        return Existing;
    }

    void Append(ContainerType&& OtherMap)
    {
        this->TypedValue.Reserve(this->TypedValue.Num() + OtherMap.Num());
        for (auto& Pair : OtherMap)
        {
            Add(MoveTempIfPossible(Pair.Key), MoveTempIfPossible(Pair.Value));
        }
        OtherMap.Reset();
    }
    void Append(const ContainerType& OtherMap)
    {
        this->TypedValue.Reserve(this->TypedValue.Num() + OtherMap.Num());
        for (auto& Pair : OtherMap)
        {
            Add(Pair.Key, Pair.Value);
        }
    }

    template<typename InitValueType = ValueType>
    void Set(KeyConstPointerType InKey, InitValueType&& InValue)
    {
        if (!this->TypedValue.Contains(InKey) ||
            ChangeTrackingSystem::IsEqual(this->TypedValue[InKey], InValue)
        ) return;

        ChangeElement(InKey);
        this->TypedValue[InKey] = Forward<InitValueType>(InValue);
    }

    int32 Remove(KeyConstPointerType InKey)
    {
        const int32 NumRemoved = this->TypedValue.Remove(InKey);
        if (NumRemoved > 0)
        {
            RemoveElement(InKey); // TODO need check collisions
        }

        return NumRemoved;
    }

    void Reset()
    {
        for (auto& Pair : this->TypedValue)
        {
            RemoveElement(Pair.Key);
        }
        this->TypedValue.Reset();
    }

    [[nodiscard]] const TSet<KeyType>& GetAddedElements() const { return AddedKeys; }
    [[nodiscard]] const TSet<KeyType>& GetChangedElements() const { return ChangedKeys; }
    [[nodiscard]] const TSet<KeyType>& GetRemovedElements() const { return RemovedKeys; }

    virtual void ChangeValue(const ContainerType& NewValue) override
    {
        Reset();
        Append(NewValue);
    }
    virtual void ChangeValue(ContainerType&& NewValue) override
    {
        Reset();
        Append(MoveTempIfPossible(NewValue));
    }

    virtual void ResetChangedState() override
    {
        AddedKeys.Reset();
        ChangedKeys.Reset();
        RemovedKeys.Reset();

        Super::ResetChangedState();
    }

private:
    void AddElement(const KeyType& Key)
    {
        if (RemovedKeys.Contains(Key))
        {
            RemovedKeys.Remove(Key);
            ChangedKeys.Add(Key);
            this->MarkChanged();
            return;
        }

        AddedKeys.Add(Key);
        this->MarkChanged();
    }

    void ChangeElement(const KeyType& Key)
    {
        if (AddedKeys.Contains(Key)) return;

        ChangedKeys.Add(Key);
        this->MarkChanged();
    }

    void RemoveElement(const KeyType& Key)
    {
        if (AddedKeys.Contains(Key))
        {
            AddedKeys.Remove(Key);
            if (RemovedKeys.IsEmpty() && AddedKeys.IsEmpty() && ChangedKeys.IsEmpty())
            {
                this->ResetChangedState();
            }
            return;
        }

        ChangedKeys.Remove(Key);
        RemovedKeys.Add(Key);
        this->MarkChanged();
    }
};
