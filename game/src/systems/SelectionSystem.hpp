#pragma once

#include "engine/Event.hpp"

#include "entt/entt.hpp"

namespace lq
{
    class SelectionSystem
    {
        entt::entity selectedActor = entt::null;

      public:
        sage::Event<entt::entity, entt::entity> onSelectedActorChange{}; // prev, current

        [[nodiscard]] entt::entity GetSelectedActor() const;
        void SetSelectedActor(entt::entity actor);
    };
} // namespace lq
