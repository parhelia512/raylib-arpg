#pragma once

#include "raylib.h"

namespace lq
{
    class AbilitySlot;
    class LeverUIEngine;
    class PlayerAbilitySystem;

    class PlayerAbilitySlotBinder
    {
        LeverUIEngine* engine;
        PlayerAbilitySystem* playerAbilitySystem;
        Texture emptySlotTex{};

      public:
        PlayerAbilitySlotBinder(
            LeverUIEngine* _engine, PlayerAbilitySystem* _playerAbilitySystem, Texture _emptySlotTex);

        void Bind(AbilitySlot& slot, unsigned int slotNumber) const;
    };
} // namespace lq
