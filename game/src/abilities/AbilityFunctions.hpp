#pragma once

#include "raylib.h"

#include "entt/entt.hpp"

namespace sage
{
    class EngineSystems;
}
namespace lq
{
    void AOEAtPoint(
        entt::registry* registry, entt::entity caster, entt::entity abilityEntity, Vector3 point, float radius);

    void HitSingleTarget(
        entt::registry* registry,
        sage::EngineSystems* sys,
        entt::entity caster,
        entt::entity abilityEntity,
        entt::entity target);

} // namespace lq