// Copyright DRICODYSS. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChangeTrackerArray.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerArrayTest_Init,
    "ChangeTrackingSystem.ChangeTracker.Array.Init",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerArrayTest_Init::RunTest(const FString&)
{
    const TChangeTracker<TArray<int32>> Tracker{};

    TestFalse(TEXT("Initial IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Initial array empty"), Tracker.GetValue().IsEmpty());
    TestTrue(TEXT("No added"), Tracker.GetAddedElements().IsEmpty());
    TestTrue(TEXT("No changed"), Tracker.GetChangedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerArrayTest_Add,
    "ChangeTrackingSystem.ChangeTracker.Array.Add",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerArrayTest_Add::RunTest(const FString&)
{
    TChangeTracker<TArray<int32>> Tracker{};

    Tracker.Add(1);
    int32 Value = 2;
    Tracker.Add(MoveTemp(Value));

    const TArray<int32> Arr = {3, 4};
    Tracker.Append(Arr);

    TArray<int32> ArrMove = {5, 6};
    Tracker.Append(MoveTemp(ArrMove));

    TestEqual(TEXT("Size after adds"), Tracker.GetValue().Num(), 6);
    TestEqual(TEXT("All elements added"), Tracker.GetAddedElements().Num(), 6);
    TestTrue(TEXT("IsChanged true"), Tracker.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerArrayTest_Replace,
    "ChangeTrackingSystem.ChangeTracker.Array.Replace",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerArrayTest_Replace::RunTest(const FString&)
{
    TChangeTracker<TArray<int32>> Tracker{};
    Tracker.Append({1, 2, 3});
    Tracker.ResetChangedState();

    Tracker.ReplaceAt(1, 20);

    TestTrue(TEXT("Index 1 changed"), Tracker.GetChangedElements().Contains(1));
    TestEqual(TEXT("Value updated"), Tracker[1], 20);

    Tracker.ReplaceAt(1, 20);
    TestEqual(TEXT("No duplicate change"), Tracker.GetChangedElements().Num(), 1);

    Tracker.ReplaceAt(999, 50);
    TestEqual(TEXT("Invalid index ignored"), Tracker.GetChangedElements().Num(), 1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerArrayTest_ReplaceItems,
    "ChangeTrackingSystem.ChangeTracker.Array.ReplaceItems",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerArrayTest_ReplaceItems::RunTest(const FString&)
{
    TChangeTracker<TArray<int32>> Tracker{};
    Tracker.Append({1, 2, 3});
    Tracker.ResetChangedState();

    Tracker.ReplaceItems({10, 20, 30, 40});

    TestEqual(TEXT("Size correct"), Tracker.GetValue().Num(), 4);
    TestTrue(TEXT("Changed indices valid"),
        Tracker.GetChangedElements().Contains(0) &&
        Tracker.GetChangedElements().Contains(1) &&
        Tracker.GetChangedElements().Contains(2)
    );
    TestTrue(TEXT("Added index 3"), Tracker.GetAddedElements().Contains(3));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerArrayTest_Invariant,
    "ChangeTrackingSystem.ChangeTracker.Array.Invariant",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerArrayTest_Invariant::RunTest(const FString&)
{
    TChangeTracker<TArray<int32>> Tracker{};

    Tracker.Add(1);
    Tracker.ReplaceAt(0, 10);

    TestTrue(TEXT("Index stays in Added"), Tracker.GetAddedElements().Contains(0));
    TestFalse(TEXT("Index not in Changed"), Tracker.GetChangedElements().Contains(0));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerArrayTest_Remove,
    "ChangeTrackingSystem.ChangeTracker.Array.Remove",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerArrayTest_Remove::RunTest(const FString&)
{
    TChangeTracker<TArray<int32>> Tracker{};

    Tracker.Add(1);
    Tracker.Remove(1);

    TestTrue(TEXT("Array empty"), Tracker.GetValue().IsEmpty());
    TestTrue(TEXT("Diff invalidated (added empty)"), Tracker.GetAddedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerArrayTest_Reset,
    "ChangeTrackingSystem.ChangeTracker.Array.Reset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerArrayTest_Reset::RunTest(const FString&)
{
    TChangeTracker<TArray<int32>> Tracker{};
    Tracker.Append({1, 2, 3});

    Tracker.Reset();

    TestTrue(TEXT("Array empty"), Tracker.GetValue().IsEmpty());
    TestTrue(TEXT("Diff invalidated"), Tracker.GetAddedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerArrayTest_NoOp,
    "ChangeTrackingSystem.ChangeTracker.Array.NoOp",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerArrayTest_NoOp::RunTest(const FString&)
{
    TChangeTracker<TArray<int32>> Tracker{};
    Tracker.Append({1, 2, 3});
    Tracker.ResetChangedState();

    Tracker.ReplaceItems(Tracker.GetValue());
    TestFalse(TEXT("No change detected"), Tracker.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerArrayTest_Assignment,
    "ChangeTrackingSystem.ChangeTracker.Array.Assignment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerArrayTest_Assignment::RunTest(const FString&)
{
    TChangeTracker<TArray<int32>> A{};
    A.Append({1, 2, 3});

    TChangeTracker<TArray<int32>> B{};
    B = A;

    TestEqual(TEXT("Assignment copied values"), B.GetValue().Num(), 3);
    TestEqual(TEXT("Assignment copied values"), B.GetAddedElements().Num(), 3);
    TestTrue(TEXT("Assignment marks changed"), B.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerArrayTest_Callback,
    "ChangeTrackingSystem.ChangeTracker.Array.Callback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerArrayTest_Callback::RunTest(const FString&)
{
    TChangeTracker<TArray<int32>> Tracker{};

    int32 CallCount = 0;
    Tracker.OnMarkChanged = [&]()
    {
        ++CallCount;
    };

    Tracker.Add(1);
    Tracker.Add(2);

    TestEqual(TEXT("Callback called once"), CallCount, 1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChangeTrackerArrayTest_Complex,
    "ChangeTrackingSystem.ChangeTracker.Array.Complex",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerArrayTest_Complex::RunTest(const FString&)
{
    TChangeTracker<TArray<int32>> Tracker{};

    Tracker.Add(1);
    Tracker.Add(2);
    Tracker.ReplaceAt(0, 10);
    Tracker.Append({3, 4});
    Tracker.Remove(2);

    //TODO: if Remove element, all indices will become invalid
    return false;

    TestTrue(TEXT("IsChanged true"), Tracker.IsChanged());
    TestEqual(TEXT("Assignment copied values"), Tracker.GetAddedElements().Num(), 3);
    TestTrue(TEXT("Size invalidated"), Tracker.GetChangedElements().IsEmpty());
    TestTrue(TEXT("Size invalidated"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

#endif
