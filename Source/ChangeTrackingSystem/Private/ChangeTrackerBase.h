// Copyright DRICODYSS. All Rights Reserved.

#pragma once

namespace UE::Core::Private
{
    struct FChangeTrackerBase
    {
    protected:
        bool bIsChanged = false;

    public:
        TFunction<void()> OnMarkChanged = nullptr;

        virtual ~FChangeTrackerBase() = default;
        
        [[nodiscard]] UE_FORCEINLINE_HINT explicit constexpr operator bool() const
        {
            return this->IsChanged();
        }

        virtual bool IsChanged() const { return bIsChanged; }
        void MarkChanged()
        {
            if (bIsChanged) return;

            bIsChanged = true;
            if (OnMarkChanged) OnMarkChanged();
        }
        virtual void ResetChangedState() { bIsChanged = false; }

        //virtual void CollectChanges(TArray<FDiffEntry>& Out, const FName& Prefix = TEXT("")) = 0;
    };
    
    template<typename PropertyType>
    struct TChangeTrackerBase : public FChangeTrackerBase
    {
    protected:
        PropertyType TypedValue;

    public:
        TChangeTrackerBase() = default;

        TChangeTrackerBase(const PropertyType& InValue) : TypedValue(InValue) {}
        TChangeTrackerBase(PropertyType&& InValue) : TypedValue(MoveTempIfPossible(InValue)) {}

        TChangeTrackerBase& operator=(const TChangeTrackerBase& Other)
        {
            if (&Other != this)
            {
                ChangeValue(Other.GetValue());
            }
            return *this;
        }
        TChangeTrackerBase& operator=(TChangeTrackerBase&& Other)
        {
            if (&Other != this)
            {
                ChangeValue(MoveTempIfPossible(Other.GetValue()));
            }
            return *this;
        }

        TChangeTrackerBase& operator=(const PropertyType& InValue)
        {
            if (std::addressof(InValue) != std::addressof(this->TypedValue))
            {
                ChangeValue(InValue);
            }
            return *this;
        }
        TChangeTrackerBase& operator=(PropertyType&& InValue)
        {
            if (std::addressof(InValue) != std::addressof(this->TypedValue))
            {
                ChangeValue(MoveTempIfPossible(InValue));
            }
            return *this;
        }

        virtual void ChangeValue(const PropertyType& NewValue)
        {
            this->TypedValue = NewValue;
            MarkChanged();
        }
        virtual void ChangeValue(PropertyType&& NewValue)
        {
            this->TypedValue = MoveTemp(NewValue);
            MarkChanged();
        }

        [[nodiscard]] constexpr PropertyType& GetValue()
        {
            return this->TypedValue;
        }
        [[nodiscard]] UE_FORCEINLINE_HINT constexpr const PropertyType& GetValue() const
        {
            return const_cast<TChangeTrackerBase*>(this)->GetValue();
        }

        [[nodiscard]] constexpr PropertyType* operator->()
        {
            return std::addressof(GetValue());
        }
        [[nodiscard]] UE_FORCEINLINE_HINT constexpr const PropertyType* operator->() const
        {
            return const_cast<TChangeTrackerBase*>(this)->operator->();
        }

        [[nodiscard]] constexpr PropertyType& operator*()
        {
            return GetValue();
        }
        [[nodiscard]] UE_FORCEINLINE_HINT constexpr const PropertyType& operator*() const
        {
            return const_cast<TChangeTrackerBase*>(this)->operator*();
        }

        [[nodiscard]] constexpr operator const PropertyType&() const
        {
            return GetValue();
        }
    };
}
