#pragma once

namespace lq
{
    class EquipmentSlot;
    class InventorySlot;
    class LeverUIEngine;

    class InventorySlotBinder
    {
        LeverUIEngine* engine;

      public:
        explicit InventorySlotBinder(LeverUIEngine* _engine);

        void Bind(InventorySlot& slot) const;
    };

    class EquipmentSlotBinder
    {
        LeverUIEngine* engine;

      public:
        explicit EquipmentSlotBinder(LeverUIEngine* _engine);

        void Bind(EquipmentSlot& slot) const;
    };
} // namespace lq
