#include "PlayerStateMachine.hpp"
#include "animation/RpgAnimationIds.hpp"

#include "Systems.hpp"

#include "../../components/ControllableActor.hpp"
#include "../ControllableActorSystem.hpp"
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
#include "PartyMemberStateMachine.hpp"
#include "systems/DialogSystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"

#include "engine/systems/TransformSystem.hpp"
#include "raylib.h"
#include "StateMachines.hpp"
#include "systems/LootSystem.hpp"
#include "systems/PartySystem.hpp"

#include <cassert>
#include <format>

namespace lq
{
    class PlayerStateMachine::DefaultState : public sage::State
    {
        PlayerStateMachine* stateController;

      public:
        void Update(entt::entity entity) override
        {
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnEnter(entt::entity entity, const sage::StatePayload&) override
        {
            auto& animation = registry->get<sage::Animation>(entity);
            animation.ChangeAnimationById(lq::animation_ids::Idle);
        }

        void OnExit(entt::entity entity) override
        {
        }

        ~DefaultState() override = default;

        explicit DefaultState(entt::registry* _registry) : State(_registry), stateController(nullptr)
        {
        }

        friend class PlayerStateMachine;
    };

    // ----------------------------

    class PlayerStateMachine::MovingToLocationState : public sage::State
    {
        Systems* sys;
        PlayerStateMachine* stateController;

        void onMovementCancelled(entt::entity self) const
        {

            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(entt::entity self) const
        {
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

      public:
        void Update(entt::entity self) override
        {
        }

        void OnEnter(entt::entity self, const sage::StatePayload&) override
        {
            sys->engine.actorMovementSystem->CancelMovement(self);
            auto& moveable = registry->get<sage::MoveableActor>(self);
            auto& state = registry->get<PlayerState>(self);

            auto party = sys->partySystem->GetAllMembers();

            for (const auto& member : party)
            {
                const auto& collideable = registry->get<sage::Collideable>(member);
                sys->engine.navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false);
            }

            if (sys->engine.actorMovementSystem->TryPathfindToLocation(
                    self, sys->engine.cursor->getFirstCollision().point))
            {
                auto& animation = registry->get<sage::Animation>(self);
                animation.ChangeAnimationById(lq::animation_ids::Run);
                auto sub = moveable.onDestinationReached.Subscribe(
                    [this](entt::entity _entity) { onTargetReached(_entity); });
                state.ManageSubscription(sub);
                auto sub1 = moveable.onMovementCancel.Subscribe(
                    [this](entt::entity _entity) { onMovementCancelled(_entity); });
                state.ManageSubscription(sub1);
                auto sub2 = moveable.onDestinationUnreachable.Subscribe(
                    [this](entt::entity _entity, Vector3) { onMovementCancelled(_entity); });
                state.ManageSubscription(sub2);
            }
            else
            {
                stateController->ChangeState(self, PlayerStateEnum::Default);
            }

            for (const auto& member : party)
            {
                const auto& collideable = registry->get<sage::Collideable>(member);
                sys->engine.navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true);
            }
        }

        void OnExit(entt::entity self) override
        {
        }

        ~MovingToLocationState() override = default;

        MovingToLocationState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry), sys(_sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateMachine::MovingToTalkToNPCState final : public sage::State
    {
        Systems* sys;
        PlayerStateMachine* stateController;
        std::unordered_map<entt::entity, entt::entity> targets;

        void onMovementCancelled(const entt::entity self) const
        {
            auto& moveable = registry->get<sage::MoveableActor>(self);
            moveable.movementCollisionTarget.reset();
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(const entt::entity self) const
        {
            const DialogTargetPayload payload{targets.at(self)};
            stateController->ChangeState(self, PlayerStateEnum::InDialog, payload);
        }

      public:
        void Update(entt::entity self) override
        {
        }

        void OnEnter(entt::entity self, const sage::StatePayload& payload) override
        {
            auto& moveable = registry->get<sage::MoveableActor>(self);
            const auto& dialogPayload = dynamic_cast<const DialogTargetPayload&>(payload);
            assert(dialogPayload.target != entt::null);
            targets.insert_or_assign(self, dialogPayload.target);
            moveable.movementCollisionTarget = dialogPayload.target;
            const auto& pos = registry->get<DialogComponent>(dialogPayload.target).conversationPos;
            sys->engine.actorMovementSystem->PathfindToLocation(self, pos);

            auto& state = registry->get<PlayerState>(self);
            auto sub = moveable.onDestinationReached.Subscribe(
                [this](entt::entity _entity) { onTargetReached(_entity); });
            state.ManageSubscription(std::move(sub));
            auto sub1 = moveable.onMovementCancel.Subscribe(
                [this](entt::entity _entity) { onMovementCancelled(_entity); });
            state.ManageSubscription(std::move(sub1));

            auto& animation = registry->get<sage::Animation>(self);
            animation.ChangeAnimationById(lq::animation_ids::Run);
        }

        void OnExit(entt::entity self) override
        {
            targets.erase(self);
            registry->get<sage::MoveableActor>(self).movementCollisionTarget.reset();
        }

        ~MovingToTalkToNPCState() override = default;

        MovingToTalkToNPCState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry), sys(_sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateMachine::InDialogState : public sage::State
    {
        Systems* sys;
        PlayerStateMachine* stateController;
        std::unordered_map<entt::entity, entt::entity> targets;

      public:
        void OnEnter(entt::entity self, const sage::StatePayload& payload) override
        {
            const auto& dialogPayload = dynamic_cast<const DialogTargetPayload&>(payload);
            assert(dialogPayload.target != entt::null);
            targets.insert_or_assign(self, dialogPayload.target);
            const auto target = dialogPayload.target;
            registry->get<sage::Animation>(self).ChangeAnimationById(lq::animation_ids::Talk);
            if (registry->any_of<sage::Animation>(target))
            {
                registry->get<sage::Animation>(target).ChangeAnimationById(lq::animation_ids::Talk);
            }

            // Rotate to look at NPC
            auto& actorTrans = registry->get<sage::sgTransform>(self);
            const auto& npcTrans = registry->get<sage::sgTransform>(target);
            Vector3 direction = Vector3Subtract(npcTrans.GetWorldPos(), actorTrans.GetWorldPos());
            direction = Vector3Normalize(direction);
            const float angle = atan2f(direction.x, direction.z);
            sys->engine.transformSystem->SetRotation(
                self, {actorTrans.GetWorldRot().x, RAD2DEG * angle, actorTrans.GetWorldRot().z});

            sys->dialogSystem->StartConversation(npcTrans, target);
            sys->playerAbilitySystem->UnsubscribeFromUserInput();
        }

        void OnExit(entt::entity self) override
        {
            const auto target = targets.at(self);
            if (registry->any_of<sage::Animation>(target))
            {
                registry->get<sage::Animation>(target).ChangeAnimationById(lq::animation_ids::Idle);
            }
            targets.erase(self);
            sys->playerAbilitySystem->SubscribeToUserInput();
            // TODO: Bug: Doesn't change back to default on dialog end
        }

        ~InDialogState() override = default;

        InDialogState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry), sys(_sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateMachine::MovingToLootState : public sage::State
    {
      public:
        struct Payload final : sage::StatePayload
        {
            entt::entity target = entt::null;

            explicit Payload(const entt::entity _target) : target(_target)
            {
            }
        };

      private:
        Systems* sys;
        PlayerStateMachine* stateController;
        std::unordered_map<entt::entity, entt::entity> targets;

        void onMovementCancelled(const entt::entity self) const
        {
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(const entt::entity self) const
        {
            sys->lootSystem->OnChestClick(targets.at(self));
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

      public:
        void Update(entt::entity self) override
        {
        }

        void OnEnter(entt::entity self, const sage::StatePayload& payload) override
        {
            auto& moveable = registry->get<sage::MoveableActor>(self);
            const auto& lootPayload = dynamic_cast<const Payload&>(payload);
            const auto target = lootPayload.target;
            assert(target != entt::null);
            targets.insert_or_assign(self, target);
            const auto& trans = registry->get<sage::sgTransform>(self);
            const auto& chestTrans = registry->get<sage::sgTransform>(target);
            Vector3 dest = Vector3Add(
                trans.GetWorldPos(),
                sage::Vector3MultiplyByValue(
                    Vector3Subtract(chestTrans.GetWorldPos(), trans.GetWorldPos()), 0.85));
            // TODO: N.B. Right now, its possible that 'dest' is outside of LOOT_RANGE
            sys->engine.actorMovementSystem->PathfindToLocation(self, dest);

            auto& state = registry->get<PlayerState>(self);
            auto sub = moveable.onDestinationReached.Subscribe(
                [this](entt::entity _entity) { onTargetReached(_entity); });
            state.ManageSubscription(std::move(sub));
            auto sub1 = moveable.onMovementCancel.Subscribe(
                [this](entt::entity _entity) { onMovementCancelled(_entity); });
            state.ManageSubscription(std::move(sub1));

            auto& animation = registry->get<sage::Animation>(self);
            animation.ChangeAnimationById(lq::animation_ids::Run);
        }

        void OnExit(entt::entity self) override
        {
            targets.erase(self);
        }

        ~MovingToLootState() override = default;

        MovingToLootState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry), sys(_sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class PlayerStateMachine::MovingToAttackEnemyState : public sage::State
    {
        Systems* sys;
        PlayerStateMachine* stateController;

        void onAttackCancelled(entt::entity self, entt::entity) const
        {
            auto& playerCombatable = registry->get<CombatableActor>(self);
            playerCombatable.target = entt::null;
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        void onTargetReached(entt::entity self) const
        {
            stateController->ChangeState(self, PlayerStateEnum::Combat);
        }

      public:
        void OnEnter(entt::entity self, const sage::StatePayload&) override
        {
            auto& animation = registry->get<sage::Animation>(self);
            animation.ChangeAnimationById(lq::animation_ids::Run);

            auto& moveableActor = registry->get<sage::MoveableActor>(self);

            auto& state = registry->get<PlayerState>(self);
            auto sub = moveableActor.onDestinationReached.Subscribe(
                [this](const entt::entity _entity) { onTargetReached(_entity); });
            state.ManageSubscription(std::move(sub));

            auto& combatable = registry->get<CombatableActor>(self);
            assert(combatable.target != entt::null);

            auto& controllable = registry->get<ControllableActor>(self);
            auto sub1 = controllable.onFloorClick.Subscribe(
                [this](const entt::entity _self, entt::entity clicked) { onAttackCancelled(_self, clicked); });
            state.ManageSubscription(std::move(sub1));

            const auto& enemyTrans = registry->get<sage::sgTransform>(combatable.target);

            Vector3 playerPos = registry->get<sage::sgTransform>(self).GetWorldPos();
            Vector3 enemyPos = enemyTrans.GetWorldPos();
            Vector3 direction = Vector3Subtract(enemyPos, playerPos);
            direction = Vector3Scale(Vector3Normalize(direction), combatable.attackRange);

            Vector3 targetPos = Vector3Subtract(enemyPos, direction);

            sys->engine.actorMovementSystem->PathfindToLocation(self, targetPos);
        }

        void OnExit(entt::entity self) override
        {
        }

        ~MovingToAttackEnemyState() override = default;

        MovingToAttackEnemyState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry), sys(_sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    // TODO: Move combat to its own state machine
    class PlayerStateMachine::CombatState : public sage::State
    {
        Systems* sys;
        PlayerStateMachine* stateController;

        void onAttackCancelled(entt::entity self, entt::entity) const
        {
            // Both outcomes are the same
            onTargetDeath(self, entt::null);
        }

        void onTargetDeath(entt::entity self, entt::entity) const
        {
            auto& combatable = registry->get<CombatableActor>(self);
            combatable.target = entt::null;
            stateController->ChangeState(self, PlayerStateEnum::Default);
        }

        bool checkInCombat(entt::entity entity)
        {
            // Might do more here later
            return true;
        }

      public:
        void Update(entt::entity entity) override
        {
        }

        void OnEnter(entt::entity entity, const sage::StatePayload&) override
        {
            auto& animation = registry->get<sage::Animation>(entity);
            animation.ChangeAnimationById(lq::animation_ids::AutoAttack);

            auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
            registry->get<Ability>(abilityEntity).startCast.Publish(abilityEntity);

            auto& combatable = registry->get<CombatableActor>(entity);
            assert(combatable.target != entt::null);

            auto& enemyCombatable = registry->get<CombatableActor>(combatable.target);
            combatable.onTargetDeathSub =
                enemyCombatable.onDeath.Subscribe([entity, this](const entt::entity target) {
                    const auto& c = registry->get<CombatableActor>(entity);
                    c.onTargetDeath.Publish(entity, target);
                });

            auto& state = registry->get<PlayerState>(entity);
            auto sub = combatable.onTargetDeath.Subscribe(
                [this](entt::entity self, entt::entity target) { onTargetDeath(self, target); });
            state.ManageSubscription(std::move(sub));
        }

        void OnExit(entt::entity entity) override
        {
            auto& combatable = registry->get<CombatableActor>(entity);
            combatable.onTargetDeathSub.UnSubscribe();
            auto abilityEntity = sys->abilityFactory->GetAbility(entity, AbilityEnum::PLAYER_AUTOATTACK);
            registry->get<Ability>(abilityEntity).cancelCast.Publish(abilityEntity);
        }

        ~CombatState() override = default;
        CombatState(entt::registry* _registry, Systems* _sys, PlayerStateMachine* _stateController)
            : State(_registry), sys(_sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    void PlayerStateMachine::onFloorClick(const entt::entity self, entt::entity)
    {
        auto& state = registry->get<PlayerState>(self);
        // We're not allowed to change to the same state, so change to default and then back again
        if (state.GetCurrentStateEnum() == PlayerStateEnum::MovingToLocation)
        {
            ChangeState(self, PlayerStateEnum::Default);
        }
        ChangeState(self, PlayerStateEnum::MovingToLocation);
    }

    void PlayerStateMachine::onChestClick(const entt::entity self, entt::entity target)
    {
        auto& state = registry->get<PlayerState>(self);
        // We're not allowed to change to the same state, so change to default and then back again
        if (state.GetCurrentStateEnum() == PlayerStateEnum::MovingToLoot)
        {
            ChangeState(self, PlayerStateEnum::Default);
        }
        const MovingToLootState::Payload payload{target};
        ChangeState(self, PlayerStateEnum::MovingToLoot, payload);
    }

    void PlayerStateMachine::onNPCLeftClick(entt::entity self, entt::entity target)
    {
        if (!registry->any_of<DialogComponent>(target)) return;

        const DialogTargetPayload payload{target};
        ChangeState(self, PlayerStateEnum::MovingToTalkToNPC, payload);
        //        ChangeStateExArgs<MovingToTalkToNPCState, entt::entity>(self, PlayerStateEnum::MovingToTalkToNPC,
        //        target);
    }

    void PlayerStateMachine::onEnemyLeftClick(entt::entity self, entt::entity target)
    {
        auto& combatable = registry->get<CombatableActor>(self);
        combatable.target = target;
        ChangeState(self, PlayerStateEnum::MovingToAttackEnemy);
    }

    void PlayerStateMachine::Update()
    {
        for (const auto view = registry->view<PlayerState>(); const auto& entity : view)
        {
            assert(!registry->any_of<PartyMemberState>(entity));
            const auto state = registry->get<PlayerState>(entity).GetCurrentStateEnum();
            GetStateFromEnum(state)->Update(entity);
        }
    }

    void PlayerStateMachine::Draw3D()
    {
        for (const auto view = registry->view<PlayerState>(); const auto& entity : view)
        {
            assert(!registry->any_of<PartyMemberState>(entity));
            const auto state = registry->get<PlayerState>(entity).GetCurrentStateEnum();
            GetStateFromEnum(state)->Draw3D(entity);
        }
    }

    void PlayerStateMachine::onComponentAdded(entt::entity entity)
    {
        // Cursor and controllable events are connected in ControllableActorSystem
        auto& controllable = registry->get<ControllableActor>(entity);
        controllable.onEnemyLeftClickSub = controllable.onEnemyLeftClick.Subscribe(
            [this](entt::entity self, entt::entity target) { onEnemyLeftClick(self, target); });
        controllable.onNPCLeftClickSub = controllable.onNPCLeftClick.Subscribe(
            [this](entt::entity self, entt::entity target) { onNPCLeftClick(self, target); });
        controllable.onFloorClickSub = controllable.onFloorClick.Subscribe(
            [this](entt::entity self, entt::entity target) { onFloorClick(self, target); });
        controllable.onChestClickSub = controllable.onChestClick.Subscribe(
            [this](entt::entity self, entt::entity target) { onChestClick(self, target); });
        // ----------------------------
    }

    void PlayerStateMachine::onComponentRemoved(entt::entity entity)
    {
        auto& controllable = registry->get<ControllableActor>(entity);
        controllable.onEnemyLeftClickSub.UnSubscribe();
        controllable.onChestClickSub.UnSubscribe();
        controllable.onNPCLeftClickSub.UnSubscribe();
        controllable.onFloorClickSub.UnSubscribe();
    }

    PlayerStateMachine::PlayerStateMachine(entt::registry* _registry, Systems* _sys) : StateMachine(_registry)
    {
        states[PlayerStateEnum::Default] = std::make_unique<DefaultState>(_registry);
        states[PlayerStateEnum::MovingToAttackEnemy] =
            std::make_unique<MovingToAttackEnemyState>(_registry, _sys, this);
        states[PlayerStateEnum::Combat] = std::make_unique<CombatState>(_registry, _sys, this);
        states[PlayerStateEnum::MovingToTalkToNPC] =
            std::make_unique<MovingToTalkToNPCState>(_registry, _sys, this);
        states[PlayerStateEnum::InDialog] = std::make_unique<InDialogState>(_registry, _sys, this);
        states[PlayerStateEnum::MovingToLocation] = std::make_unique<MovingToLocationState>(_registry, _sys, this);
        states[PlayerStateEnum::MovingToLoot] = std::make_unique<MovingToLootState>(_registry, _sys, this);

        registry->on_construct<PlayerState>().connect<&PlayerStateMachine::onComponentAdded>(this);
        registry->on_destroy<PlayerState>().connect<&PlayerStateMachine::onComponentRemoved>(this);
    }
} // namespace lq
