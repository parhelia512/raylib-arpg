//
// Created by Steve Wheeler on 29/12/2024.
//

#pragma once

namespace sage
{
    class DoorBehaviorComponent
    {
        float openYRotation = -130;
        bool open = false;
        bool locked = true;

      public:
        DoorBehaviorComponent() = default;

        friend class DoorSystem;
    };

} // namespace sage
