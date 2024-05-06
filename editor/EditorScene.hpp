﻿//
// Created by steve on 22/02/2024.
//

#pragma once

#include "../core/src/UserInput.hpp"
#include "../core/src/GameManager.hpp"
#include "scenes/Scene.hpp"

#include "entt/entt.hpp"
#include "windows/FloatingWindow.hpp"

#include <vector>
#include <memory>

namespace sage
{

enum EditorMode
{
    IDLE,
    SELECT,
    MOVE,
    CREATE
};

class EditorScene : public Scene
{
    EditorMode currentEditorMode = IDLE;
    
    editor::FloatingWindow guiWindow1;
    editor::FloatingWindow guiWindow2;
    
    // Event responses
    void OnCursorClick();
    void OnCollisionHit();
    void OnSerializeSave();
    void OnSerializeLoad();
    void OnDeleteModeKeyPressed();
    void OnCreateModeKeyPressed();
    void OnGenGridKeyPressed();
    
    entt::entity selectedObject{};
    void moveSelectedObjectToCursorHit();
    
public:
    EditorScene(entt::registry* _registry, Game* _ecs);
    ~EditorScene() override;
    void Update() override;
    void Draw3D() override;
    void Draw2D() override;
};

}



