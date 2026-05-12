// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "components/States.hpp"
#include "engine/systems/states/StateMachineBase.hpp"

#include "entt/entt.hpp"

namespace lq
{
    class Systems;

    class PartyMemberStateMachine final
        : public sage::StateMachineBase<PartyMemberStateMachine, PartyMemberState>
    {
        using Base = sage::StateMachineBase<PartyMemberStateMachine, PartyMemberState>;
        friend Base;

        Systems* sys;

        // ===== Default =====
        void onEnter(PartyMemberDefaultState&, entt::entity entity);
        void onExit(PartyMemberDefaultState&, entt::entity)
        {
        }
        void update(PartyMemberDefaultState&, entt::entity)
        {
        }

        // ===== FollowingLeader =====
        void onEnter(PartyMemberFollowingLeaderState&, entt::entity entity);
        void onExit(PartyMemberFollowingLeaderState&, entt::entity entity);
        void update(PartyMemberFollowingLeaderState&, entt::entity entity);

        // ===== WaitingForLeader =====
        void onEnter(PartyMemberWaitingForLeaderState&, entt::entity entity);
        void onExit(PartyMemberWaitingForLeaderState&, entt::entity entity);
        void update(PartyMemberWaitingForLeaderState&, entt::entity entity);

        // ===== DestinationUnreachable =====
        void onEnter(PartyMemberDestinationUnreachableState& s, entt::entity entity);
        void onExit(PartyMemberDestinationUnreachableState&, entt::entity)
        {
        }
        void update(PartyMemberDestinationUnreachableState& s, entt::entity entity);

        void onLeaderMove(entt::entity entity);
        void onFollowingTargetPathChanged(entt::entity entity, entt::entity target);

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity) const
        {
        }

      public:
        void Update();
        void Draw3D();

        ~PartyMemberStateMachine() = default;
        PartyMemberStateMachine(entt::registry* _registry, Systems* sys);
    };
} // namespace lq
