//
// Created by steve on 05/11/2024.
//

#pragma once

#include "common_types.hpp"
#include "raylib.h"

#include "entt/entt.hpp"

#include <optional>

namespace lq
{
    struct PartyMemberComponent
    {
        bool mainCharacter = false;
        const entt::entity entity;
        std::optional<entt::entity> followTarget;
        RenderTexture portraitImg{};
        AssetID portraitImage{};
        explicit PartyMemberComponent(entt::entity _entity) : entity(_entity)
        {
        }
    };
} // namespace lq
