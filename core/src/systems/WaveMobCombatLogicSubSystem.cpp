//
// Created by steve on 05/06/24.
//

#include "raylib.h"
#include "raymath.h"

#include "WaveMobCombatLogicSubSystem.hpp"
#include "../components/CombatableActor.hpp"
#include "../components/Animation.hpp"
#include "../components/Transform.hpp"
#include "../components/HealthBar.hpp"

namespace sage
{
void WaveMobCombatLogicSubSystem::Update(entt::entity entity) const
{
	CheckInCombat(entity);
	auto& c = registry->get<CombatableActor>(entity);
	if (c.target == entt::null || !c.inCombat) return;
	// Move to target
	// Progress tick
	// Turn to look at target
	// Autoattack

	// Player is out of combat if no enemy is targetting them?
	if (c.autoAttackTick >= c.autoAttackTickThreshold) // Maybe can count time since last autoattack to time out combat?
	{
		AutoAttack(entity);
	}
	else
	{
		c.autoAttackTick += GetFrameTime();
	}
}

void WaveMobCombatLogicSubSystem::CheckInCombat(entt::entity entity) const
{
    // If the entity is not the target of any other combatable.
	// If no current target
	// Have a timer for aggro and if the player is not within that range for a certain amount of time they resume their regular task (tasks TBC)
	auto& combatable = registry->get<CombatableActor>(entity);
	if (combatable.target == entt::null)
	{
		combatable.inCombat = false;
		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
	}
}

void WaveMobCombatLogicSubSystem::destroyEnemy(entt::entity entity)
{
    {
        auto& animation = registry->get<Animation>(entity);
        entt::sink sink { animation.onAnimationEnd };
        sink.disconnect<&WaveMobCombatLogicSubSystem::destroyEnemy>(this);
    }
    registry->destroy(entity);
}

void WaveMobCombatLogicSubSystem::OnDeath(entt::entity entity)
{
    auto& combatable = registry->get<CombatableActor>(entity);
	combatable.inCombat = false;
	combatable.target = entt::null;
	
    {
        entt::sink sink { combatable.onHit };
        sink.disconnect<&WaveMobCombatLogicSubSystem::OnHit>(this);
    }
	
    auto& animation = registry->get<Animation>(entity);
    animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
    {
        entt::sink sink { animation.onAnimationEnd };
        sink.connect<&WaveMobCombatLogicSubSystem::destroyEnemy>(this);
    }
}

void WaveMobCombatLogicSubSystem::AutoAttack(entt::entity entity) const
{
	auto& c = registry->get<CombatableActor>(entity);
	auto& t = registry->get<Transform>(entity);
    auto& animation = registry->get<Animation>(entity);
	auto& enemyPos = registry->get<Transform>(c.target).position;
    
    if (Vector3Distance(t.position, enemyPos) > c.attackRange)
    {
        animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
        if (t.movementTick >= t.movementTickThreshold)
        {
            Vector2 minRage;
            Vector2 maxRange;
            navigationGridSystem->GetPathfindRange(entity, 25.0f, minRage, maxRange); // TODO: 25.0f is temporary
            auto path = navigationGridSystem->Pathfind(t.position, enemyPos, minRage, maxRange);
            if (path.empty())
            {
                // Go out of combat
                transformSystem->PathfindToLocation(entity, {t.position});
                c.target = entt::null;
                return;
            }
            transformSystem->PathfindToLocation(entity, path);
            t.movementTick = 0;
        }
        else
        {
            t.movementTick += GetFrameTime();
        }
        return;
    }
    
	Vector3 direction = Vector3Subtract(enemyPos, t.position);
	float angle = atan2f(direction.x, direction.z) * RAD2DEG;
	t.rotation.y = angle;
	c.autoAttackTick = 0;
	animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);
}

void WaveMobCombatLogicSubSystem::StartCombat(entt::entity entity)
{

}

void WaveMobCombatLogicSubSystem::OnHit(entt::entity entity, entt::entity attacker, float damage)
{
    // Aggro when player hits
    auto& c = registry->get<CombatableActor>(entity);
    c.target = attacker;
    c.inCombat = true;
	
	auto& healthbar = registry->get<HealthBar>(entity);
	healthbar.Decrement(entity, damage);
	if (healthbar.hp <= 0)
	{
		c.onDeath.publish(entity);
		c.target = entt::null;
		OnDeath(entity);
	}
}

WaveMobCombatLogicSubSystem::WaveMobCombatLogicSubSystem(entt::registry *_registry,
                                                         TransformSystem* _transformSystem,
                                                         NavigationGridSystem* _navigationGridSystem) :
                                                         registry(_registry),
                                                         transformSystem(_transformSystem),
                                                         navigationGridSystem(_navigationGridSystem)
{}
} // sage