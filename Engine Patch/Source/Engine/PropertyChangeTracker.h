// Copyright BazaltTheOne. All Rights Reserved.

#pragma once

#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

#define UE_API COREUOBJECT_API

class FChangeTrackerProperty : public FProperty
{
	DECLARE_FIELD_API(FChangeTrackerProperty, FProperty, CASTCLASS_ChangeTrackerProperty, UE_API)

public:
	FProperty* Inner = nullptr;
	int32 ValueOffset = 0;

public:
	UE_API FChangeTrackerProperty(FFieldVariant InOwner, const FName& InName, EObjectFlags InObjectFlags);
	UE_API FChangeTrackerProperty(FFieldVariant InOwner, const UECodeGen_Private::FGenericPropertyParams& Prop);

	virtual ~FChangeTrackerProperty();

	void SetValueProperty(FProperty* InValueProperty, int32 InValueOffset);

	// UObject interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void GetPreloadDependencies(TArray<UObject*>& OutDeps) override;
	// End of UObject interface

	// Field interface
	UE_API virtual void PostDuplicate(const FField& InField) override;
	UE_API virtual FField* GetInnerFieldByName(const FName& InName) override;
	UE_API virtual void GetInnerFields(TArray<FField*>& OutFields) override;
	UE_API virtual void AddCppProperty(FProperty* Property) override;
	// End of Field interface

	// FProperty interface
	virtual void LinkInternal(FArchive& Ar) override;
	virtual bool Identical(const void* A, const void* B, uint32 PortFlags) const override;
	virtual void SerializeItem(FStructuredArchive::FSlot Slot, void* Data, void const* Defaults) const override;
	virtual bool NetSerializeItem(FArchive& Ar, UPackageMap* Map, void* Data, TArray<uint8>* MetaData = nullptr) const override;
	virtual void CopyValuesInternal(void* Dest, void const* Src, int32 Count) const override;
	virtual void ClearValueInternal(void* Data) const override;
	virtual void InitializeValueInternal(void* Data) const override;
	virtual void DestroyValueInternal(void* Data) const override;
	virtual int32 GetMinAlignment() const override;
	virtual bool SameType(const FProperty* Other) const override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FProperty interface

protected:
	void* GetValuePointer(void* Data) const;
	const void* GetValuePointer(const void* Data) const;
};

#undef UE_API
