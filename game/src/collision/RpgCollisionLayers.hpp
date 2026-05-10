#pragma once

#include "engine/CollisionLayers.hpp"

namespace lq::collision_layers
{
    inline constexpr sage::CollisionLayer Player = sage::MakeCollisionLayer(16);
    inline constexpr sage::CollisionLayer Npc = sage::MakeCollisionLayer(17);
    inline constexpr sage::CollisionLayer Enemy = sage::MakeCollisionLayer(18);
    inline constexpr sage::CollisionLayer Building = sage::MakeCollisionLayer(19);
    inline constexpr sage::CollisionLayer Item = sage::MakeCollisionLayer(20);
    inline constexpr sage::CollisionLayer Interactable = sage::MakeCollisionLayer(21);
    inline constexpr sage::CollisionLayer Chest = sage::MakeCollisionLayer(22);
} // namespace lq::collision_layers

namespace lq::collision_masks
{
    inline constexpr sage::CollisionMask DefaultQuery =
        sage::collision_masks::Navigation | collision_layers::Player | collision_layers::Enemy |
        collision_layers::Npc | collision_layers::Item | collision_layers::Interactable |
        collision_layers::Chest | collision_layers::Building;

    inline constexpr sage::CollisionMask Player =
        collision_layers::Enemy | collision_layers::Building | collision_layers::Interactable |
        collision_layers::Chest;

    inline constexpr sage::CollisionMask Enemy = collision_layers::Player | collision_layers::Building;

    inline constexpr sage::CollisionMask CursorHover =
        collision_layers::Npc | collision_layers::Enemy | collision_layers::Item |
        collision_layers::Interactable | collision_layers::Chest;

    [[nodiscard]] constexpr sage::CollisionMask ForLayer(const sage::CollisionLayer layer)
    {
        if (layer == collision_layers::Player) return Player;
        if (layer == collision_layers::Enemy) return Enemy;
        return sage::collision_masks::None;
    }
} // namespace lq::collision_masks
