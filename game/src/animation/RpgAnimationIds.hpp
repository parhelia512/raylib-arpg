#pragma once

#include "engine/AnimationId.hpp"

namespace lq::animation_ids
{
    inline constexpr sage::AnimationId Idle = sage::MakeAnimationId(0);
    inline constexpr sage::AnimationId Idle2 = sage::MakeAnimationId(1);
    inline constexpr sage::AnimationId Death = sage::MakeAnimationId(2);
    inline constexpr sage::AnimationId AutoAttack = sage::MakeAnimationId(3);
    inline constexpr sage::AnimationId Walk = sage::MakeAnimationId(4);
    inline constexpr sage::AnimationId Talk = sage::MakeAnimationId(5);
    inline constexpr sage::AnimationId Spin = sage::MakeAnimationId(6);
    inline constexpr sage::AnimationId Slash = sage::MakeAnimationId(7);
    inline constexpr sage::AnimationId Run = sage::MakeAnimationId(8);
    inline constexpr sage::AnimationId SpellcastForward = sage::MakeAnimationId(9);
    inline constexpr sage::AnimationId SpellcastUp = sage::MakeAnimationId(10);
    inline constexpr sage::AnimationId Roll = sage::MakeAnimationId(11);
} // namespace lq::animation_ids
