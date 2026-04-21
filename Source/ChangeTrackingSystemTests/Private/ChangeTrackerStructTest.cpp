// Copyright DRICODYSS. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChangeTrackerStruct.h"
#include "TestChangeTrackerStruct.h"

using FStructTracker = TChangeTracker<FTestChangeTrackerStruct>;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerStructTest_Init,
    "ChangeTrackingSystem.ChangeTracker.Struct.Init",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerStructTest_Init::RunTest(const FString&)
{
    FStructTracker Tracker;

    TestFalse(TEXT("Initially not changed"), Tracker.IsChanged());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerStructTest_FieldPropagation,
    "ChangeTrackingSystem.ChangeTracker.Struct.FieldPropagation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerStructTest_FieldPropagation::RunTest(const FString&)
{
    FStructTracker Tracker;

    Tracker.GetValue().ValueA = 10;

    TestTrue(TEXT("Struct marked changed"), Tracker.IsChanged());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerStructTest_MultipleFields,
    "ChangeTrackingSystem.ChangeTracker.Struct.MultipleFields",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerStructTest_MultipleFields::RunTest(const FString&)
{
    FStructTracker Tracker;

    auto& Value = Tracker.GetValue();

    Value.ValueA = 1;
    Value.ValueB = 2;

    TestTrue(TEXT("Changed"), Tracker.IsChanged());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerStructTest_ResetState,
    "ChangeTrackingSystem.ChangeTracker.Struct.ResetState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerStructTest_ResetState::RunTest(const FString&)
{
    FStructTracker Tracker;

    Tracker.GetValue().ValueA = 5;

    Tracker.ResetChangedState();

    TestFalse(TEXT("Tracker reset"), Tracker.IsChanged());
    TestFalse(TEXT("Field reset"), Tracker.GetValue().ValueA.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerStructTest_Assignment,
    "ChangeTrackingSystem.ChangeTracker.Struct.Assignment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerStructTest_Assignment::RunTest(const FString&)
{
    FStructTracker A;
    A.GetValue().ValueA = 10;
    A.ResetChangedState();

    FStructTracker B;
    B = A;

    TestTrue(TEXT("Copied value"), B.GetValue().ValueA.GetValue() == 10);
    TestTrue(TEXT("Assignment triggers change"), B.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerStructTest_ChangeValue,
    "ChangeTrackingSystem.ChangeTracker.Struct.ChangeValue",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerStructTest_ChangeValue::RunTest(const FString&)
{
    FStructTracker Tracker;

    FTestChangeTrackerStruct NewStruct;
    NewStruct.ValueA = 100;

    Tracker = NewStruct;

    TestTrue(TEXT("Changed"), Tracker.IsChanged());
    TestEqual(TEXT("Value copied"), Tracker.GetValue().ValueA.GetValue(), 100);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerStructTest_Callback,
    "ChangeTrackingSystem.ChangeTracker.Struct.Callback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerStructTest_Callback::RunTest(const FString&)
{
    FStructTracker Tracker;

    int32 CallCount = 0;
    Tracker.OnMarkChanged = [&]()
    {
        ++CallCount;
    };

    Tracker.GetValue().ValueA = 1;
    Tracker.GetValue().ValueB = 2;

    TestEqual(TEXT("Called once"), CallCount, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerStructTest_PropertyDiff,
    "ChangeTrackingSystem.ChangeTracker.Struct.PropertyDiff",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerStructTest_PropertyDiff::RunTest(const FString&)
{
    FRandomStream Rand(123);

    for (int32 Iter = 0; Iter < 100; ++Iter)
    {
        FStructTracker Tracker;

        int32 RefA = Tracker->ValueA;
        int32 RefB = Tracker->ValueB;

        for (int32 Step = 0; Step < 50; ++Step)
        {
            int32 NewValue = Rand.RandRange(0, 1000);
            bool bExpectedChange = false;

            switch (Rand.RandRange(0, 2))
            {
            case 0:
                bExpectedChange = (Tracker->ValueA.GetValue() != NewValue);
                Tracker->ValueA = NewValue;
                RefA = NewValue;
                break;

            case 1:
                bExpectedChange = (Tracker->ValueB.GetValue() != NewValue);
                Tracker->ValueB = NewValue;
                RefB = NewValue;
                break;

            case 2:
            {
                FTestChangeTrackerStruct NewStruct{};
                NewStruct.ValueA = RefA;
                NewStruct.ValueB = RefB;

                Tracker.ChangeValue(NewStruct);
                bExpectedChange = true;
                break;
            }
            }

            TestEqual(TEXT("ValueA correct"), Tracker->ValueA.GetValue(), RefA);
            TestEqual(TEXT("ValueB correct"), Tracker->ValueB.GetValue(), RefB);

            bExpectedChange 
                ? TestTrue("Should be marked changed", Tracker.IsChanged()) 
                : TestFalse("No change expected", Tracker.IsChanged());

            Tracker.ResetChangedState();
            bExpectedChange = false;
        }
    }
    return true;
}

#endif
