//
// Created by Steve Wheeler on 31/07/2024.
//

#include "GameModeStateMachine.hpp"

#include <iostream>

namespace lq
{
    // ====== GameWaveState ===========================================================

    void GameModeStateMachine::onEnter(GameWaveState&, entt::entity)
    {
        std::cout << "Wave state entered! \n";
    }

    // ====== GameCombatState =========================================================

    void GameModeStateMachine::onEnter(GameCombatState&, entt::entity)
    {
        std::cout << "Combat state entered! \n";
    }

    // ====== Lifecycle ===============================================================

    void GameModeStateMachine::StartCombat()
    {
        ChangeState(GameCombatState{});
    }

    void GameModeStateMachine::Update()
    {
        auto& state = registry->get<GameState>(gameEntity);
        std::visit([this](auto& cur) { update(cur, gameEntity); }, state.current);
    }

    void GameModeStateMachine::Draw3D()
    {
    }

    GameModeStateMachine::GameModeStateMachine(entt::registry* _registry)
        : Base(_registry), gameEntity(_registry->create())
    {
        registry->emplace<GameState>(gameEntity);
    }
} // namespace lq
