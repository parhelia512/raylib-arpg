//
// Created by Steve Wheeler on 30/12/2025.
//

#pragma once
#include "engine/components/BaseStateComponent.hpp"
#include "engine/Event.hpp"

#include "entt/entt.hpp"

#include <variant>
#include <vector>

namespace lq
{

    enum class PartyMemberStateEnum
    {
        Default,
        FollowingLeader,
        WaitingForLeader,
        DestinationUnreachable
    };

    class PartyMemberState : public sage::BaseStateComponent<PartyMemberState, PartyMemberStateEnum>
    {
      public:
        PartyMemberState() : BaseStateComponent(PartyMemberStateEnum::Default)
        {
        }
    };

    struct PlayerDefaultState
    {
    };

    struct PlayerMovingToLocationState
    {
    };

    struct PlayerMovingToAttackEnemyState
    {
    };

    struct PlayerMovingToTalkState
    {
        entt::entity target = entt::null;
    };

    struct PlayerMovingToLootState
    {
        entt::entity target = entt::null;
    };

    struct PlayerInDialogState
    {
        entt::entity target = entt::null;
    };

    struct PlayerCombatState
    {
    };

    struct PlayerState
    {
        using Variant = std::variant<
            PlayerDefaultState,
            PlayerMovingToLocationState,
            PlayerMovingToAttackEnemyState,
            PlayerMovingToTalkState,
            PlayerMovingToLootState,
            PlayerInDialogState,
            PlayerCombatState>;

        Variant current = PlayerDefaultState{};
        std::vector<sage::Subscription> subscriptions;

        void BindSubscription(sage::Subscription newSubscription)
        {
            subscriptions.push_back(std::move(newSubscription));
        }

        void RemoveAllSubscriptions()
        {
            for (auto& subscription : subscriptions)
            {
                subscription.UnSubscribe();
            }
            subscriptions.clear();
        }

        ~PlayerState()
        {
            RemoveAllSubscriptions();
        }

        PlayerState() = default;
        PlayerState(PlayerState&& other) noexcept = default;
        PlayerState& operator=(PlayerState&& other) noexcept = default;
        PlayerState(const PlayerState&) = delete;
        PlayerState& operator=(const PlayerState&) = delete;
    };

    enum class GameStateEnum
    {
        Default,
        Wave, // TODO: Can remove this now
        Combat
    };

    class GameState : public sage::BaseStateComponent<GameState, GameStateEnum>
    {
      public:
        GameState() : BaseStateComponent(GameStateEnum::Default)
        {
        }
    };

    enum class WavemobStateEnum
    {
        Default,
        TargetOutOfRange,
        Combat,
        Dying
    };

    class WavemobState : public sage::BaseStateComponent<WavemobState, WavemobStateEnum>
    {
      public:
        WavemobState() : BaseStateComponent(WavemobStateEnum::Default)
        {
        }
    };

    enum class AbilityStateEnum
    {
        IDLE,
        CURSOR_SELECT,
        AWAITING_EXECUTION
    };

    class AbilityState : public sage::BaseStateComponent<AbilityState, AbilityStateEnum>
    {
      public:
        AbilityState() : BaseStateComponent(AbilityStateEnum::IDLE)
        {
        }
    };

} // namespace lq
