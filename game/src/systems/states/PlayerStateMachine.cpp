#include "PlayerStateMachine.hpp"
#include "animation/RpgAnimationIds.hpp"

#include "Systems.hpp"

#include "../../components/ControllableActor.hpp"
#include "AbilityFactory.hpp"
#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"
#include "components/DialogComponent.hpp"
#include "engine/Camera.hpp"
#include "engine/components/Animation.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Cursor.hpp"
#include "engine/systems/ActorMovementSystem.hpp"
#include "engine/systems/TransformSystem.hpp"
#include "raylib.h"
#include "systems/DialogSystem.hpp"
#include "systems/LootSystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"

#include <cassert>

namespace lq
{
    // ====== PlayerDefaultState ======================================================

    void PlayerStateMachine::onEnter(PlayerDefaultState&, const entt::entity entity)
    {
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Idle);
    }

    // ====== PlayerMovingToLocationState =============================================

    void PlayerStateMachine::onEnter(PlayerMovingToLocationState&, const entt::entity entity)
    {
        sys->engine.actorMovementSystem->CancelMovement(entity);
        auto& moveable = registry->get<sage::MoveableActor>(entity);
        auto& state = registry->get<PlayerState>(entity);

        const auto party = sys->partySystem->GetAllMembers();
        for (const auto& member : party)
        {
            const auto& collideable = registry->get<sage::Collideable>(member);
            sys->engine.navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false);
        }

        if (sys->engine.actorMovementSystem->TryPathfindToLocation(
                entity, sys->engine.cursor->getFirstCollision().point))
        {
            registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Run);
            state.BindSubscription(moveable.onDestinationReached.Subscribe(
                [this](const entt::entity e) { ChangeState(e, PlayerDefaultState{}); }));
            state.BindSubscription(moveable.onMovementCancel.Subscribe(
                [this](const entt::entity e) { ChangeState(e, PlayerDefaultState{}); }));
            state.BindSubscription(moveable.onDestinationUnreachable.Subscribe(
                [this](const entt::entity e, Vector3) { ChangeState(e, PlayerDefaultState{}); }));
        }
        else
        {
            ChangeState(entity, PlayerDefaultState{});
        }

        for (const auto& member : party)
        {
            const auto& collideable = registry->get<sage::Collideable>(member);
            sys->engine.navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true);
        }
    }

    // ====== PlayerMovingToAttackEnemyState ==========================================

    void PlayerStateMachine::onEnter(PlayerMovingToAttackEnemyState&, const entt::entity entity)
    {
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Run);

        auto& moveableActor = registry->get<sage::MoveableActor>(entity);
        auto& combatable = registry->get<CombatableActor>(entity);
        assert(combatable.target != entt::null);
        auto& controllable = registry->get<ControllableActor>(entity);
        auto& state = registry->get<PlayerState>(entity);

        auto onTargetReached = [this](const entt::entity e) { ChangeState(e, PlayerCombatState{}); };
        auto onAttackCancelled = [this](const entt::entity e, entt::entity) {
            registry->get<CombatableActor>(e).target = entt::null;
            ChangeState(e, PlayerDefaultState{});
        };

        state.BindSubscription(moveableActor.onDestinationReached.Subscribe(onTargetReached));
        state.BindSubscription(controllable.onFloorClick.Subscribe(onAttackCancelled));

        const Vector3 playerPos = registry->get<sage::sgTransform>(entity).GetWorldPos();
        const Vector3 enemyPos = registry->get<sage::sgTransform>(combatable.target).GetWorldPos();
        const Vector3 offset =
            Vector3Scale(Vector3Normalize(Vector3Subtract(enemyPos, playerPos)), combatable.attackRange);
        sys->engine.actorMovementSystem->PathfindToLocation(entity, Vector3Subtract(enemyPos, offset));
    }

    // ====== PlayerMovingToTalkState =================================================

    void PlayerStateMachine::onEnter(PlayerMovingToTalkState& s, const entt::entity entity)
    {
        assert(s.target != entt::null);
        auto& moveable = registry->get<sage::MoveableActor>(entity);
        auto& state = registry->get<PlayerState>(entity);

        moveable.movementCollisionTarget = s.target;
        const auto destination = registry->get<DialogComponent>(s.target).conversationPos;
        sys->engine.actorMovementSystem->PathfindToLocation(entity, destination);

        state.BindSubscription(moveable.onDestinationReached.Subscribe([this](const entt::entity e) {
            const auto target = std::get<PlayerMovingToTalkState>(registry->get<PlayerState>(e).current).target;
            ChangeState(e, PlayerInDialogState{.target = target});
        }));
        state.BindSubscription(moveable.onMovementCancel.Subscribe(
            [this](const entt::entity e) { ChangeState(e, PlayerDefaultState{}); }));

        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Run);
    }

    void PlayerStateMachine::onExit(PlayerMovingToTalkState&, const entt::entity entity)
    {
        registry->get<sage::MoveableActor>(entity).movementCollisionTarget.reset();
    }

    // ====== PlayerMovingToLootState =================================================

    void PlayerStateMachine::onEnter(PlayerMovingToLootState& s, const entt::entity entity)
    {
        assert(s.target != entt::null);
        auto& moveable = registry->get<sage::MoveableActor>(entity);
        auto& state = registry->get<PlayerState>(entity);

        const auto& trans = registry->get<sage::sgTransform>(entity);
        const auto& chestTrans = registry->get<sage::sgTransform>(s.target);
        // TODO: N.B. Right now, its possible that a loot destination is outside of LOOT_RANGE.
        const auto destination = Vector3Add(
            trans.GetWorldPos(),
            sage::Vector3MultiplyByValue(
                Vector3Subtract(chestTrans.GetWorldPos(), trans.GetWorldPos()), 0.85));
        sys->engine.actorMovementSystem->PathfindToLocation(entity, destination);

        state.BindSubscription(moveable.onDestinationReached.Subscribe([this](const entt::entity e) {
            const auto target = std::get<PlayerMovingToLootState>(registry->get<PlayerState>(e).current).target;
            sys->lootSystem->OnChestClick(target);
            ChangeState(e, PlayerDefaultState{});
        }));
        state.BindSubscription(moveable.onMovementCancel.Subscribe(
            [this](const entt::entity e) { ChangeState(e, PlayerDefaultState{}); }));

        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Run);
    }

    // ====== PlayerInDialogState =====================================================

    void PlayerStateMachine::onEnter(PlayerInDialogState& s, const entt::entity entity)
    {
        assert(s.target != entt::null);
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::Talk);
        if (registry->any_of<sage::Animation>(s.target))
        {
            registry->get<sage::Animation>(s.target).ChangeAnimationById(lq::animation_ids::Talk);
        }

        auto& actorTrans = registry->get<sage::sgTransform>(entity);
        const auto& npcTrans = registry->get<sage::sgTransform>(s.target);
        const Vector3 direction =
            Vector3Normalize(Vector3Subtract(npcTrans.GetWorldPos(), actorTrans.GetWorldPos()));
        const float angle = atan2f(direction.x, direction.z);
        sys->engine.transformSystem->SetRotation(
            entity, {actorTrans.GetWorldRot().x, RAD2DEG * angle, actorTrans.GetWorldRot().z});

        sys->dialogSystem->StartConversation(npcTrans, s.target);
        sys->playerAbilitySystem->UnsubscribeFromUserInput();
    }

    void PlayerStateMachine::onExit(PlayerInDialogState& s, const entt::entity)
    {
        if (registry->any_of<sage::Animation>(s.target))
        {
            registry->get<sage::Animation>(s.target).ChangeAnimationById(lq::animation_ids::Idle);
        }
        sys->playerAbilitySystem->SubscribeToUserInput();
        // TODO: Bug: Doesn't change back to default on dialog end
    }

    // ====== PlayerCombatState =======================================================

    // TODO: Move combat to its own state machine
    void PlayerStateMachine::onEnter(PlayerCombatState&, const entt::entity entity)
    {
        registry->get<sage::Animation>(entity).ChangeAnimationById(lq::animation_ids::AutoAttack);

        const auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
        registry->get<Ability>(abilityEntity).startCast.Publish(abilityEntity);

        auto& combatable = registry->get<CombatableActor>(entity);
        assert(combatable.target != entt::null);
        auto& enemyCombatable = registry->get<CombatableActor>(combatable.target);

        auto forwardEnemyDeath = [entity, this](const entt::entity target) {
            registry->get<CombatableActor>(entity).onTargetDeath.Publish(entity, target);
        };
        auto onTargetDeath = [this](const entt::entity e, entt::entity) {
            registry->get<CombatableActor>(e).target = entt::null;
            ChangeState(e, PlayerDefaultState{});
        };

        combatable.onTargetDeathSub = enemyCombatable.onDeath.Subscribe(forwardEnemyDeath);

        auto& state = registry->get<PlayerState>(entity);
        state.BindSubscription(combatable.onTargetDeath.Subscribe(onTargetDeath));
    }

    void PlayerStateMachine::onExit(PlayerCombatState&, const entt::entity entity)
    {
        auto& combatable = registry->get<CombatableActor>(entity);
        combatable.onTargetDeathSub.UnSubscribe();
        const auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
        registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);
    }

    // ====== Click handlers ==========================================================

    void PlayerStateMachine::onFloorClick(const entt::entity entity, entt::entity)
    {
        ChangeState(entity, PlayerMovingToLocationState{});
    }

    void PlayerStateMachine::onChestClick(const entt::entity entity, const entt::entity target)
    {
        ChangeState(entity, PlayerMovingToLootState{.target = target});
    }

    void PlayerStateMachine::onNPCLeftClick(const entt::entity entity, const entt::entity target)
    {
        if (!registry->any_of<DialogComponent>(target)) return;
        ChangeState(entity, PlayerMovingToTalkState{.target = target});
    }

    void PlayerStateMachine::onEnemyLeftClick(const entt::entity entity, const entt::entity target)
    {
        registry->get<CombatableActor>(entity).target = target;
        ChangeState(entity, PlayerMovingToAttackEnemyState{});
    }

    // ====== Lifecycle ===============================================================

    void PlayerStateMachine::Update()
    {
    }

    void PlayerStateMachine::Draw3D()
    {
    }

    void PlayerStateMachine::onComponentAdded(const entt::entity entity)
    {
        // Cursor and controllable events are connected in ControllableActorSystem
        auto& controllable = registry->get<ControllableActor>(entity);
        controllable.onEnemyLeftClickSub = controllable.onEnemyLeftClick.Subscribe(
            [this](entt::entity e, entt::entity target) { onEnemyLeftClick(e, target); });
        controllable.onNPCLeftClickSub = controllable.onNPCLeftClick.Subscribe(
            [this](entt::entity e, entt::entity target) { onNPCLeftClick(e, target); });
        controllable.onFloorClickSub = controllable.onFloorClick.Subscribe(
            [this](entt::entity e, entt::entity target) { onFloorClick(e, target); });
        controllable.onChestClickSub = controllable.onChestClick.Subscribe(
            [this](entt::entity e, entt::entity target) { onChestClick(e, target); });

        auto& state = registry->get<PlayerState>(entity);
        std::visit([this, entity](auto& cur) { onEnter(cur, entity); }, state.current);
    }

    void PlayerStateMachine::onComponentRemoved(const entt::entity entity)
    {
        auto& controllable = registry->get<ControllableActor>(entity);
        controllable.onEnemyLeftClickSub.UnSubscribe();
        controllable.onChestClickSub.UnSubscribe();
        controllable.onNPCLeftClickSub.UnSubscribe();
        controllable.onFloorClickSub.UnSubscribe();
    }

    PlayerStateMachine::PlayerStateMachine(entt::registry* _registry, Systems* _sys)
        : Base(_registry), sys(_sys)
    {
        registry->on_construct<PlayerState>().connect<&PlayerStateMachine::onComponentAdded>(this);
        registry->on_destroy<PlayerState>().connect<&PlayerStateMachine::onComponentRemoved>(this);
    }
} // namespace lq
