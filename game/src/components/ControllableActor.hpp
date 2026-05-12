//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "engine/TextureTerrainOverlay.hpp"

#include <memory>

namespace lq
{
    class ControllableActor
    {
      public:
        std::unique_ptr<sage::TextureTerrainOverlay>
            selectedIndicator; // Initialised by ControllableActorSystem on creation
    };
} // namespace lq
