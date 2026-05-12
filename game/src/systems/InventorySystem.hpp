//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include "engine/Event.hpp"

#include "entt/entt.hpp"
#include "raylib.h"
#include <unordered_map>

namespace lq
{
    class Systems;
    class Window;

    class InventorySystem
    {
        struct LastHit
        {
            Vector3 pos;
            bool reachable = false;
        };

        struct InventorySubscriptions
        {
            sage::Subscription onItemAdded;
            sage::Subscription onItemRemoved;
        };

        entt::registry* registry;
        Systems* sys;
        LastHit lastWorldItemHovered;
        std::unordered_map<entt::entity, InventorySubscriptions> inventorySubscriptions{};
        void onWorldItemClicked(entt::entity entity);
        void inventoryUpdated() const;
        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        [[nodiscard]] bool CheckWorldItemRange(bool hover = false);
        sage::Event<> onInventoryUpdated;
        InventorySystem(entt::registry* _registry, Systems* _sys);
    };

} // namespace lq
