// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "components/States.hpp"
#include "engine/systems/states/StateMachine.hpp"
#include "entt/entt.hpp"

namespace lq
{
    class Systems;

    struct DialogTargetPayload final : sage::StatePayload
    {
        entt::entity target = entt::null;

        explicit DialogTargetPayload(const entt::entity _target) : target(_target)
        {
        }
    };

    enum class PlayerInteractionKind
    {
        Talk,
        Loot
    };

    struct PlayerInteractionPayload final : sage::StatePayload
    {
        PlayerInteractionKind kind{};
        entt::entity target = entt::null;

        PlayerInteractionPayload(const PlayerInteractionKind _kind, const entt::entity _target)
            : kind(_kind), target(_target)
        {
        }
    };

    class PlayerStateMachine final : public sage::StateMachine<PlayerState, PlayerStateEnum>
    {
        class DefaultState;
        class MovingToLocationState;
        class MovingToAttackEnemyState;
        class MovingToInteractionTargetState;
        class InDialogState;
        class CombatState;

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

        void onFloorClick(entt::entity self, entt::entity);
        void onChestClick(entt::entity self, entt::entity);
        void onNPCLeftClick(entt::entity self, entt::entity target);
        void onEnemyLeftClick(entt::entity self, entt::entity target);

      public:
        void Update();
        void Draw3D();

        ~PlayerStateMachine() override = default;
        PlayerStateMachine(entt::registry* _registry, Systems* sys);

        friend class StateMachine; // Required for CRTP
    };
} // namespace lq
