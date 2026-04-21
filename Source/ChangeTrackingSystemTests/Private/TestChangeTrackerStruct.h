// Copyright BazaltTheOne. All Rights Reserved.
#pragma once

#include "ChangeTrackerPlain.h"
#include "TestChangeTrackerStruct.generated.h"

USTRUCT()
struct FTestChangeTrackerStruct
{
    GENERATED_BODY()

    UPROPERTY()
    TChangeTracker<int32> ValueA = -1;

    UPROPERTY()
    TChangeTracker<int32> ValueB = -1;
};
