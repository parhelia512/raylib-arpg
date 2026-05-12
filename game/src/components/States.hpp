//
// Created by Steve Wheeler on 30/12/2025.
//

#pragma once
#include "engine/components/BaseStateComponent.hpp"
#include "engine/Event.hpp"

#include "entt/entt.hpp"

#include <type_traits>
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

    enum class PlayerStateEnum
    {
        Default,
        MovingToLocation,
        MovingToAttackEnemy,
        MovingToInteractionTarget,
        InDialog,
        Combat
    };

    enum class PlayerInteractionKind
    {
        Talk,
        Loot
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

    struct PlayerMovingToInteractionTargetState
    {
        PlayerInteractionKind kind{};
        entt::entity target = entt::null;
    };

    struct PlayerInDialogState
    {
        entt::entity target = entt::null;
    };

    struct PlayerCombatState
    {
    };

    class PlayerState
    {
        using StateVariant = std::variant<
            PlayerDefaultState,
            PlayerMovingToLocationState,
            PlayerMovingToAttackEnemyState,
            PlayerMovingToInteractionTargetState,
            PlayerInDialogState,
            PlayerCombatState>;

        StateVariant currentState = PlayerDefaultState{};
        std::vector<sage::Subscription> currentStateSubscriptions;

      public:
        void ManageSubscription(sage::Subscription newSubscription)
        {
            currentStateSubscriptions.push_back(std::move(newSubscription));
        }

        template <typename StateType>
        [[nodiscard]] bool Is() const
        {
            return std::holds_alternative<StateType>(currentState);
        }

        template <typename StateType>
        StateType& Get()
        {
            return std::get<StateType>(currentState);
        }

        template <typename StateType>
        const StateType& Get() const
        {
            return std::get<StateType>(currentState);
        }

        template <typename StateType>
        void Set(StateType nextState)
        {
            currentState = std::move(nextState);
        }

        void RemoveAllSubscriptions()
        {
            for (auto& subscription : currentStateSubscriptions)
            {
                subscription.UnSubscribe();
            }
            currentStateSubscriptions.clear();
        }

        [[nodiscard]] PlayerStateEnum GetCurrentStateEnum() const
        {
            return std::visit(
                []<typename T0>(const T0& state) {
                    using StateType = std::decay_t<T0>;
                    if constexpr (std::is_same_v<StateType, PlayerDefaultState>)
                    {
                        return PlayerStateEnum::Default;
                    }
                    else if constexpr (std::is_same_v<StateType, PlayerMovingToLocationState>)
                    {
                        return PlayerStateEnum::MovingToLocation;
                    }
                    else if constexpr (std::is_same_v<StateType, PlayerMovingToAttackEnemyState>)
                    {
                        return PlayerStateEnum::MovingToAttackEnemy;
                    }
                    else if constexpr (std::is_same_v<StateType, PlayerMovingToInteractionTargetState>)
                    {
                        return PlayerStateEnum::MovingToInteractionTarget;
                    }
                    else if constexpr (std::is_same_v<StateType, PlayerInDialogState>)
                    {
                        return PlayerStateEnum::InDialog;
                    }
                    else
                    {
                        return PlayerStateEnum::Combat;
                    }
                },
                currentState);
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
