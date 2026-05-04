//
// Created by Steve Wheeler on 03/08/2024.
//

#include "Explosion.hpp"

#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/systems/TransformSystem.hpp"

namespace lq
{
    void Explosion::Restart()
    {
        scale = 0;
        auto& r = registry->get<sage::Renderable>(entity);
        r.active = true;
    }

    void Explosion::Update()
    {
        if (scale >= maxScale) return;
        scale += increment * GetFrameTime();
        sys->transformSystem->SetScale(entity, scale);
        if (scale >= maxScale)
        {
            auto& r = registry->get<sage::Renderable>(entity);
            r.active = false;
        }
    }

    void Explosion::SetOrigin(Vector3 origin)
    {
        sys->transformSystem->SetPosition(entity, origin);
    }

    Explosion::Explosion(entt::registry* _registry, sage::EngineSystems* _sys) : sys(_sys)
    {
        registry = _registry;
        auto sphere = LoadModelFromMesh(GenMeshHemiSphere(1.0f, 16, 16));
        entity = registry->create();
        registry->emplace<sage::sgTransform>(entity);
        auto& renderable =
            registry->emplace<sage::Renderable>(entity, sage::ModelSafeUnique(sphere), MatrixIdentity());
        renderable.hint = Color{255, 0, 0, 100};
        // Procedural Renderable holds a ModelSafeUnique; downcast to reach public SetShader.
        static_cast<sage::ModelSafeUnique*>(renderable.GetModel())
            ->SetShader(
                sage::ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/base.fs"), 0);
    }
} // namespace lq