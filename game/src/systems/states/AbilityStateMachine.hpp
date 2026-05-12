#pragma once

#include "components/States.hpp"
#include "engine/systems/states/StateMachineBase.hpp"

#include "entt/entt.hpp"

namespace lq
{
    class Systems;

    class AbilityStateMachine final : public sage::StateMachineBase<AbilityStateMachine, AbilityState>
    {
        using Base = sage::StateMachineBase<AbilityStateMachine, AbilityState>;
        friend Base;

        Systems* sys;

        // ===== Idle =====
        void onEnter(AbilityIdleState&, entt::entity)
        {
        }
        void onExit(AbilityIdleState&, entt::entity)
        {
        }
        void update(AbilityIdleState&, entt::entity entity);

        // ===== CursorSelect =====
        void onEnter(AbilityCursorSelectState&, entt::entity entity);
        void onExit(AbilityCursorSelectState&, entt::entity entity);
        void update(AbilityCursorSelectState&, entt::entity entity);

        // ===== AwaitingExecution =====
        // TODO: I think this should be split into two states depending on whether its detached or not
        // Or maybe if it has a cast time or not...
        void onEnter(AbilityAwaitingExecutionState&, entt::entity entity);
        void onExit(AbilityAwaitingExecutionState&, entt::entity)
        {
        }
        void update(AbilityAwaitingExecutionState&, entt::entity entity);

        void enableCursor(entt::entity entity);
        void disableCursor(entt::entity entity);

        void startCast(entt::entity entity);
        void cancelCast(entt::entity entity);
        void spawnAbility(entt::entity entity);
        void executeAbility(entt::entity entity);
        [[nodiscard]] bool checkRange(entt::entity entity) const;

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity) const
        {
        }

      public:
        void Update();
        void Draw3D();

        ~AbilityStateMachine() = default;
        AbilityStateMachine(const AbilityStateMachine&) = delete;
        AbilityStateMachine& operator=(const AbilityStateMachine&) = delete;
        AbilityStateMachine(entt::registry* _registry, Systems* _sys);
    };
} // namespace lq
