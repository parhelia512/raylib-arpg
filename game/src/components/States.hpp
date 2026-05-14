//
// Created by Steve Wheeler on 30/12/2025.
//

#pragma once
#include "engine/Event.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

#include <variant>
#include <vector>

namespace lq
{

    // =================================================================================
    // Player
    // =================================================================================

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
        std::vector<sage::Subscription> persistentSubscriptions;

        void BindSubscription(sage::Subscription newSubscription)
        {
            subscriptions.push_back(std::move(newSubscription));
        }

        void BindPersistentSubscription(sage::Subscription newSubscription)
        {
            persistentSubscriptions.push_back(std::move(newSubscription));
        }

        void RemoveAllSubscriptions()
        {
            for (auto& subscription : subscriptions)
            {
                subscription.UnSubscribe();
            }
            subscriptions.clear();
        }

        void RemovePersistentSubscriptions()
        {
            for (auto& subscription : persistentSubscriptions)
            {
                subscription.UnSubscribe();
            }
            persistentSubscriptions.clear();
        }

        ~PlayerState()
        {
            RemoveAllSubscriptions();
            RemovePersistentSubscriptions();
        }

        PlayerState() = default;
        PlayerState(PlayerState&& other) noexcept = default;
        PlayerState& operator=(PlayerState&& other) noexcept = default;
        PlayerState(const PlayerState&) = delete;
        PlayerState& operator=(const PlayerState&) = delete;
    };

    // =================================================================================
    // PartyMember
    // =================================================================================

    struct PartyMemberDefaultState
    {
    };

    struct PartyMemberFollowingLeaderState
    {
    };

    struct PartyMemberWaitingForLeaderState
    {
    };

    struct PartyMemberDestinationUnreachableState
    {
        Vector3 originalDestination{};
        double timeStart{};
        unsigned int tryCount = 0;
    };

    struct PartyMemberState
    {
        using Variant = std::variant<
            PartyMemberDefaultState,
            PartyMemberFollowingLeaderState,
            PartyMemberWaitingForLeaderState,
            PartyMemberDestinationUnreachableState>;

        Variant current = PartyMemberDefaultState{};
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

        ~PartyMemberState()
        {
            RemoveAllSubscriptions();
        }

        PartyMemberState() = default;
        PartyMemberState(PartyMemberState&& other) noexcept = default;
        PartyMemberState& operator=(PartyMemberState&& other) noexcept = default;
        PartyMemberState(const PartyMemberState&) = delete;
        PartyMemberState& operator=(const PartyMemberState&) = delete;
    };

    // =================================================================================
    // Wavemob
    // =================================================================================

    struct WavemobDefaultState
    {
    };

    struct WavemobTargetOutOfRangeState
    {
    };

    struct WavemobCombatState
    {
    };

    struct WavemobDyingState
    {
    };

    struct WavemobState
    {
        using Variant = std::variant<
            WavemobDefaultState,
            WavemobTargetOutOfRangeState,
            WavemobCombatState,
            WavemobDyingState>;

        Variant current = WavemobDefaultState{};
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

        ~WavemobState()
        {
            RemoveAllSubscriptions();
        }

        WavemobState() = default;
        WavemobState(WavemobState&& other) noexcept = default;
        WavemobState& operator=(WavemobState&& other) noexcept = default;
        WavemobState(const WavemobState&) = delete;
        WavemobState& operator=(const WavemobState&) = delete;
    };

    // =================================================================================
    // GameMode
    // =================================================================================

    struct GameDefaultState
    {
    };

    struct GameWaveState
    {
    };

    struct GameCombatState
    {
    };

    struct GameState
    {
        using Variant = std::variant<GameDefaultState, GameWaveState, GameCombatState>;

        Variant current = GameDefaultState{};
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

        ~GameState()
        {
            RemoveAllSubscriptions();
        }

        GameState() = default;
        GameState(GameState&& other) noexcept = default;
        GameState& operator=(GameState&& other) noexcept = default;
        GameState(const GameState&) = delete;
        GameState& operator=(const GameState&) = delete;
    };

    // =================================================================================
    // Ability
    // =================================================================================

    struct AbilityIdleState
    {
    };

    struct AbilityCursorSelectState
    {
    };

    struct AbilityAwaitingExecutionState
    {
    };

    struct AbilityState
    {
        using Variant =
            std::variant<AbilityIdleState, AbilityCursorSelectState, AbilityAwaitingExecutionState>;

        Variant current = AbilityIdleState{};
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

        ~AbilityState()
        {
            RemoveAllSubscriptions();
        }

        AbilityState() = default;
        AbilityState(AbilityState&& other) noexcept = default;
        AbilityState& operator=(AbilityState&& other) noexcept = default;
        AbilityState(const AbilityState&) = delete;
        AbilityState& operator=(const AbilityState&) = delete;
    };

} // namespace lq
