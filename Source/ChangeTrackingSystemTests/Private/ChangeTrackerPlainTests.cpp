// Copyright DRICODYSS. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChangeTrackerPlain.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerPlainTest_Init,
    "ChangeTrackingSystem.ChangeTracker.Plain.Init",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPlainTest_Init::RunTest(const FString&)
{
    TChangeTracker<int32> Tracker{};
    TestFalse(TEXT("Initial IsChanged"), Tracker.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerPlainTest_ChangeValue,
    "ChangeTrackingSystem.ChangeTracker.Plain.ChangeValue",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPlainTest_ChangeValue::RunTest(const FString&)
{
    TChangeTracker<int32> Tracker{};

    Tracker.ChangeValue(10);

    TestTrue(TEXT("Changed after ChangeValue"), Tracker.IsChanged());
    TestEqual(TEXT("Value correct"), Tracker.GetValue(), 10);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerPlainTest_NoOp,
    "ChangeTrackingSystem.ChangeTracker.Plain.NoOp",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPlainTest_NoOp::RunTest(const FString&)
{
    TChangeTracker<int32> Tracker{};

    Tracker.ChangeValue(10);
    Tracker.ResetChangedState();

    Tracker.ChangeValue(10);
    TestFalse(TEXT("No change if same value"), Tracker.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerPlainTest_Assignment,
    "ChangeTrackingSystem.ChangeTracker.Plain.Assignment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPlainTest_Assignment::RunTest(const FString&)
{
    TChangeTracker<int32> Tracker{};

    Tracker = 42;

    TestTrue(TEXT("Assignment marks changed"), Tracker.IsChanged());
    TestEqual(TEXT("Value correct"), *Tracker, 42);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerPlainTest_AssignmentNoOp,
    "ChangeTrackingSystem.ChangeTracker.Plain.AssignmentNoOp",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPlainTest_AssignmentNoOp::RunTest(const FString&)
{
    TChangeTracker<int32> Tracker{};

    Tracker = 5;
    Tracker.ResetChangedState();

    Tracker = 5;
    TestFalse(TEXT("Assignment same value does not mark changed"), Tracker.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerPlainTest_Construct,
    "ChangeTrackingSystem.ChangeTracker.Plain.Construct",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPlainTest_Construct::RunTest(const FString&)
{
    TChangeTracker<int32> Tracker(100);

    TestFalse(TEXT("No changed"), Tracker.IsChanged());
    TestEqual(TEXT("Value correct"), Tracker.GetValue(), 100);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerPlainTest_Move,
    "ChangeTrackingSystem.ChangeTracker.Plain.Move",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPlainTest_Move::RunTest(const FString&)
{
    int32 Value = 77;
    TChangeTracker<int32> Tracker(MoveTemp(Value));

    TestFalse(TEXT("No changed"), Tracker.IsChanged());
    TestEqual(TEXT("Value correct"), Tracker.GetValue(), 77);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerPlainTest_CopyTracker,
    "ChangeTrackingSystem.ChangeTracker.Plain.CopyTracker",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPlainTest_CopyTracker::RunTest(const FString&)
{
    TChangeTracker<int32> A(10);
    A.ResetChangedState();

    TChangeTracker<int32> B{};
    B = A;

    TestTrue(TEXT("Copy marks changed"), B.IsChanged());
    TestEqual(TEXT("Value copied"), B.GetValue(), 10);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerPlainTest_Reset,
    "ChangeTrackingSystem.ChangeTracker.Plain.Reset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPlainTest_Reset::RunTest(const FString&)
{
    TChangeTracker<int32> Tracker{};

    Tracker = 10;
    Tracker.ResetChangedState();

    TestFalse(TEXT("Reset clears change flag"), Tracker.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerPlainTest_Callback,
    "ChangeTrackingSystem.ChangeTracker.Plain.Callback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPlainTest_Callback::RunTest(const FString&)
{
    TChangeTracker<int32> Tracker{};

    int32 CallCount = 0;
    Tracker.OnMarkChanged = [&]()
    {
        ++CallCount;
    };

    Tracker = 10;
    Tracker = 20;

    TestEqual(TEXT("Callback called once"), CallCount, 1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerPlainTest_Complex,
    "ChangeTrackingSystem.ChangeTracker.Plain.Complex",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPlainTest_Complex::RunTest(const FString&)
{
    TChangeTracker<int32> Tracker;

    Tracker = 1;
    Tracker = 2;
    Tracker = 5;
    Tracker = 8;

    TestTrue(TEXT("Still changed"), Tracker.IsChanged());
    TestEqual(TEXT("Final value correct"), Tracker.GetValue(), 8);

    return true;
}

#endif
