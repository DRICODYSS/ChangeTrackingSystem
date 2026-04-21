// Copyright DRICODYSS. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChangeTrackerSet.h"

using FIntSet = TSet<int32>;
using FSetTracker = TChangeTracker<FIntSet>;

namespace
{
    bool CheckDisjoint(const FSetTracker& T)
    {
        const auto& A = T.GetAddedElements();
        const auto& R = T.GetRemovedElements();

        for (int32 K : A)
            if (R.Contains(K)) return false;

        for (int32 K : R)
            if (A.Contains(K)) return false;

        return true;
    }

    bool CheckAddedValid(const FSetTracker& T)
    {
        for (int32 K : T.GetAddedElements())
            if (!T.GetValue().Contains(K)) return false;

        return true;
    }

    bool CheckRemovedValid(const FSetTracker& T)
    {
        for (int32 K : T.GetRemovedElements())
            if (T.GetValue().Contains(K)) return false;

        return true;
    }

    void ComputeReferenceDiff(
        const FIntSet& Base,
        const FIntSet& Current,
        TSet<int32>& OutAdded,
        TSet<int32>& OutRemoved
    )
    {
        for (int32 K : Current)
            if (!Base.Contains(K)) OutAdded.Add(K);

        for (int32 K : Base)
            if (!Current.Contains(K)) OutRemoved.Add(K);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_Init,
    "ChangeTrackingSystem.ChangeTracker.Set.Init",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_Init::RunTest(const FString&)
{
    FSetTracker Tracker;

    TestFalse(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Empty set"), Tracker.GetValue().IsEmpty());
    TestTrue(TEXT("No added"), Tracker.GetAddedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_Add_New,
    "ChangeTrackingSystem.ChangeTracker.Set.Add.New",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_Add_New::RunTest(const FString&)
{
    FSetTracker Tracker;

    Tracker.Add(1);
    Tracker.Add(2);

    TestTrue(TEXT("Changed"), Tracker.IsChanged());
    TestEqual(TEXT("Size"), Tracker.GetValue().Num(), 2);
    TestEqual(TEXT("Added count"), Tracker.GetAddedElements().Num(), 2);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_Add_Existing,
    "ChangeTrackingSystem.ChangeTracker.Set.Add.Existing",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_Add_Existing::RunTest(const FString&)
{
    FSetTracker Tracker;

    Tracker.Add(1);
    Tracker.Add(1);

    TestEqual(TEXT("Only once added"), Tracker.GetAddedElements().Num(), 1);

    Tracker.ResetChangedState();
    Tracker.Add(1);

    TestFalse(TEXT("No change after duplicate add"), Tracker.IsChanged());
    TestEqual(TEXT("Value stable"), Tracker.GetValue().Num(), 1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_Remove,
    "ChangeTrackingSystem.ChangeTracker.Set.Remove",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_Remove::RunTest(const FString&)
{
    FSetTracker Tracker;

    Tracker.Add(1);
    Tracker.ResetChangedState();

    Tracker.Remove(1);

    TestTrue(TEXT("Changed"), Tracker.IsChanged());
    TestTrue(TEXT("Removed contains"), Tracker.GetRemovedElements().Contains(1));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_RemoveNonExisting,
    "ChangeTrackingSystem.ChangeTracker.Set.RemoveNonExisting",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_RemoveNonExisting::RunTest(const FString&)
{
    FSetTracker Tracker;

    Tracker.Remove(999);

    TestFalse(TEXT("No-op remove"), Tracker.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_AddRemoveCancel,
    "ChangeTrackingSystem.ChangeTracker.Set.AddRemoveCancel",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_AddRemoveCancel::RunTest(const FString&)
{
    FSetTracker Tracker;

    Tracker.Add(1);
    Tracker.Remove(1);

    TestTrue(TEXT("Empty set"), Tracker.GetValue().IsEmpty());
    TestTrue(TEXT("No added"), Tracker.GetAddedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());
    TestFalse(TEXT("No net change"), Tracker.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_RemoveAdd,
    "ChangeTrackingSystem.ChangeTracker.Set.RemoveAdd",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_RemoveAdd::RunTest(const FString&)
{
    FSetTracker Tracker;

    Tracker.Add(1);
    Tracker.ResetChangedState();

    Tracker.Remove(1);
    Tracker.Add(1);

    TestTrue(TEXT("Restored"), Tracker.GetValue().Contains(1));
    TestTrue(TEXT("No diff"),
        Tracker.GetAddedElements().IsEmpty() &&
        Tracker.GetRemovedElements().IsEmpty()
    );
    TestFalse(TEXT("No net change"), Tracker.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_Append,
    "ChangeTrackingSystem.ChangeTracker.Set.Append",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_Append::RunTest(const FString&)
{
    FSetTracker Tracker;

    Tracker.Append({1, 2, 3});

    TestEqual(TEXT("Size"), Tracker.GetValue().Num(), 3);
    TestEqual(TEXT("Added"), Tracker.GetAddedElements().Num(), 3);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_ChangeValue,
    "ChangeTrackingSystem.ChangeTracker.Set.ChangeValue",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_ChangeValue::RunTest(const FString&)
{
    FSetTracker Tracker;

    Tracker.Append({1, 2});
    Tracker.ResetChangedState();

    Tracker.ChangeValue({2, 3});

    TestTrue(TEXT("Contains 2"), Tracker.GetValue().Contains(2));
    TestTrue(TEXT("Contains 3"), Tracker.GetValue().Contains(3));
    TestTrue(TEXT("Removed 1"), Tracker.GetRemovedElements().Contains(1));
    TestTrue(TEXT("Added 3"), Tracker.GetAddedElements().Contains(3));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_Reset,
    "ChangeTrackingSystem.ChangeTracker.Set.Reset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_Reset::RunTest(const FString&)
{
    FSetTracker Tracker;

    Tracker.Append({1, 2, 3});
    Tracker.ResetChangedState();

    Tracker.Reset();

    TestTrue(TEXT("Empty"), Tracker.GetValue().IsEmpty());
    TestEqual(TEXT("Removed all"), Tracker.GetRemovedElements().Num(), 3);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_Callback,
    "ChangeTrackingSystem.ChangeTracker.Set.Callback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_Callback::RunTest(const FString&)
{
    FSetTracker Tracker;

    int32 CallCount = 0;
    Tracker.OnMarkChanged = [&]()
    {
        ++CallCount;
    };

    Tracker.Add(1);
    Tracker.Add(2);

    TestEqual(TEXT("Called once"), CallCount, 1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_Property,
    "ChangeTrackingSystem.ChangeTracker.Set.Property",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_Property::RunTest(const FString&)
{
    FRandomStream Rand(42);

    for (int32 Iter = 0; Iter < 300; ++Iter)
    {
        FSetTracker Tracker;

        for (int32 Step = 0; Step < 100; ++Step)
        {
            int32 K = Rand.RandRange(0, 10);

            switch (Rand.RandRange(0, 2))
            {
                case 0: Tracker.Add(K); break;
                case 1: Tracker.Remove(K); break;
                case 2: Tracker.Reset(); break;
            }

            TestTrue(TEXT("Disjoint"), CheckDisjoint(Tracker));
            TestTrue(TEXT("Added valid"), CheckAddedValid(Tracker));
            TestTrue(TEXT("Removed valid"), CheckRemovedValid(Tracker));
        }
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_Complex,
    "ChangeTrackingSystem.ChangeTracker.Set.Complex",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_Complex::RunTest(const FString&)
{
    FRandomStream Rand(1337);

    for (int32 Iter = 0; Iter < 500; ++Iter)
    {
        FSetTracker Tracker;

        for (int32 Step = 0; Step < 200; ++Step)
        {
            int32 K = Rand.RandRange(0, 20);

            switch (Rand.RandRange(0, 3))
            {
                case 0: Tracker.Add(K); break;
                case 1: Tracker.Remove(K); break;
                case 2:
                {
                    TArray<int32> Tmp = { K };
                    Tracker.Append(Tmp);
                    break;
                }
                case 3: Tracker.Reset(); break;
            }

            TestTrue(TEXT("Disjoint"), CheckDisjoint(Tracker));
        }
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerSetTest_DiffCorrectness,
    "ChangeTrackingSystem.ChangeTracker.Set.DiffCorrectness",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerSetTest_DiffCorrectness::RunTest(const FString&)
{
    FRandomStream Rand(777);

    for (int32 Iter = 0; Iter < 200; ++Iter)
    {
        FSetTracker Tracker;
        FIntSet Base;

        for (int i = 0; i < 5; ++i)
        {
            int32 K = Rand.RandRange(0, 20);
            Base.Add(K);
            Tracker.Add(K);
        }

        FIntSet Initial = Base;
        Tracker.ResetChangedState();

        for (int32 Step = 0; Step < 50; ++Step)
        {
            int32 K = Rand.RandRange(0, 20);

            if (Rand.RandRange(0, 1) == 0)
            {
                Tracker.Add(K);
                Base.Add(K);
            }
            else
            {
                Tracker.Remove(K);
                Base.Remove(K);
            }
        }

        TSet<int32> A, R;
        ComputeReferenceDiff(Initial, Tracker.GetValue(), A, R);

        TestTrue(TEXT("Added match"), A.Includes(Tracker.GetAddedElements()) && Tracker.GetAddedElements().Includes(A));
        TestTrue(TEXT("Removed match"), R.Includes(Tracker.GetRemovedElements()) && Tracker.GetRemovedElements().Includes(R));
    }

    return true;
}

#endif
