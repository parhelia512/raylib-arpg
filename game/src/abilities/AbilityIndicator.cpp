#include "AbilityIndicator.hpp"

#include "engine/EngineSystems.hpp"
#include "engine/TextureTerrainOverlay.hpp"

namespace lq
{

    void AbilityIndicator::Init(Vector3 mouseRayHit)
    {
        indicatorTexture->Init(mouseRayHit);
    }

    void AbilityIndicator::Update(Vector3 mouseRayHit)
    {
        indicatorTexture->Update(mouseRayHit);
    }

    void AbilityIndicator::Enable(bool enable)
    {
        indicatorTexture->Enable(enable);
    }

    AbilityIndicator::AbilityIndicator(
        entt::registry* _registry, sage::EngineSystems* _engineSystems, const std::string& assetId)
        : indicatorTexture(
              std::make_unique<sage::TextureTerrainOverlay>(
                  _registry, _engineSystems, assetId, WHITE, "resources/shaders/glsl330/base.fs"))
    {
    }
} // namespace lq