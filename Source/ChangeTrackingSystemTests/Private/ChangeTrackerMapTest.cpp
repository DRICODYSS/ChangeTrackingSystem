// Copyright DRICODYSS. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChangeTrackerMap.h"

using FIntMap     = TMap<int32, int32>;
using FMapTracker = TChangeTracker<FIntMap>;

namespace 
{
    void MakeCleanTracker(FMapTracker& Tracker)
    {
        Tracker.Add(1, 10);
        Tracker.Add(2, 20);
        Tracker.Add(3, 30);
        Tracker.ResetChangedState();
    }

    bool CheckDisjoint(const FMapTracker& _Tracker)
    {
        const auto& A = _Tracker.GetAddedElements();
        const auto& C = _Tracker.GetChangedElements();
        const auto& R = _Tracker.GetRemovedElements();

        for (int32 K : A)
            if (C.Contains(K) || R.Contains(K)) return false;

        for (int32 K : C)
            if (A.Contains(K) || R.Contains(K)) return false;

        for (int32 K : R)
            if (A.Contains(K) || C.Contains(K)) return false;

        return true;
    }

    bool CheckAddedValid(const FMapTracker& _Tracker)
    {
        for (int32 K : _Tracker.GetAddedElements())
            if (!_Tracker.GetValue().Contains(K)) return false;
        return true;
    }

    bool CheckRemovedValid(const FMapTracker& _Tracker)
    {
        for (int32 K : _Tracker.GetRemovedElements())
            if (_Tracker.GetValue().Contains(K)) return false;
        return true;
    }

    void ComputeReferenceDiff(
        const FIntMap& Base,
        const FIntMap& Current,
        TSet<int32>& OutAdded,
        TSet<int32>& OutChanged,
        TSet<int32>& OutRemoved
    )
    {
        for (auto& Pair : Current)
        {
            if (!Base.Contains(Pair.Key))           OutAdded.Add(Pair.Key);
            else if (Base[Pair.Key] != Pair.Value)  OutChanged.Add(Pair.Key);
        }

        for (auto& Pair : Base)
        {
            if (!Current.Contains(Pair.Key)) OutRemoved.Add(Pair.Key);
        }
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_Init,
    "ChangeTrackingSystem.ChangeTracker.Map.Init",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_Init::RunTest(const FString&)
{
    FMapTracker Tracker{};

    TestFalse(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Map empty"), Tracker.GetValue().IsEmpty());
    TestTrue(TEXT("No added"), Tracker.GetAddedElements().IsEmpty());
    TestTrue(TEXT("No changed"), Tracker.GetChangedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_Add_New,
    "ChangeTrackingSystem.ChangeTracker.Map.Add.New",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_Add_New::RunTest(const FString&)
{
    FMapTracker Tracker;

    Tracker.Add(1, 10);

    TestTrue(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Add added"), Tracker.GetAddedElements().Contains(1));
    TestTrue(TEXT("No changed"), Tracker.GetChangedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_Add_Existing,
    "ChangeTrackingSystem.ChangeTracker.Map.Add.Existing",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_Add_Existing::RunTest(const FString&)
{
    FMapTracker Tracker;
    MakeCleanTracker(Tracker);

    Tracker.Add(1, 99);

    TestTrue(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Add changed"), Tracker.GetChangedElements().Contains(1));
    TestTrue(TEXT("No added"), Tracker.GetAddedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_Set,
    "ChangeTrackingSystem.ChangeTracker.Map.Set.Simple",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_Set::RunTest(const FString&)
{
    FMapTracker Tracker;
    MakeCleanTracker(Tracker);

    Tracker.Set(2, 999);

    TestTrue(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Add changed"), Tracker.GetChangedElements().Contains(2));
    TestEqual(TEXT("Value"), Tracker.GetValue()[2], 999);
    TestTrue(TEXT("No added"), Tracker.GetAddedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_Set_EdgeCases,
    "ChangeTrackingSystem.ChangeTracker.Map.Set.EdgeCases",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_Set_EdgeCases::RunTest(const FString&)
{
    FMapTracker Tracker;
    MakeCleanTracker(Tracker);

    Tracker.Set(999, 1);
    TestFalse(TEXT("No change on non-existing Set"), Tracker.IsChanged());

    Tracker.Set(1, 10);
    TestFalse(TEXT("No change on same value"), Tracker.IsChanged());
    TestTrue(TEXT("No changed"), Tracker.GetChangedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_Remove,
    "ChangeTrackingSystem.ChangeTracker.Map.Remove",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_Remove::RunTest(const FString&)
{
    FMapTracker Tracker;
    MakeCleanTracker(Tracker);

    Tracker.Remove(1);

    TestTrue(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Add removed"), Tracker.GetRemovedElements().Contains(1));
    TestFalse(TEXT("Exists"), Tracker.GetValue().Contains(1));
    TestTrue(TEXT("No added"), Tracker.GetAddedElements().IsEmpty());
    TestTrue(TEXT("No changed"), Tracker.GetChangedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_AddRemove,
    "ChangeTrackingSystem.ChangeTracker.Map.AddRemove",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_AddRemove::RunTest(const FString&)
{
    FMapTracker Tracker;

    Tracker.Add(1, 10);
    Tracker.Remove(1);

    TestFalse(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Map empty"), Tracker.GetValue().IsEmpty());
    TestTrue(TEXT("No added"), Tracker.GetAddedElements().IsEmpty());
    TestTrue(TEXT("No changed"), Tracker.GetChangedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_RemoveAdd,
    "ChangeTrackingSystem.ChangeTracker.Map.RemoveAdd",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_RemoveAdd::RunTest(const FString&)
{
    FMapTracker Tracker;
    MakeCleanTracker(Tracker);

    Tracker.Remove(1);
    Tracker.Add(1, 100);

    TestTrue(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Add Changed"), Tracker.GetChangedElements().Contains(1));
    TestTrue(TEXT("No added"), Tracker.GetAddedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_Proxy,
    "ChangeTrackingSystem.ChangeTracker.Map.Proxy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_Proxy::RunTest(const FString&)
{
    FMapTracker Tracker;
    MakeCleanTracker(Tracker);

    Tracker[1] = 500;

    TestTrue(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Add changed"), Tracker.GetChangedElements().Contains(1));
    TestEqual(TEXT("Value"), Tracker.GetValue()[1], 500);
    TestTrue(TEXT("No added"), Tracker.GetAddedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_Append,
    "ChangeTrackingSystem.ChangeTracker.Map.Append",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_Append::RunTest(const FString&)
{
    FMapTracker Tracker;

    FIntMap _Map;
    _Map.Add(1, 10);

    Tracker.Append(_Map);

    TestTrue(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Add added"), Tracker.GetAddedElements().Contains(1));
    TestTrue(TEXT("No changed"), Tracker.GetChangedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_Reset,
    "ChangeTrackingSystem.ChangeTracker.Map.Reset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_Reset::RunTest(const FString&)
{
    FMapTracker Tracker;
    MakeCleanTracker(Tracker);
    int32 ElemCount = Tracker->Num();

    Tracker.Reset();

    TestTrue(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Map empty"), Tracker.GetValue().IsEmpty());
    TestTrue(TEXT("No added"), Tracker.GetAddedElements().IsEmpty());
    TestTrue(TEXT("No changed"), Tracker.GetChangedElements().IsEmpty());
    TestEqual(TEXT("All removed"), Tracker.GetRemovedElements().Num(), ElemCount);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_ResetState,
    "ChangeTrackingSystem.ChangeTracker.Map.ResetState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_ResetState::RunTest(const FString&)
{
    FMapTracker Tracker;

    Tracker.Add(1, 10);
    Tracker.ResetChangedState();

    TestFalse(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("No added"), Tracker.GetAddedElements().IsEmpty());
    TestTrue(TEXT("No changed"), Tracker.GetChangedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_ChangeValue,
    "ChangeTrackingSystem.ChangeTracker.Map.ChangeValue",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_ChangeValue::RunTest(const FString&)
{
    FMapTracker Tracker;

    FIntMap _Map;
    _Map.Add(5, 50);

    Tracker.ChangeValue(_Map);
    
    TestTrue(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Add added"), Tracker.GetAddedElements().Contains(5));
    TestTrue(TEXT("No changed"), Tracker.GetChangedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_MoveChangeValue,
    "ChangeTrackingSystem.ChangeTracker.Map.MoveChangeValue",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_MoveChangeValue::RunTest(const FString&)
{
    FMapTracker Tracker;

    FIntMap _Map;
    _Map.Add(1, 10);

    Tracker.ChangeValue(MoveTemp(_Map));

    TestEqual(TEXT("Moved map empty"), _Map.Num(), 0);
    TestTrue(TEXT("Tracker has value"), Tracker.GetValue().Contains(1));

    TestTrue(TEXT("IsChanged"), Tracker.IsChanged());
    TestTrue(TEXT("Add added"), Tracker.GetAddedElements().Contains(1));
    TestTrue(TEXT("No changed"), Tracker.GetChangedElements().IsEmpty());
    TestTrue(TEXT("No removed"), Tracker.GetRemovedElements().IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_Callback,
    "ChangeTrackingSystem.ChangeTracker.Map.Callback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_Callback::RunTest(const FString&)
{
    FMapTracker Tracker;

    int32 CallCount = 0;
    Tracker.OnMarkChanged = [&]()
    {
        ++CallCount;
    };

    Tracker.Add(1, 10);
    Tracker.Add(2, 20);

    TestTrue(TEXT("IsChanged"), Tracker.IsChanged());
    TestEqual(TEXT("Only once"), CallCount, 1);

    Tracker.ResetChangedState();
    Tracker.Add(3, 30);

    TestTrue(TEXT("IsChanged"), Tracker.IsChanged());
    TestEqual(TEXT("Second call"), CallCount, 2);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_Property, 
    "ChangeTrackingSystem.ChangeTracker.Map.Property", 
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_Property::RunTest(const FString&)
{
    FRandomStream Rand(42);

    for (int32 Iter = 0; Iter < 300; ++Iter)
    {
        FMapTracker Tracker;

        for (int32 Step = 0; Step < 100; ++Step)
        {
            int32 K = Rand.RandRange(0, 10);

            switch (Rand.RandRange(0, 2))
            {
                case 0: Tracker.Add(K, Rand.RandRange(0, 1000)); break;
                case 1: Tracker.Set(K, Rand.RandRange(0, 1000)); break;
                case 2: Tracker.Remove(K); break;
            }

            TestTrue(TEXT("Disjoint"), CheckDisjoint(Tracker));
            TestTrue(TEXT("Added valid"), CheckAddedValid(Tracker));
            TestTrue(TEXT("Removed valid"), CheckRemovedValid(Tracker));
        }
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_Complex, 
    "ChangeTrackingSystem.ChangeTracker.Map.Complex", 
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_Complex::RunTest(const FString&)
{
    FRandomStream Rand(1337);

    for (int32 Iteration = 0; Iteration < 500; ++Iteration)
    {
        FMapTracker Tracker;

        for (int32 Step = 0; Step < 200; ++Step)
        {
            int32 K = Rand.RandRange(0, 20);
            int32 V = Rand.RandRange(0, 1000);

            switch (Rand.RandRange(0, 4))
            {
                case 0: Tracker.Add(K, V); break;
                case 1: Tracker.Set(K, V); break;
                case 2: Tracker.Remove(K); break;
                case 3: Tracker.Reset(); break;
                case 4:
                {
                    FIntMap Tmp;
                    Tmp.Add(K, V);
                    Tracker.Append(Tmp);
                    break;
                }
            }

            TestTrue(TEXT("Disjoint"), CheckDisjoint(Tracker));
        }
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerMapTest_DiffCorrectness, 
    "ChangeTrackingSystem.ChangeTracker.Map.DiffCorrectness", 
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerMapTest_DiffCorrectness::RunTest(const FString&)
{
    FRandomStream Rand(777);

    for (int32 Iter = 0; Iter < 200; ++Iter)
    {
        FMapTracker Tracker;
        FIntMap Base;

        for (int i = 0; i < 5; ++i)
        {
            int32 K = Rand.RandRange(0, 20);
            int32 V = Rand.RandRange(0, 100);
            Base.Add(K, V);
            Tracker.Add(K, V);
        }

        FIntMap Initial = Base;
        Tracker.ResetChangedState();
        

        for (int32 Step = 0; Step < 50; ++Step)
        {
            int32 K = Rand.RandRange(0, 20);
            int32 V = Rand.RandRange(0, 100);

            switch (Rand.RandRange(0, 2))
            {
                case 0: 
                    Tracker.Add(K, V); 
                    Base.Add(K, V);
                    break;
                case 1: 
                    Tracker.Remove(K); 
                    Base.Remove(K); 
                    break;
                case 2: 
                    Tracker.Set(K, V); 
                    if (Base.Contains(K)) Base[K] = V; 
                    break;
            }

            TestTrue(TEXT("Equal size"), Base.Num() == Tracker.GetValue().Num());
        }

        {
            TSet<int32> A, C, R;
            ComputeReferenceDiff(Base, Tracker.GetValue(), A, C, R);

            TestTrue(TEXT("Map match"), A.IsEmpty() && C.IsEmpty() && R.IsEmpty());
        }

        {
            TSet<int32> A, C, R;
            ComputeReferenceDiff(Initial, Tracker.GetValue(), A, C, R);

            TestTrue(TEXT("Added match"), A.Includes(Tracker.GetAddedElements()) && Tracker.GetAddedElements().Includes(A));
            TestTrue(TEXT("Changed match"), C.Includes(Tracker.GetChangedElements()) && Tracker.GetChangedElements().Includes(C));
            TestTrue(TEXT("Removed match"), R.Includes(Tracker.GetRemovedElements()) && Tracker.GetRemovedElements().Includes(R));
        }
    }

    return true;
}

#endif
