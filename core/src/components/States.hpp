#pragma once

#include "raylib.h"

#include <entt/entt.hpp>
#include <Event.hpp>
#include <vector>

namespace sage
{
    template <typename StateName, typename StateEnum>
    class BaseState
    {
        StateEnum currentState;
        std::vector<entt::connection> currentStateConnections;

      public:
        void AddConnection(entt::connection newConnection)
        {
            currentStateConnections.push_back(newConnection);
        }

        void RemoveAllConnections()
        {
            for (auto& connection : currentStateConnections)
            {
                connection.release();
            }
        }

        void SetState(StateEnum newState)
        {
            currentState = newState;
        }

        [[nodiscard]] StateEnum GetCurrentState() const
        {
            return currentState;
        }

        ~BaseState()
        {
            RemoveAllConnections();
        }

        // BaseState(const BaseState&) = delete;
        // BaseState& operator=(const BaseState&) = delete;

        explicit BaseState(StateEnum initialState) : currentState(initialState)
        {
        }
    };

    enum class PartyMemberStateEnum
    {
        Default,
        FollowingLeader,
        WaitingForLeader,
        DestinationUnreachable
    };

    class PartyMemberState : public BaseState<PartyMemberState, PartyMemberStateEnum>
    {
      public:
        Connection<entt::entity> onLeaderMoveCnx{};
        Connection<entt::entity> onDestinationReachedCnx{};
        Connection<entt::entity, entt::entity> onTargetPathChangedCnx{};
        Connection<entt::entity> onMovementCancelCnx{};
        Connection<entt::entity, Vector3> onDestinationUnreachableCnx{};

        std::unique_ptr<Event<entt::entity, entt::entity>> onLeaderMove; // self, leader

        PartyMemberState()
            : BaseState(PartyMemberStateEnum::Default),
              onLeaderMove(std::make_unique<Event<entt::entity, entt::entity>>())
        {
        }
    };

    enum class PlayerStateEnum
    {
        Default,
        MovingToLocation,
        MovingToAttackEnemy,
        MovingToTalkToNPC,
        InDialog,
        DestinationUnreachable,
        Combat
    };

    class PlayerState : public BaseState<PlayerState, PlayerStateEnum>
    {
      public:
        PlayerState() : BaseState(PlayerStateEnum::Default)
        {
        }
    };

    enum class GameStateEnum
    {
        Default,
        Wave, // TODO: Can remove this now
        Combat
    };

    class GameState : public BaseState<GameState, GameStateEnum>
    {
      public:
        GameState() : BaseState(GameStateEnum::Default)
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

    class WavemobState : public BaseState<WavemobState, WavemobStateEnum>
    {
      public:
        WavemobState() : BaseState(WavemobStateEnum::Default)
        {
        }
    };

    enum class AbilityStateEnum
    {
        IDLE,
        CURSOR_SELECT,
        AWAITING_EXECUTION
    };

    class AbilityState : public BaseState<AbilityState, AbilityStateEnum>
    {
      public:
        AbilityState() : BaseState(AbilityStateEnum::IDLE)
        {
        }
    };

} // namespace sage