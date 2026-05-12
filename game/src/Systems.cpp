//
// Created by Steve Wheeler on 27/03/2024.
//

#include "Systems.hpp"

#include "collision/RpgCollisionLayers.hpp"
#include "ui/GameUI.hpp"

#include "engine/Camera.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/NavigationGridSquare.hpp"
#include "system_includes.hpp"

namespace lq
{
    Systems::Systems(
        entt::registry* _registry,
        sage::KeyMapping* _keyMapping,
        sage::Settings* _settings,
        sage::AudioManager* _audioManager)
        : engine(_registry, _keyMapping, _settings, _audioManager),
          selectionSystem(std::make_unique<SelectionSystem>()),
          cursorClickIndicator(std::make_unique<CursorClickIndicator>(_registry, this)),
          doorSystem(std::make_unique<DoorSystem>(_registry, &engine)),
          dialogSystem(std::make_unique<DialogSystem>(_registry, this)),
          dialogFactory(std::make_unique<DialogFactory>(_registry, this)),
          npcManager(std::make_unique<NPCManager>(_registry, this)),
          healthBarSystem(std::make_unique<HealthBarSystem>(_registry, engine.camera.get())),
          stateMachines(std::make_unique<StateMachines>(_registry, this)),
          abilityFactory(std::make_unique<AbilityFactory>(_registry, this)),
          itemFactory(std::make_unique<ItemFactory>(_registry)),
          playerAbilitySystem(std::make_unique<PlayerAbilitySystem>(_registry, this)),
          combatSystem(std::make_unique<CombatSystem>(_registry)),
          inventorySystem(std::make_unique<InventorySystem>(_registry, this)),
          partySystem(std::make_unique<PartySystem>(_registry, this)),
          equipmentSystem(std::make_unique<EquipmentSystem>(_registry, this)),
          controllableActorSystem(std::make_unique<ControllableActorSystem>(_registry, this)),
          questManager(std::make_unique<QuestManager>(_registry, this)),
          contextualDialogSystem(std::make_unique<ContextualDialogSystem>(_registry, this)),
          lootTable(std::make_unique<LootTable>(_registry, this)),
          lootSystem(std::make_unique<LootSystem>(_registry, this))
    {
        engine.ReplaceUiEngine(std::make_unique<LeverUIEngine>(_registry, this));
        selectionSystem->onSelectedActorChange.Subscribe([this](entt::entity prev, entt::entity current) {
            cursorClickIndicator->OnSelectedActorChanged(prev, current);
        });
        engine.collisionSystem->SetDefaultQueryMask(collision_masks::DefaultQuery);
        engine.cursor->SetNavigationRangeProvider([this, _registry](const Vector3& point) {
            const auto selectedActor = selectionSystem->GetSelectedActor();
            if (selectedActor == entt::null || !_registry->valid(selectedActor) ||
                !_registry->all_of<sage::MoveableActor>(selectedActor))
            {
                return true;
            }

            const auto& moveable = _registry->get<sage::MoveableActor>(selectedActor);
            sage::GridSquare minRange{};
            sage::GridSquare maxRange{};
            if (!engine.navigationGridSystem->GetPathfindRange(
                    selectedActor, moveable.pathfindingBounds, minRange, maxRange))
            {
                return false;
            }

            return engine.navigationGridSystem->CheckWithinBounds(point, minRange, maxRange);
        });
        engine.cursor->SetNavigationValidityProvider([this, _registry](const Vector3& point) {
            const auto selectedActor = selectionSystem->GetSelectedActor();
            if (selectedActor == entt::null || !_registry->valid(selectedActor) ||
                !_registry->all_of<sage::MoveableActor>(selectedActor))
            {
                return true;
            }
            return engine.navigationGridSystem->IsValidMove(point, selectedActor);
        });
        engine.cursor->SetCursorHoverMask(collision_masks::CursorHover);
        engine.cursor->SetCursorTexture(collision_layers::Npc, "cursor_talk");
        engine.cursor->SetCursorTexture(collision_layers::Enemy, "cursor_attack");
        engine.cursor->SetCursorTexture(collision_layers::Item, "cursor_pickup");
        engine.cursor->SetCursorTexture(collision_layers::Chest, "cursor_pickup");
        engine.cursor->SetCursorTexture(collision_layers::Interactable, "cursor_interact");
        engine.cursor->SetCursorTexture(collision_layers::Building, "cursor_denied");
    }

    LeverUIEngine& Systems::UI()
    {
        return static_cast<LeverUIEngine&>(engine.UI());
    }

    const LeverUIEngine& Systems::UI() const
    {
        return static_cast<const LeverUIEngine&>(engine.UI());
    }
} // namespace lq
