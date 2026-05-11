#pragma once

#include "raylib.h"

namespace lq
{
    class AbilitySlot;
    class CharacterStatText;
    class EquipmentCharacterPreview;
    class EquipmentSlot;
    class InventorySlot;
    class LeverUIEngine;
    class PartyMemberPortrait;
    class PlayerAbilitySystem;

    // Binding is the setup step that connects passive UI widgets to RPG data and actions.
    // Widgets expose providers/events; these functions fill those hooks and subscribe refreshes,
    // so the widgets can render/update without directly knowing about RPG systems.
    void BindPlayerAbilitySlot(
        LeverUIEngine* engine,
        PlayerAbilitySystem* playerAbilitySystem,
        Texture emptySlotTex,
        AbilitySlot& slot,
        unsigned int slotNumber);
    void BindInventorySlot(LeverUIEngine* engine, InventorySlot& slot, bool followSelectedActor = true);
    void BindEquipmentSlot(LeverUIEngine* engine, EquipmentSlot& slot);
    void BindCharacterStatText(LeverUIEngine* engine, CharacterStatText& statText);
    void BindEquipmentCharacterPreview(LeverUIEngine* engine, EquipmentCharacterPreview& preview);
    void BindPartyMemberPortrait(LeverUIEngine* engine, PartyMemberPortrait& portrait);
} // namespace lq
