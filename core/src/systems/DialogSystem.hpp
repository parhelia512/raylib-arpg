﻿//
// Created by steve on 11/05/2024.
//

#pragma once

#include "systems/BaseSystem.hpp"

#include "components/DialogComponent.hpp"
#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class GameData;
    class sgTransform;
    class Window;

    class DialogSystem : public BaseSystem
    {
        GameData* gameData;
        Window* dialogWindow{};
        void endConversation() const;
        void progressConversation(const dialog::Conversation* conversation);

      public:
        void StartConversation(const sgTransform& cutscenePose, entt::entity npc);
        dialog::Conversation* GetConversation(entt::entity owner, ConversationID conversationId);
        explicit DialogSystem(entt::registry* registry, GameData* _gameData);
    };
} // namespace sage
