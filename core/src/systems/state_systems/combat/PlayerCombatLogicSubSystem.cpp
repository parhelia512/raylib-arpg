//
// Created by Steve on 05/06/24.
//


#include "PlayerCombatLogicSubSystem.hpp"
#include "components/states/PlayerStateComponents.hpp"
#include "components/CombatableActor.hpp"
#include "components/Animation.hpp"
#include "components/sgTransform.hpp"
#include "components/HealthBar.hpp"

#include "raylib.h"
#include "raymath.h"

namespace sage
{
	void PlayerCombatLogicSubSystem::Update() const
	{
		auto view = registry->view<CombatableActor, StatePlayerCombat>();

		for (const auto& entity : view)
		{
			auto& c = registry->get<CombatableActor>(entity);
			if (!CheckInCombat(entity)) continue;

			// Player is out of combat if no enemy is targetting them?
			if (c.autoAttackTick >= c.autoAttackTickThreshold)
				// Maybe can count time since last autoattack to time out combat?
			{
				AutoAttack(entity);
			}
			else
			{
				c.autoAttackTick += GetFrameTime();
			}
		}
	}

	bool PlayerCombatLogicSubSystem::CheckInCombat(entt::entity entity) const
	{
		// If the entity is not the target of any other combatable.
		// If no current target
		// Have a timer for aggro and if the player is not within that range for a certain amount of time they resume their regular task (tasks TBC)
		auto& combatable = registry->get<CombatableActor>(entity);
		if (combatable.target == entt::null)
		{
			stateMachineSystem->ChangeState<StatePlayerDefault, StateComponents>(entity);
			return false;
		}
		return true;
	}

	void PlayerCombatLogicSubSystem::OnDeath(entt::entity entity)
	{
	}

	void PlayerCombatLogicSubSystem::OnTargetDeath(entt::entity entity)
	{
		auto& enemyCombatable = registry->get<CombatableActor>(entity);
		auto& playerCombatable = registry->get<CombatableActor>(controllableActorSystem->GetControlledActor());
		{
			entt::sink sink{ enemyCombatable.onDeath };
			sink.disconnect<&PlayerCombatLogicSubSystem::OnTargetDeath>(this);
		}
		{
			entt::sink sink{ playerCombatable.onAttackCancelled };
			sink.disconnect<&PlayerCombatLogicSubSystem::OnAttackCancel>(this);
		}
		playerCombatable.target = entt::null;
	}

	void PlayerCombatLogicSubSystem::OnAttackCancel(entt::entity entity)
	{
		auto& playerCombatable = registry->get<CombatableActor>(entity);
		playerCombatable.target = entt::null;
		auto& playerTrans = registry->get<sgTransform>(entity);
		{
			entt::sink sink{ playerTrans.onFinishMovement };
			sink.disconnect<&PlayerCombatLogicSubSystem::StartCombat>(this);
		}
		controllableActorSystem->CancelMovement(entity);
	}

	
	void PlayerCombatLogicSubSystem::AutoAttack(entt::entity entity) const
	{
		// TODO: Check if unit is still within our attack range?
		auto& c = registry->get<CombatableActor>(entity);

		auto& t = registry->get<sgTransform>(entity);
		auto& enemyPos = registry->get<sgTransform>(c.target).position();
		Vector3 direction = Vector3Subtract(enemyPos, t.position());
		float angle = atan2f(direction.x, direction.z) * RAD2DEG;
		t.SetRotation({ 0, angle, 0 }, entity);
		c.autoAttackTick = 0;

		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);
		if (registry->any_of<CombatableActor>(c.target))
		{
			auto& enemyCombatable = registry->get<CombatableActor>(c.target);
			enemyCombatable.onHit.publish(c.target, entity, 10); // TODO: tmp dmg
		}
	}

	void PlayerCombatLogicSubSystem::OnHit(entt::entity entity, entt::entity attacker)
	{
	}

	void PlayerCombatLogicSubSystem::onEnemyClick(entt::entity actor, entt::entity target)
	{
		auto& combatable = registry->get<CombatableActor>(actor);
		{
			entt::sink sink{ combatable.onAttackCancelled };
			sink.connect<&PlayerCombatLogicSubSystem::OnAttackCancel>(this);
		}
		auto& playerTrans = registry->get<sgTransform>(actor);
		const auto& enemyTrans = registry->get<sgTransform>(target);

		const auto& enemyCollideable = registry->get<Collideable>(combatable.target);
		Vector3 enemyPos = enemyTrans.position();

		// Calculate the direction vector from player to enemy
		Vector3 direction = Vector3Subtract(enemyPos, playerTrans.position());

		// Normalize the direction vector
		float length = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
		direction.x = (direction.x / length) * combatable.attackRange;
		direction.y = (direction.y / length) * combatable.attackRange;
		direction.z = (direction.z / length) * combatable.attackRange;

		// Calculate the target position by subtracting the normalized direction vector
		// multiplied by the attack range from the enemy position
		Vector3 targetPos = Vector3Subtract(enemyPos, direction);

		controllableActorSystem->PathfindToLocation(actor, targetPos);
		{
			entt::sink sink{playerTrans.onFinishMovement};
			sink.connect<&PlayerCombatLogicSubSystem::StartCombat>(this);
		}
	}

	void PlayerCombatLogicSubSystem::StartCombat(entt::entity entity)
	{
		{
			auto& playerTrans = registry->get<sgTransform>(entity);
			entt::sink sink{ playerTrans.onFinishMovement };
			sink.disconnect<&PlayerCombatLogicSubSystem::StartCombat>(this);
		}

		auto& playerCombatable = registry->get<CombatableActor>(entity);
		stateMachineSystem->ChangeState<StatePlayerCombat, StateComponents>(entity);

		auto& enemyCombatable = registry->get<CombatableActor>(playerCombatable.target);
		{
			entt::sink sink{ enemyCombatable.onDeath };
			sink.connect<&PlayerCombatLogicSubSystem::OnTargetDeath>(this);
		}
	}

	void PlayerCombatLogicSubSystem::Enable()
	{
		// Add checks to see if the player should be in combat
		auto view = registry->view<CombatableActor>();
		for (const auto& entity : view)
		{
			auto& combatable = registry->get<CombatableActor>(entity);
			if (combatable.actorType == CombatableActorType::PLAYER)
			{
				{
					entt::sink sink{ combatable.onEnemyClicked };
					sink.connect<&PlayerCombatLogicSubSystem::onEnemyClick>(this);
				}
				{
					entt::sink sink{ combatable.onAttackCancelled };
					sink.connect<&PlayerCombatLogicSubSystem::OnAttackCancel>(this);
				}
			}
		}
	}

	void PlayerCombatLogicSubSystem::Disable()
	{
		// Remove checks to see if the player should be in combat
		auto view = registry->view<CombatableActor>();
		for (const auto& entity : view)
		{
			auto& combatable = registry->get<CombatableActor>(entity);
			if (combatable.actorType == CombatableActorType::PLAYER)
			{
				{
					entt::sink sink{ combatable.onEnemyClicked };
					sink.disconnect<&PlayerCombatLogicSubSystem::onEnemyClick>(this);
				}
				{
					entt::sink sink{ combatable.onAttackCancelled };
					sink.disconnect<&PlayerCombatLogicSubSystem::OnAttackCancel>(this);
				}
			}
		}
	}

	void PlayerCombatLogicSubSystem::OnStateAdded(entt::entity entity) const
	{
		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK); // TODO: Change to "combat move" animation
	}

	void PlayerCombatLogicSubSystem::OnStateRemoved(entt::entity entity) const
	{
		controllableActorSystem->CancelMovement(entity);
	}

	PlayerCombatLogicSubSystem::PlayerCombatLogicSubSystem(
		entt::registry* _registry,
		StateMachineSystem* _stateMachineSystem,
		ControllableActorSystem* _controllableActorSystem) :
		registry(_registry),
		stateMachineSystem(_stateMachineSystem),
		controllableActorSystem(_controllableActorSystem)
	{
		registry->on_construct<StatePlayerCombat>().connect<&PlayerCombatLogicSubSystem::OnStateAdded>(this);
		registry->on_destroy<StatePlayerCombat>().connect<&PlayerCombatLogicSubSystem::OnStateRemoved>(this);
	}
} // sage
