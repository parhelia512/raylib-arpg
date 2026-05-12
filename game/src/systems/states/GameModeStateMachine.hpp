//
// Created by Steve Wheeler on 31/07/2024.
//

#pragma once

#include "components/States.hpp"
#include "engine/systems/states/StateMachineBase.hpp"

#include "entt/entt.hpp"

#include <utility>

namespace lq
{
    class GameModeStateMachine final : public sage::StateMachineBase<GameModeStateMachine, GameState>
    {
        using Base = sage::StateMachineBase<GameModeStateMachine, GameState>;
        friend Base;

        entt::entity gameEntity;

        // ===== Default =====
        void onEnter(GameDefaultState&, entt::entity)
        {
        }
        void onExit(GameDefaultState&, entt::entity)
        {
        }
        void update(GameDefaultState&, entt::entity)
        {
        }

        // ===== Wave =====
        void onEnter(GameWaveState&, entt::entity entity);
        void onExit(GameWaveState&, entt::entity)
        {
        }
        void update(GameWaveState&, entt::entity)
        {
        }

        // ===== Combat =====
        void onEnter(GameCombatState&, entt::entity entity);
        void onExit(GameCombatState&, entt::entity)
        {
        }
        void update(GameCombatState&, entt::entity)
        {
        }

      public:
        // No-entity convenience overload — the game state machine has a single, internally
        // owned entity. Hides Base::ChangeState(entity, NewState) inside this class scope,
        // which is intentional: callers should never need to know about gameEntity.
        template <typename NewState>
        void ChangeState(NewState newState = {})
        {
            Base::ChangeState(gameEntity, std::move(newState));
        }

        void StartCombat();
        void Update();
        void Draw3D();

        ~GameModeStateMachine() = default;
        explicit GameModeStateMachine(entt::registry* _registry);
    };

} // namespace lq
