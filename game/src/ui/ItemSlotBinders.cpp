#include "ui/ItemSlotBinders.hpp"

#include "GameUI.hpp"
#include "Systems.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/InventorySystem.hpp"

#include "engine/Cursor.hpp"

namespace lq
{
    InventorySlotBinder::InventorySlotBinder(LeverUIEngine* _engine) : engine(_engine)
    {
    }

    void InventorySlotBinder::Bind(InventorySlot& slot) const
    {
        auto* slotPtr = &slot;

        engine->sys->engine.cursor->onSelectedActorChange.Subscribe(
            [slotPtr](entt::entity, entt::entity current) { slotPtr->SetOwner(current); });

        engine->sys->inventorySystem->onInventoryUpdated.Subscribe([slotPtr]() { slotPtr->RetrieveInfo(); });
    }

    EquipmentSlotBinder::EquipmentSlotBinder(LeverUIEngine* _engine) : engine(_engine)
    {
    }

    void EquipmentSlotBinder::Bind(EquipmentSlot& slot) const
    {
        auto* slotPtr = &slot;

        engine->sys->engine.cursor->onSelectedActorChange.Subscribe(
            [slotPtr](entt::entity, entt::entity) { slotPtr->RetrieveInfo(); });

        engine->sys->equipmentSystem->onEquipmentUpdated.Subscribe(
            [slotPtr](entt::entity) { slotPtr->RetrieveInfo(); });
    }
} // namespace lq
