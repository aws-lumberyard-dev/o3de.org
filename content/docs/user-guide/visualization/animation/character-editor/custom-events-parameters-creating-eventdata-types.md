---
description: ' Use EventData types to create custom parameters for motion events in
  Open 3D Engine. '
title: Creating EventData Types
---

You create objects deriving from the [`EMotionFX::EventData`](/docs/api/gems/emotionfx/) interface to attach an arbitrary number of data objects to a single event.

**To create an EventData type**

1. Using the following criteria, subclass either the `EventData` class or the `EventDataSyncable` class:
   + For general-purpose parameters, subclass the `EventData` type.
   + For parameters for the sync track, use the `EventDataSyncable` class. These parameters are for synchronizing blended motions.

   The following code snippet subclasses the `EventDataSyncable` class to create a `LeftFootEvent` for a left footstep.

   ```
   class LeftFootEvent final
       : public EMotionFX::EventDataSyncable
   {
   public:
       AZ_RTTI(LeftFootEvent, "{117454DC-0675-483E-843E-841C57A4354D}", EventDataSyncable);

       LeftFootEvent() = default;
   ...
   ```

1. Implement the `Equal` function in the subclass. The `Equal` function tests whether two `EventData` instances are equal and is used for deduplication of `EventData` instances.

   The following example checks whether the `EventData` passed in is a `LeftFootEvent`.

   ```
   bool Equal(const EventData& rhs, const bool /*ignoreEmptyFields*/) const override
   {
       // All LeftFootEvents are equal.
       const LeftFootEvent* rhsEvent = azrtti_cast<const LeftFootEvent*>(&rhs);
       return rhsEvent != nullptr;
   }
   ```

For more information about the `Equal` function, refer to [More About the Equal Function](#more-about-the-equal-function).

1. Implement the `Reflect` method to reflect the type to the [serialization context](/docs/user-guide/programming/components/reflection/serialization-context/) and [edit context](/docs/user-guide/programming/components/reflection/edit-context/).

When you reflect the event to the edit context, add the `Creatable` attribute to `ClassElement`. This makes the `EventData` type visible in **Animation Editor** in the **Motion Events** tab so that users can select it.

The following code example reflects the `LeftFootEvent` to the serialization and edit contexts.

   ```
   ...
   static void Reflect(AZ::ReflectContext* context)
   {
       AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
       if (!serializeContext) return;

       serializeContext->Class<LeftFootEvent, EventDataSyncable>()->Version(1);

       AZ::EditContext* editContext = serializeContext->GetEditContext();
       if (!editContext) return;

       editContext->Class<LeftFootEvent>("LeftFootEvent", "")
           ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
               ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
               ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
               ->Attribute(AZ_CRC("Creatable", 0x47bff8c4), true);
   }
   ...
   ```

1. To add the event to a motion, use the `FindOrCreateEventData` template, which accepts any subclass of `EventData`.

   ```
   ...
   auto footstepData = GetEMotionFX().GetEventManager()->FindOrCreateEventData<LeftFootEvent>();
   motion->GetEventTable()->FindTrackByName("Sound")->AddEvent(0.2f, footstepData);
   ...
   ```

## Example 

The following example shows the completed `LeftFootEvent` sample `EventData` subclass and the code to add the event to a motion.

```
class LeftFootEvent final
    : public EMotionFX::EventDataSyncable
{
public:
    AZ_RTTI(LeftFootEvent, "{117454DC-0675-483E-843E-841C57A4354D}", EventDataSyncable);

    LeftFootEvent() = default;

    static void Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (!serializeContext) return;

        serializeContext->Class<LeftFootEvent, EventDataSyncable>()->Version(1);

        AZ::EditContext* editContext = serializeContext->GetEditContext();
        if (!editContext) return;

        editContext->Class<LeftFootEvent>("LeftFootEvent", "")
            ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                ->Attribute(AZ_CRC("Creatable", 0x47bff8c4), true);
    }

    bool Equal(const EventData& rhs, const bool /*ignoreEmptyFields*/) const override
    {
        // all LeftFootEvents are equal
        const LeftFootEvent* rhsEvent = azrtti_cast<const LeftFootEvent*>(&rhs);
        return rhsEvent != nullptr;
    }

    size_t HashForSyncing(bool isMirror) const override { return isMirror ? 1 : 0; }

protected:
    LeftFootEvent(const size_t /*hash*/) : LeftFootEvent() {}
};

auto footstepData = GetEMotionFX().GetEventManager()->FindOrCreateEventData<LeftFootEvent>();
motion->GetEventTable()->FindTrackByName("Sound")->AddEvent(0.2f, footstepData);
```

## More About the Equal Function 

The `Equal` function tests whether two `EventData` instances are equal.

**Syntax**

```
virtual bool Equal(const EventData& rhs, bool ignoreEmptyFields = false) const = 0;
```

EMotion FX uses the `Equal` method to deduplicate instances of `EventData` subclasses. The `AnimGraphMotionCondition` class also uses it for motion event matching logic.

When Event Manager loads a `.motion` file and deserializes the motion events on the event tracks, `EventManager::FindOrCreateEventData` processes each `EventData` instance.

The `EventManager` stores a list of the `EventData` instances in use and attempts to find an `EventData` instance in which the call to `Equal(loadedEventData)` returns true. If the `EventManager` finds an `EventData` instance that is equal, the duplicate data is discarded.

When a call to `AnimGraphMotionCondition` tests a motion event, `AnimGraphMotionCondition::TestCondition` calls the `Equal` method with the `ignoreEmptyFields` parameter set to `true`. The `ignoreEmptyFields` parameter enables partial matching of `EventData` instances. For example, if one of the fields is a string and the string value is empty in the condition, any value in the field matches.

## Synchronizing Blended Motions

The `EMotionFX::EventDataSyncable` class extends the functionality of the base `EventData` class and enables events that drive motion synchronization behavior. Use the `EventDataSyncable` class to specify parameters for synchronizing blended motions. The class calls `HashForSyncing` on the sync tracks of two different motions, compares the results, and finds events that are equal based on their hash value.

### Mirroring

You can use `EMotionFX` to mirror motions programmatically. When a motion is being mirrored, its sync events must also be mirrored. To signal this mirroring, the `HashForSyncing` method accepts an `isMirror` parameter.

For example, suppose that you use an `EventDataSyncable` subclass to mirror the gait of a horse. You use an integer field to represent the feet of the horse with the following convention.

```
0=left rear
1=right rear
2=left front
3=right front
```

Implement `HashForSyncing` as in the following example.

```
size_t HashForSyncing(bool isMirror) const
{
    if (!isMirror)
    {
        return m_footIndex;
    }
    // Translate left foot (an even foot index) to right foot, and vice
    // versa
    return (m_footIndex % 2 == 0) ? m_footIndex + 1 : m_footIndex - 1;
}
```

The default implementation for `HashForSyncing` returns the hash of the type ID of the type and ignores the `isMirror` parameter.
