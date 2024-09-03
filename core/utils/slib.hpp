//
// Created by Steve Wheeler on 06/07/2024.
//

#pragma once

#include "raylib.h"

#include <string>

namespace sage
{
    struct MaterialPaths
    {
        std::string diffuse{};
        std::string specular{};
        std::string normal{};
        Color emission;

        template <typename Archive>
        void serialize(Archive& archive)
        {
            archive(diffuse, specular, normal, emission);
        };
    };

    // Allowed unsafe access to model
    class Renderable;            // Forward dec for friend (Needed for serialisation)
    class ResourceManager;       // Forward dec for friend (Needs friend due to deep copy (could move that here))
    class TextureTerrainOverlay; // Forward dec for friend (Changes mesh data on the fly)

    /**
     * Defines a memory safe wrapper for raylib model. Does not safeguard against access to underlying pointers.
     */
    class ModelSafe
    {
        Model rlmodel{};

        void UnloadShaderLocs() const;
        void UnloadModelTextures() const;

      public:
        [[nodiscard]] BoundingBox CalculateModelBoundingBox();
        [[nodiscard]] RayCollision GetRayMeshCollision(Ray ray, int meshNum) const;
        void UpdateAnimation(ModelAnimation anim, int frame);
        void Draw(Vector3 position, float scale, Color tint);
        void Draw(Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint);
        [[nodiscard]] int GetMeshCount() const;
        [[nodiscard]] int GetMaterialCount() const;
        [[nodiscard]] Matrix GetTransform() const;
        void SetTransform(Matrix trans);
        void SetTexture(Texture texture, int materialIdx, MaterialMapIndex mapIdx) const;
        [[nodiscard]] Shader GetShader(int materialIdx) const;
        void SetShader(Shader shader, int materialIdx) const;
        ModelSafe(const ModelSafe&) = delete;
        ModelSafe& operator=(const ModelSafe&) = delete;
        ModelSafe(ModelSafe&& other) noexcept;
        ModelSafe& operator=(ModelSafe&& other) noexcept;
        ~ModelSafe();
        explicit ModelSafe(const char* path);
        explicit ModelSafe(Model& _model);
        ModelSafe() = default;

        friend class Renderable;
        friend class ResourceManager;
        friend class TextureTerrainOverlay;
    };

    Vector2 Vec3ToVec2(const Vector3& vec3);
    Vector3 NegateVector(const Vector3& vec3);
    Vector3 Vector3MultiplyByValue(const Vector3& vec3, float value);
    Image GenImageGradientRadialTrans(int width, int height, float density, Color inner, Color outer);
} // namespace sage
