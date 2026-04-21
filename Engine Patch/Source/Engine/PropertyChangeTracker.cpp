// Copyright BazaltTheOne. All Rights Reserved.

#include "UObject/PropertyChangeTracker.h"

IMPLEMENT_FIELD(FChangeTrackerProperty)

FChangeTrackerProperty::FChangeTrackerProperty(FFieldVariant InOwner, const FName& InName, EObjectFlags InObjectFlags)
	: Super(InOwner, InName, InObjectFlags)
{
}

FChangeTrackerProperty::FChangeTrackerProperty(FFieldVariant InOwner, const UECodeGen_Private::FGenericPropertyParams& Prop)
	: Super(InOwner, (const UECodeGen_Private::FPropertyParamsBaseWithOffset&)Prop)
{
}

FChangeTrackerProperty::~FChangeTrackerProperty()
{
	if (Inner)
	{
		delete Inner;
		Inner = nullptr;
	}
}

void FChangeTrackerProperty::PostDuplicate(const FField& InField)
{
	const FChangeTrackerProperty& Source = static_cast<const FChangeTrackerProperty&>(InField);
	Inner = CastFieldChecked<FProperty>(FField::Duplicate(Source.Inner, this));
	Super::PostDuplicate(InField);
}

FField* FChangeTrackerProperty::GetInnerFieldByName(const FName& InName)
{
	checkSlow(Inner);
	if (Inner->GetFName() == InName)
	{
		return Inner;
	}
	return nullptr;
}

void FChangeTrackerProperty::GetInnerFields(TArray<FField*>& OutFields)
{
	checkSlow(Inner);
	OutFields.Add(Inner);
	Inner->GetInnerFields(OutFields);
}

void FChangeTrackerProperty::AddCppProperty(FProperty* Property)
{
	SetValueProperty(Property, 0);
}

void FChangeTrackerProperty::SetValueProperty(FProperty* InValueProperty, int32 InValueOffset)
{
	check(!Inner);
	check(InValueProperty);

	Inner = InValueProperty;
	ValueOffset = InValueOffset;
}

void FChangeTrackerProperty::Serialize(FArchive& Ar)
{
	FProperty::Serialize(Ar);
	SerializeSingleField(Ar, Inner, this);
}

void FChangeTrackerProperty::GetPreloadDependencies(TArray<UObject*>& OutDeps)
{
	FProperty::GetPreloadDependencies(OutDeps);
	if (Inner)
	{
		Inner->GetPreloadDependencies(OutDeps);
	}
}

void FChangeTrackerProperty::LinkInternal(FArchive& Ar)
{
	checkSlow(Inner);
	Inner->Link(Ar);
}

bool FChangeTrackerProperty::Identical(const void* A, const void* B, uint32 PortFlags) const
{
	checkSlow(A && Inner);
	return Inner->Identical(GetValuePointer(A), GetValuePointer(B), PortFlags);
}

void FChangeTrackerProperty::SerializeItem(FStructuredArchive::FSlot Slot, void* Data, void const* Defaults) const
{
	checkSlow(Inner);
	void* ValueData = GetValuePointer(Data);
	const void* DefaultData = Defaults ? GetValuePointer(Defaults) : nullptr;
	Inner->SerializeItem(Slot, ValueData, DefaultData);
}

bool FChangeTrackerProperty::NetSerializeItem(FArchive& Ar, UPackageMap* Map, void* Data, TArray<uint8>* MetaData) const
{
	checkSlow(Inner);
	void* ValueData = GetValuePointer(Data);
	return Inner->NetSerializeItem(Ar, Map, ValueData, MetaData);
}

void FChangeTrackerProperty::CopyValuesInternal(void* Dest, void const* Src, int32 Count) const
{
	checkSlow(Inner);
	const int32 WrapperSize = GetSize();
	for (int32 Index = 0; Index < Count; ++Index)
	{
		Inner->CopySingleValue(GetValuePointer(Dest), GetValuePointer(Src));
		Dest = static_cast<uint8*>(Dest) + WrapperSize;
		Src  = static_cast<const uint8*>(Src) + WrapperSize;
	}
}

void FChangeTrackerProperty::ClearValueInternal(void* Data) const
{
	checkSlow(Inner);
	Inner->ClearValue(GetValuePointer(Data));
}

void FChangeTrackerProperty::InitializeValueInternal(void* Data) const
{
	checkSlow(Inner);
	Inner->InitializeValue(GetValuePointer(Data));
}

void FChangeTrackerProperty::DestroyValueInternal(void* Data) const
{
	checkSlow(Inner);
	Inner->DestroyValue(GetValuePointer(Data));
}

int32 FChangeTrackerProperty::GetMinAlignment() const
{
	return Inner ? Inner->GetMinAlignment() : 1;
}

bool FChangeTrackerProperty::SameType(const FProperty* Other) const
{
	if (const FChangeTrackerProperty* OtherTracker = CastField<FChangeTrackerProperty>(Other))
	{
		return Inner->SameType(OtherTracker->Inner);
	}
	return false;
}

void FChangeTrackerProperty::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (Inner)
	{
		Inner->AddReferencedObjects(Collector);
	}
}

void* FChangeTrackerProperty::GetValuePointer(void* Data) const
{
	return static_cast<uint8*>(Data) + ValueOffset;
}

const void* FChangeTrackerProperty::GetValuePointer(const void* Data) const
{
	return static_cast<const uint8*>(Data) + ValueOffset;
}
