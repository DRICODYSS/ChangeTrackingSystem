// Copyright DRICODYSS. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChangeTrackerPtr.h"
#include "TestChangeTrackerObject.h"
#include "TestChangeTrackerStruct.h"

using FPtrTracker = TChangeTracker<UTestChangeTrackerObject*>;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerPointerTest_Init,
    "ChangeTrackingSystem.ChangeTracker.Pointer.Init",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPointerTest_Init::RunTest(const FString&)
{
    FPtrTracker Tracker{};

    TestFalse(TEXT("Initially not changed"), Tracker.IsChanged());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerPointerTest_FieldPropagation,
    "ChangeTrackingSystem.ChangeTracker.Pointer.FieldPropagation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPointerTest_FieldPropagation::RunTest(const FString&)
{
    auto Obj = NewObject<UTestChangeTrackerObject>();
    FPtrTracker Tracker = Obj;

    Tracker.GetValue()->ValueA = 42;

    TestTrue(TEXT("Parent marked changed"), Tracker.IsChanged());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerPointerTest_MultipleFields,
    "ChangeTrackingSystem.ChangeTracker.Pointer.MultipleFields",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPointerTest_MultipleFields::RunTest(const FString&)
{
    const auto Obj = NewObject<UTestChangeTrackerObject>();
    FPtrTracker Tracker(Obj);

    Obj->ValueA = 1;
    Tracker.GetValue()->ValueB = 2;

    TestTrue(TEXT("Changed"), Tracker.IsChanged());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerPointerTest_ResetState,
    "ChangeTrackingSystem.ChangeTracker.Pointer.ResetState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPointerTest_ResetState::RunTest(const FString&)
{
    const auto Obj = NewObject<UTestChangeTrackerObject>();
    FPtrTracker Tracker(Obj);

    Obj->ValueA = 10;

    Tracker.ResetChangedState();

    TestFalse(TEXT("Tracker reset"), Tracker.IsChanged());
    TestFalse(TEXT("Field reset"), Obj->ValueA.IsChanged());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerPointerTest_Rebind,
    "ChangeTrackingSystem.ChangeTracker.Pointer.Rebind",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPointerTest_Rebind::RunTest(const FString&)
{
    const auto Obj = NewObject<UTestChangeTrackerObject>();
    FPtrTracker Tracker = NewObject<UTestChangeTrackerObject>();
    Tracker = Obj;

    TestTrue(TEXT("Set New object"), Tracker.IsChanged());

    Tracker.ResetChangedState();
    Obj->ValueA = 123;

    TestTrue(TEXT("New object tracked"), Tracker.IsChanged());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerPointerTest_UnbindOld,
    "ChangeTrackingSystem.ChangeTracker.Pointer.UnbindOld",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPointerTest_UnbindOld::RunTest(const FString&)
{
    const auto Obj1 = NewObject<UTestChangeTrackerObject>();
    const auto Obj2 = NewObject<UTestChangeTrackerObject>();

    FPtrTracker Tracker(Obj1);
    Tracker.ChangeValue(Obj2);
    Tracker.ResetChangedState();

    Obj1->ValueA = 50;

    TestFalse(TEXT("Old object does not trigger"), Tracker.IsChanged());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerPointerTest_Null,
    "ChangeTrackingSystem.ChangeTracker.Pointer.Null",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPointerTest_Null::RunTest(const FString&)
{
    FPtrTracker Tracker = nullptr;

    TestFalse(TEXT("No crash, no change"), Tracker.IsChanged());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerPointerTest_LazyInit,
    "ChangeTrackingSystem.ChangeTracker.Pointer.LazyInit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPointerTest_LazyInit::RunTest(const FString&)
{
    FPtrTracker Tracker;

    const auto Obj = NewObject<UTestChangeTrackerObject>();
    Tracker.ChangeValue(Obj);

    Obj->ValueA = 777;

    TestTrue(TEXT("Lazy init works"), Tracker.IsChanged());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerPointerTest_Callback,
    "ChangeTrackingSystem.ChangeTracker.Pointer.Callback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPointerTest_Callback::RunTest(const FString&)
{
    const auto Obj = NewObject<UTestChangeTrackerObject>();
    FPtrTracker Tracker(Obj);

    int32 CallCount = 0;
    Tracker.OnMarkChanged = [&]()
    {
        ++CallCount;
    };

    Tracker.GetValue()->ValueA = 1;
    Obj->ValueB = 2;

    TestEqual(TEXT("Called once"), CallCount, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FChangeTrackerPointerTest_PropertyDiff,
    "ChangeTrackingSystem.ChangeTracker.Pointer.PropertyDiff",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority
)

bool FChangeTrackerPointerTest_PropertyDiff::RunTest(const FString&)
{
    FRandomStream Rand(123);

    for (int32 Iter = 0; Iter < 100; ++Iter)
    {
        auto Obj = NewObject<UTestChangeTrackerObject>();
        FPtrTracker Tracker = Obj;

        int32 RefA = Tracker.GetValue()->ValueA;
        int32 RefB = Tracker.GetValue()->ValueB;

        for (int32 Step = 0; Step < 50; ++Step)
        {
            int32 NewValue = Rand.RandRange(0, 1000);
            bool bExpectedChange = false;

            switch (Rand.RandRange(0, 2))
            {
                case 0:
                    bExpectedChange = (Obj->ValueA.GetValue() != NewValue);
                    Obj->ValueA = NewValue;
                    RefA = NewValue;
                    break;

                case 1:
                    bExpectedChange = (Obj->ValueB.GetValue() != NewValue);
                    Obj->ValueB = NewValue;
                    RefB = NewValue;
                    break;

                case 2:
                {
                    const auto NewObj = NewObject<UTestChangeTrackerObject>();
                    NewObj->ValueA = RefA;
                    NewObj->ValueB = RefB;

                    Obj = NewObj;
                    Tracker.ChangeValue(Obj);
                    bExpectedChange = true;
                    break;
                }
            }

            TestEqual(TEXT("ValueA correct"), Obj->ValueA.GetValue(), RefA);
            TestEqual(TEXT("ValueB correct"), Obj->ValueB.GetValue(), RefB);

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
