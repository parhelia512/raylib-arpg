#include "ui/PlayerAbilitySlotBinder.hpp"

#include "components/Ability.hpp"
#include "GameUI.hpp"
#include "GameUiFactory.hpp"
#include "Systems.hpp"
#include "systems/PlayerAbilitySystem.hpp"

#include "engine/Cursor.hpp"
#include "engine/ResourceManager.hpp"

namespace lq
{
    PlayerAbilitySlotBinder::PlayerAbilitySlotBinder(
        LeverUIEngine* _engine, PlayerAbilitySystem* _playerAbilitySystem, Texture _emptySlotTex)
        : engine(_engine), playerAbilitySystem(_playerAbilitySystem), emptySlotTex(_emptySlotTex)
    {
    }

    void PlayerAbilitySlotBinder::Bind(AbilitySlot& slot, const unsigned int slotNumber) const
    {
        auto* uiEngine = engine;
        auto* abilities = playerAbilitySystem;
        auto* slotPtr = &slot;
        const auto fallbackTex = emptySlotTex;

        slot.iconProvider = [abilities, slotNumber, fallbackTex]() {
            if (const auto* ability = abilities->GetAbility(slotNumber))
            {
                return sage::ResourceManager::GetInstance().TextureLoad(ability->icon);
            }

            return fallbackTex;
        };
        slot.isInteractiveProvider = [abilities, slotNumber]() {
            return abilities->GetAbility(slotNumber) != nullptr;
        };
        slot.cooldownReadyProvider = [abilities, slotNumber]() {
            if (const auto* ability = abilities->GetAbility(slotNumber))
            {
                return ability->CooldownReady();
            }

            return true;
        };
        slot.tooltipFactory = [uiEngine, abilities, slotNumber](Vector2 pos) -> sage::TooltipWindow* {
            if (const auto* ability = abilities->GetAbility(slotNumber))
            {
                return GameUiFactory::CreateAbilityToolTip(uiEngine, *ability, pos);
            }

            return nullptr;
        };

        slot.onClicked.Subscribe([abilities, slotNumber]() { abilities->PressAbility(slotNumber); });
        slot.onSwapRequested.Subscribe([abilities, slotPtr](AbilitySlot* dropped) {
            if (!dropped || dropped == slotPtr) return;

            abilities->SwapAbility(slotPtr->slotNumber, dropped->slotNumber);
            slotPtr->RetrieveInfo();
            dropped->RetrieveInfo();
        });

        uiEngine->sys->cursor->onSelectedActorChange.Subscribe(
            [slotPtr](entt::entity, entt::entity) { slotPtr->RetrieveInfo(); });

        slot.RetrieveInfo();
    }
} // namespace lq
