// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "components/States.hpp"
#include "engine/systems/states/StateMachineBase.hpp"

#include "entt/entt.hpp"

namespace lq
{
    class Systems;

    class PlayerStateMachine final : public sage::StateMachineBase<PlayerStateMachine, PlayerState>
    {
        using Base = StateMachineBase<PlayerStateMachine, PlayerState>;
        friend Base;

        Systems* sys;

        // ===== Default =====
        void onEnter(PlayerDefaultState&, entt::entity entity);
        void onExit(PlayerDefaultState&, entt::entity)
        {
        }

        // ===== MovingToLocation =====
        void onEnter(PlayerMovingToLocationState&, entt::entity entity);
        void onExit(PlayerMovingToLocationState&, entt::entity)
        {
        }

        // ===== MovingToAttackEnemy =====
        void onEnter(PlayerMovingToAttackEnemyState&, entt::entity entity);
        void onExit(PlayerMovingToAttackEnemyState&, entt::entity)
        {
        }

        // ===== MovingToTalk =====
        void onEnter(PlayerMovingToTalkState&, entt::entity entity);
        void onExit(PlayerMovingToTalkState&, entt::entity entity);

        // ===== MovingToLoot =====
        void onEnter(PlayerMovingToLootState&, entt::entity entity);
        void onExit(PlayerMovingToLootState&, entt::entity)
        {
        }

        // ===== InDialog =====
        void onEnter(PlayerInDialogState&, entt::entity entity);
        void onExit(PlayerInDialogState&, entt::entity entity);

        // ===== Combat =====
        void onEnter(PlayerCombatState&, entt::entity entity);
        void onExit(PlayerCombatState&, entt::entity entity);

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);
        void bindCursorInput(entt::entity entity);

        void onFloorClick(entt::entity entity, entt::entity);
        void onChestClick(entt::entity entity, entt::entity target);
        void onNPCLeftClick(entt::entity entity, entt::entity target);
        void onEnemyLeftClick(entt::entity entity, entt::entity target);

      public:
        void Update();
        void Draw3D();

        ~PlayerStateMachine() = default;
        PlayerStateMachine(entt::registry* _registry, Systems* sys);
    };
} // namespace lq
