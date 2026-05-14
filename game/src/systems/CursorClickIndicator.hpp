#pragma once

#include "engine/Event.hpp"
#include "engine/CollisionLayers.hpp"

#include "entt/entt.hpp"

namespace lq
{
    class Systems;

    class CursorClickIndicator
    {
        entt::registry* registry;
        Systems* sys;
        entt::entity self;
        entt::entity selectedActor = entt::null;
        float k = 0.0f;

        sage::Subscription destinationReachedSub{};
        sage::Subscription cursorClickSub{};

        void onCursorClick(entt::entity entity, sage::CollisionLayer layer) const;
        void disableIndicator() const;

      public:
        void Update();
        void OnSelectedActorChanged(entt::entity previous, entt::entity current);
        CursorClickIndicator(entt::registry* _registry, Systems* _sys);
    };
} // namespace lq
