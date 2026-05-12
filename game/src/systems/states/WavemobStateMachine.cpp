#include "WavemobStateMachine.hpp"
#include "animation/RpgAnimationIds.hpp"

#include "Systems.hpp"

#include "AbilityFactory.hpp"
#include "collision/RpgCollisionLayers.hpp"
#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"

#include "engine/components/Animation.hpp"
#include "engine/components/DeleteEntityComponent.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/systems/ActorMovementSystem.hpp"
#include "engine/systems/CollisionSystem.hpp"
#include "engine/systems/NavigationGridSystem.hpp"

#include "raylib.h"

namespace lq
{
    // ====== WavemobDefaultState =====================================================

    void WavemobStateMachine::onEnter(WavemobDefaultState&, const entt::entity entity)
    {
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Idle);
    }

    // ====== WavemobTargetOutOfRangeState ============================================

    void WavemobStateMachine::onEnter(WavemobTargetOutOfRangeState&, const entt::entity entity)
    {
        const auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::ENEMY_AUTOATTACK);
        registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);

        auto& moveable = registry->get<sage::MoveableActor>(entity);
        const auto& combatable = registry->get<CombatableActor>(entity);
        moveable.movementCollisionTarget = combatable.target;
        auto& target = registry->get<sage::MoveableActor>(combatable.target);
        auto& state = registry->get<WavemobState>(entity);

        auto onTargetReached = [this](const entt::entity e) { ChangeState(e, WavemobCombatState{}); };

        state.BindSubscription(target.onPathChanged.Subscribe(
            [this, entity](const entt::entity t) { onTargetPosUpdate(entity, t); }));
        state.BindSubscription(moveable.onDestinationReached.Subscribe(onTargetReached));

        onTargetPosUpdate(entity, combatable.target);
    }

    void WavemobStateMachine::onExit(WavemobTargetOutOfRangeState&, const entt::entity entity)
    {
        registry->get<sage::MoveableActor>(entity).movementCollisionTarget.reset();
    }

    void WavemobStateMachine::update(WavemobTargetOutOfRangeState&, const entt::entity entity)
    {
        const auto& combatable = registry->get<CombatableActor>(entity);
        if (combatable.target == entt::null || isTargetOutOfSight(entity))
        {
            ChangeState(entity, WavemobDefaultState{});
        }
    }

    bool WavemobStateMachine::isTargetOutOfSight(const entt::entity entity) const
    {
        auto& combatable = registry->get<CombatableActor>(entity);
        auto& trans = registry->get<sage::sgTransform>(entity);
        auto& collideable = registry->get<sage::Collideable>(entity);

        const auto& targetPos = registry->get<sage::sgTransform>(combatable.target).GetWorldPos();
        Vector3 direction = Vector3Subtract(targetPos, trans.GetWorldPos());
        const float distance = Vector3Distance(trans.GetWorldPos(), targetPos);
        const Vector3 normDirection = Vector3Normalize(direction);

        Ray ray;
        ray.position = trans.GetWorldPos();
        ray.direction = Vector3Scale(normDirection, distance);
        const float height = Vector3Subtract(collideable.localBoundingBox.max, collideable.localBoundingBox.min).y;
        ray.position.y = trans.GetWorldPos().y + height;
        ray.direction.y = trans.GetWorldPos().y + height;
        trans.movementDirectionDebugLine = ray;

        const auto collisions =
            sys->engine.collisionSystem->GetCollisionsWithRay(entity, ray, collideable.collidesWith);

        if (!collisions.empty() && collisions.at(0).collisionLayer != lq::collision_layers::Player)
        {
            // Lost line of sight, out of combat
            combatable.target = entt::null;
            trans.movementDirectionDebugLine = {};
            return true;
        }
        return false;
    }

    void WavemobStateMachine::onTargetPosUpdate(const entt::entity entity, const entt::entity target) const
    {
        const auto& targetPos = registry->get<sage::sgTransform>(target).GetWorldPos();
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Walk, 2);
        sys->engine.actorMovementSystem->PathfindToLocation(entity, targetPos);
    }

    // ====== WavemobCombatState ======================================================

    void WavemobStateMachine::onEnter(WavemobCombatState&, const entt::entity entity)
    {
        const auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::ENEMY_AUTOATTACK);
        registry->get<Ability>(abilityEntity).startCast.Publish(abilityEntity);
    }

    void WavemobStateMachine::onExit(WavemobCombatState&, const entt::entity entity)
    {
        const auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::ENEMY_AUTOATTACK);
        registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);
    }

    void WavemobStateMachine::update(WavemobCombatState&, const entt::entity entity)
    {
        const auto& combatable = registry->get<CombatableActor>(entity);
        if (combatable.dying || combatable.target == entt::null)
        {
            ChangeState(entity, WavemobDefaultState{});
            return;
        }
        const auto& actorTrans = registry->get<sage::sgTransform>(entity);
        const auto target = registry->get<sage::sgTransform>(combatable.target).GetWorldPos();
        const float distance = Vector3Distance(actorTrans.GetWorldPos(), target);
        // TODO: Arbitrary number. Should probably use the navigation system to
        // find the "next best square" from current position
        if (distance >= 8.0f)
        {
            ChangeState(entity, WavemobTargetOutOfRangeState{});
        }
    }

    // ====== WavemobDyingState =======================================================

    void WavemobStateMachine::onEnter(WavemobDyingState&, const entt::entity entity)
    {
        auto& combatable = registry->get<CombatableActor>(entity);
        combatable.target = entt::null;
        combatable.dying = true;
        const auto& bb = registry->get<sage::Collideable>(entity).worldBoundingBox;
        sys->engine.navigationGridSystem->MarkSquareAreaOccupied(bb, false);

        auto& animation = registry->get<sage::Animation>(entity);
        animation.ChangeAnimationById(lq::animation_ids::Death, true);

        auto& state = registry->get<WavemobState>(entity);
        state.BindSubscription(
            animation.onAnimationEnd.Subscribe([this](const entt::entity e) { destroyEntity(e); }));

        const auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::ENEMY_AUTOATTACK);
        registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);

        sys->engine.actorMovementSystem->CancelMovement(entity);
    }

    void WavemobStateMachine::destroyEntity(const entt::entity entity)
    {
        registry->get<WavemobState>(entity).RemoveAllSubscriptions();
        registry->emplace<sage::DeleteEntityComponent>(entity);
    }

    // ====== Cross-state handlers ====================================================

    void WavemobStateMachine::onHit(const AttackData attackData)
    {
        auto& combatable = registry->get<CombatableActor>(attackData.hit);
        combatable.target = attackData.attacker;
        ChangeState(attackData.hit, WavemobCombatState{});
    }

    void WavemobStateMachine::onDeath(const entt::entity entity)
    {
        ChangeState(entity, WavemobDyingState{});
    }

    // ====== Lifecycle ===============================================================

    void WavemobStateMachine::Update()
    {
        for (const auto view = registry->view<WavemobState>(); const auto& entity : view)
        {
            auto& state = registry->get<WavemobState>(entity);
            std::visit([this, entity](auto& cur) { update(cur, entity); }, state.current);
        }
    }

    void WavemobStateMachine::Draw3D()
    {
    }

    void WavemobStateMachine::onComponentAdded(const entt::entity entity)
    {
        auto& combatable = registry->get<CombatableActor>(entity);
        // Persistent subscriptions — survive state transitions, freed implicitly when the
        // combatable component (or the entity) is destroyed.
        combatable.onHit.Subscribe([this](const AttackData ad) { onHit(ad); });
        combatable.onDeath.Subscribe([this](const entt::entity e) { onDeath(e); });

        auto& state = registry->get<WavemobState>(entity);
        std::visit([this, entity](auto& cur) { onEnter(cur, entity); }, state.current);
    }

    WavemobStateMachine::WavemobStateMachine(entt::registry* _registry, Systems* _sys)
        : Base(_registry), sys(_sys)
    {
        registry->on_construct<WavemobState>().connect<&WavemobStateMachine::onComponentAdded>(this);
        registry->on_destroy<WavemobState>().connect<&WavemobStateMachine::onComponentRemoved>(this);
    }
} // namespace lq
