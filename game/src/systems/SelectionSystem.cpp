#include "systems/SelectionSystem.hpp"

namespace lq
{
    entt::entity SelectionSystem::GetSelectedActor() const
    {
        return selectedActor;
    }

    void SelectionSystem::SetSelectedActor(const entt::entity actor)
    {
        if (selectedActor == actor) return;

        const auto old = selectedActor;
        selectedActor = actor;
        onSelectedActorChange.Publish(old, actor);
    }
} // namespace lq
