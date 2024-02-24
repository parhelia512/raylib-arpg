//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "Component.hpp"
#include "raylib.h"


namespace sage
{
    enum CollisionLayer
    {
        DEFAULT,
        FLOOR,
        BUILDING
    };

    struct Collideable : public Component
    {
        const BoundingBox localBoundingBox; // BoundingBox in local space
        BoundingBox worldBoundingBox{}; // BoundingBox in world space (bb pos + world pos)
        CollisionLayer collisionLayer = DEFAULT;
        //Event OnCollisionHit;
        explicit Collideable(EntityID _entityId, BoundingBox _boundingBox) :
            Component(_entityId), localBoundingBox(_boundingBox), worldBoundingBox(_boundingBox) {}
    };

    struct CollisionInfo
    {
        EntityID collidedEntityId{};
        RayCollision rayCollision{};
    };
}

