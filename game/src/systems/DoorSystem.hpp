//
// Created by steve on 30/12/2024.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{
    class EngineSystems;
}

namespace lq
{
    class DoorSystem
    {
        entt::registry* registry;
        sage::EngineSystems* sys;

      public:
        void UnlockDoor(entt::entity entity) const;
        void UnlockAndOpenDoor(entt::entity entity);
        void OpenClickedDoor(entt::entity entity) const;

        DoorSystem(entt::registry* _registry, sage::EngineSystems* _sys);
    };

} // namespace lq
