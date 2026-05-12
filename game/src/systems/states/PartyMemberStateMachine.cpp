#include "PartyMemberStateMachine.hpp"
#include "animation/RpgAnimationIds.hpp"

#include "StateMachines.hpp"
#include "Systems.hpp"
#include "components/PartyMemberComponent.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "engine/components/Animation.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Cursor.hpp"
#include "engine/systems/ActorMovementSystem.hpp"

#include "raylib.h"

#include <cassert>

static constexpr int FOLLOW_DISTANCE = 15;

namespace lq
{
    // ====== PartyMemberDefaultState =================================================

    void PartyMemberStateMachine::onEnter(PartyMemberDefaultState&, const entt::entity entity)
    {
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Idle);
    }

    // ====== PartyMemberFollowingLeaderState =========================================

    void PartyMemberStateMachine::onEnter(PartyMemberFollowingLeaderState&, const entt::entity entity)
    {
        auto& partyMember = registry->get<PartyMemberComponent>(entity);
        assert(partyMember.followTarget.has_value());
        const auto followTarget = partyMember.followTarget.value();

        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Run);

        auto& moveable = registry->get<sage::MoveableActor>(entity);
        moveable.movementCollisionTarget = followTarget;
        auto& target = registry->get<sage::MoveableActor>(followTarget);
        auto& state = registry->get<PartyMemberState>(entity);

        auto onTargetReached = [this](const entt::entity e) { ChangeState(e, PartyMemberDefaultState{}); };
        auto onMovementCancelled = [this](const entt::entity e) { ChangeState(e, PartyMemberDefaultState{}); };
        auto onDestinationUnreachable = [this](const entt::entity e, const Vector3 requestedPos) {
            ChangeState(
                e,
                PartyMemberDestinationUnreachableState{
                    .originalDestination = requestedPos, .timeStart = GetTime()});
        };

        state.BindSubscription(moveable.onDestinationReached.Subscribe(onTargetReached));
        state.BindSubscription(target.onPathChanged.Subscribe(
            [this, entity](const entt::entity t) { onFollowingTargetPathChanged(entity, t); }));
        state.BindSubscription(moveable.onMovementCancel.Subscribe(onMovementCancelled));
        state.BindSubscription(moveable.onDestinationUnreachable.Subscribe(onDestinationUnreachable));

        onFollowingTargetPathChanged(entity, followTarget);
    }

    void PartyMemberStateMachine::onExit(PartyMemberFollowingLeaderState&, const entt::entity entity)
    {
        auto& moveable = registry->get<sage::MoveableActor>(entity);
        moveable.movementCollisionTarget.reset();
        sys->engine.actorMovementSystem->CancelMovement(entity);
    }

    void PartyMemberStateMachine::update(PartyMemberFollowingLeaderState&, const entt::entity entity)
    {
        const auto& partyMember = registry->get<PartyMemberComponent>(entity);
        assert(partyMember.followTarget.has_value());
        const auto& transform = registry->get<sage::sgTransform>(entity);
        const auto& followTrans = registry->get<sage::sgTransform>(partyMember.followTarget.value());
        const auto& followMoveable = registry->get<sage::MoveableActor>(partyMember.followTarget.value());

        // If we are closer to our destination than the leader is, then wait.
        if (followMoveable.IsMoving() &&
            Vector3Distance(followTrans.GetWorldPos(), followMoveable.path.back()) + FOLLOW_DISTANCE >
                Vector3Distance(transform.GetWorldPos(), followMoveable.path.back()))
        {
            ChangeState(entity, PartyMemberWaitingForLeaderState{});
        }
    }

    void PartyMemberStateMachine::onFollowingTargetPathChanged(
        const entt::entity entity, const entt::entity target)
    {
        const auto& trans = registry->get<sage::sgTransform>(entity);
        const auto& targetTrans = registry->get<sage::sgTransform>(target);
        const auto& targetMoveable = registry->get<sage::MoveableActor>(target);
        auto dest = targetMoveable.IsMoving() ? targetMoveable.GetDestination() : targetTrans.GetWorldPos();
        const auto dir = Vector3Normalize(Vector3Subtract(dest, trans.GetWorldPos()));
        dest = Vector3Subtract(dest, sage::Vector3MultiplyByValue(dir, FOLLOW_DISTANCE));
        sys->engine.actorMovementSystem->PathfindToLocation(entity, dest, true);
    }

    // ====== PartyMemberWaitingForLeaderState ========================================

    void PartyMemberStateMachine::onEnter(PartyMemberWaitingForLeaderState&, const entt::entity entity)
    {
        const auto& partyMember = registry->get<PartyMemberComponent>(entity);
        assert(partyMember.followTarget.has_value());
        auto& moveable = registry->get<sage::MoveableActor>(entity);
        auto& state = registry->get<PartyMemberState>(entity);

        auto onMovementCancelled = [this](const entt::entity e) { ChangeState(e, PartyMemberDefaultState{}); };

        state.BindSubscription(moveable.onMovementCancel.Subscribe(onMovementCancelled));
        state.BindSubscription(sys->selectionSystem->onSelectedActorChange.Subscribe(
            [onMovementCancelled](entt::entity, const entt::entity e) { onMovementCancelled(e); }));

        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Idle);
    }

    void PartyMemberStateMachine::onExit(PartyMemberWaitingForLeaderState&, const entt::entity entity)
    {
        registry->get<sage::MoveableActor>(entity).movementCollisionTarget.reset();
    }

    void PartyMemberStateMachine::update(PartyMemberWaitingForLeaderState&, const entt::entity entity)
    {
        if (entity == sys->selectionSystem->GetSelectedActor())
        {
            ChangeState(entity, PartyMemberDefaultState{});
            return;
        }

        const auto& partyMember = registry->get<PartyMemberComponent>(entity);
        assert(partyMember.followTarget.has_value());
        const auto& transform = registry->get<sage::sgTransform>(entity);
        const auto& followTrans = registry->get<sage::sgTransform>(partyMember.followTarget.value());
        const auto& followMoveable = registry->get<sage::MoveableActor>(partyMember.followTarget.value());

        // Follow target is now closer to its destination than we are, so we can proceed.
        if (followMoveable.IsMoving() &&
            Vector3Distance(followTrans.GetWorldPos(), followMoveable.path.back()) + FOLLOW_DISTANCE <
                Vector3Distance(transform.GetWorldPos(), followMoveable.path.back()))
        {
            ChangeState(entity, PartyMemberFollowingLeaderState{});
        }
    }

    // ====== PartyMemberDestinationUnreachableState ==================================

    static constexpr float RETRY_TIME_THRESHOLD = 1.5f;
    static constexpr unsigned int MAX_TRIES = 10;

    void PartyMemberStateMachine::onEnter(PartyMemberDestinationUnreachableState&, const entt::entity entity)
    {
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Idle);
    }

    void PartyMemberStateMachine::update(
        PartyMemberDestinationUnreachableState& s, const entt::entity entity)
    {
        auto& moveable = registry->get<sage::MoveableActor>(entity);
        if (moveable.IsMoving()) return;

        if (s.tryCount >= MAX_TRIES)
        {
            moveable.movementCollisionTarget.reset();
            sys->engine.actorMovementSystem->CancelMovement(entity);
            ChangeState(entity, PartyMemberDefaultState{});
            return;
        }

        if (GetTime() < s.timeStart + RETRY_TIME_THRESHOLD) return;

        ++s.tryCount;
        s.timeStart = GetTime();
        if (sys->engine.actorMovementSystem->TryPathfindToLocation(entity, s.originalDestination, true))
        {
            ChangeState(entity, PartyMemberFollowingLeaderState{});
            return;
        }

        // If the leader is too far, we could maybe follow a party member who is closer to the
        // destination and also moving
        // TODO: Bug. Weird bug that going to the other player's path can make us walk past the player
        const auto& target = registry->get<PartyMemberComponent>(entity).followTarget;
        assert(target.has_value());
        const auto leaderPos = registry->get<sage::sgTransform>(target.value()).GetWorldPos();
        if (sys->engine.actorMovementSystem->TryPathfindToLocation(entity, leaderPos, true))
        {
            s.tryCount = 0;
        }
    }

    // ====== Cross-state handlers ====================================================

    void PartyMemberStateMachine::onLeaderMove(const entt::entity entity)
    {
        sys->engine.actorMovementSystem->CancelMovement(entity);
        ChangeState(entity, PartyMemberFollowingLeaderState{});
    }

    // ====== Lifecycle ===============================================================

    void PartyMemberStateMachine::Update()
    {
        for (const auto view = registry->view<PartyMemberState>(); const auto& entity : view)
        {
            assert(!registry->any_of<PlayerState>(entity));
            auto& state = registry->get<PartyMemberState>(entity);
            std::visit([this, entity](auto& cur) { update(cur, entity); }, state.current);
        }
    }

    void PartyMemberStateMachine::Draw3D()
    {
    }

    void PartyMemberStateMachine::onComponentAdded(const entt::entity entity)
    {
        auto& partyMember = registry->get<PartyMemberComponent>(entity);
        partyMember.followTarget = sys->selectionSystem->GetSelectedActor();
        auto& target = registry->get<sage::MoveableActor>(partyMember.followTarget.value());
        target.onStartMovement.Subscribe([this, entity](entt::entity) { onLeaderMove(entity); });

        auto& state = registry->get<PartyMemberState>(entity);
        std::visit([this, entity](auto& cur) { onEnter(cur, entity); }, state.current);
    }

    PartyMemberStateMachine::PartyMemberStateMachine(entt::registry* _registry, Systems* _sys)
        : Base(_registry), sys(_sys)
    {
        registry->on_construct<PartyMemberState>().connect<&PartyMemberStateMachine::onComponentAdded>(this);
        registry->on_destroy<PartyMemberState>().connect<&PartyMemberStateMachine::onComponentRemoved>(this);
    }
} // namespace lq
