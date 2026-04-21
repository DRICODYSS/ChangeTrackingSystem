// Copyright BazaltTheOne. All Rights Reserved.

#pragma once

#include "ChangeTrackerPlain.h"
#include "TestChangeTrackerObject.generated.h"

UCLASS()
class UTestChangeTrackerObject : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TChangeTracker<int32> ValueA = -1;

    UPROPERTY()
    TChangeTracker<int32> ValueB = -1;
};
