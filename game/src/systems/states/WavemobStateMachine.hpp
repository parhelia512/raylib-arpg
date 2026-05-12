// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "components/States.hpp"
#include "engine/systems/states/StateMachineBase.hpp"

#include "entt/entt.hpp"

#include <variant>

namespace lq
{
    class Systems;
    struct AttackData;

    class WavemobStateMachine final : public sage::StateMachineBase<WavemobStateMachine, WavemobState>
    {
        using Base = sage::StateMachineBase<WavemobStateMachine, WavemobState>;
        friend Base;

        // Dying is terminal — block all further transitions away from it.
        static bool isLocked(const WavemobState& s)
        {
            return std::holds_alternative<WavemobDyingState>(s.current);
        }

        Systems* sys;

        // ===== Default =====
        void onEnter(WavemobDefaultState&, entt::entity entity);
        void onExit(WavemobDefaultState&, entt::entity)
        {
        }
        void update(WavemobDefaultState&, entt::entity)
        {
        }

        // ===== TargetOutOfRange =====
        void onEnter(WavemobTargetOutOfRangeState&, entt::entity entity);
        void onExit(WavemobTargetOutOfRangeState&, entt::entity entity);
        void update(WavemobTargetOutOfRangeState&, entt::entity entity);

        // ===== Combat =====
        void onEnter(WavemobCombatState&, entt::entity entity);
        void onExit(WavemobCombatState&, entt::entity entity);
        void update(WavemobCombatState&, entt::entity entity);

        // ===== Dying =====
        void onEnter(WavemobDyingState&, entt::entity entity);
        void onExit(WavemobDyingState&, entt::entity)
        {
        }
        void update(WavemobDyingState&, entt::entity)
        {
        }

        void onHit(AttackData attackData);
        void onDeath(entt::entity entity);
        [[nodiscard]] bool isTargetOutOfSight(entt::entity entity) const;
        void onTargetPosUpdate(entt::entity entity, entt::entity target) const;
        void destroyEntity(entt::entity entity);

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity) const
        {
        }

      public:
        void Update();
        void Draw3D();

        ~WavemobStateMachine() = default;
        WavemobStateMachine(entt::registry* _registry, Systems* _sys);
    };
} // namespace lq
