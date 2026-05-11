#include "ui/UIBindings.hpp"

#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/PartyMemberComponent.hpp"
#include "GameObjectFactory.hpp"
#include "GameUI.hpp"
#include "GameUiFactory.hpp"
#include "Systems.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"

#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/Cursor.hpp"
#include "engine/ResourceManager.hpp"

#include "raymath.h"

#include <format>

namespace lq
{
    void BindPlayerAbilitySlot(
        LeverUIEngine* engine,
        PlayerAbilitySystem* playerAbilitySystem,
        const Texture emptySlotTex,
        AbilitySlot& slot,
        const unsigned int slotNumber)
    {
        auto* uiEngine = engine;
        auto* abilities = playerAbilitySystem;
        auto* slotPtr = &slot;
        const auto fallbackTex = emptySlotTex;

        slot.iconProvider = [abilities, slotNumber, fallbackTex]() {
            if (const auto* ability = abilities->GetAbility(slotNumber))
            {
                return sage::ResourceManager::GetInstance().TextureLoad(ability->icon);
            }

            return fallbackTex;
        };
        slot.isInteractiveProvider = [abilities, slotNumber]() {
            return abilities->GetAbility(slotNumber) != nullptr;
        };
        slot.cooldownReadyProvider = [abilities, slotNumber]() {
            if (const auto* ability = abilities->GetAbility(slotNumber))
            {
                return ability->CooldownReady();
            }

            return true;
        };
        slot.tooltipFactory = [uiEngine, abilities, slotNumber](Vector2 pos) -> sage::TooltipWindow* {
            if (const auto* ability = abilities->GetAbility(slotNumber))
            {
                return GameUiFactory::CreateAbilityToolTip(uiEngine, *ability, pos);
            }

            return nullptr;
        };

        slot.onClicked.Subscribe([abilities, slotNumber]() { abilities->PressAbility(slotNumber); });
        slot.onSwapRequested.Subscribe([abilities, slotPtr](AbilitySlot* dropped) {
            if (!dropped || dropped == slotPtr) return;

            abilities->SwapAbility(slotPtr->slotNumber, dropped->slotNumber);
            slotPtr->RetrieveInfo();
            dropped->RetrieveInfo();
        });

        uiEngine->sys->engine.cursor->onSelectedActorChange.Subscribe(
            [slotPtr](entt::entity, entt::entity) { slotPtr->RetrieveInfo(); });

        slot.RetrieveInfo();
    }
} // namespace lq
namespace lq
{
    namespace
    {
        Texture loadItemIcon(entt::registry* registry, const entt::entity itemId)
        {
            const auto& item = registry->get<ItemComponent>(itemId);
            return sage::ResourceManager::GetInstance().TextureLoad(item.icon);
        }

        Texture loadEquipmentEmptyTexture(const EquipmentSlotName itemType)
        {
            switch (itemType)
            {
                case EquipmentSlotName::HELM:
                    return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/helm.png");
                case EquipmentSlotName::ARMS:
                    return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/arms.png");
                case EquipmentSlotName::CHEST:
                    return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/chest.png");
                case EquipmentSlotName::BELT:
                    return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/belt.png");
                case EquipmentSlotName::BOOTS:
                    return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/boots.png");
                case EquipmentSlotName::LEGS:
                    return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/legs.png");
                case EquipmentSlotName::LEFTHAND:
                    return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/mainhand.png");
                case EquipmentSlotName::RIGHTHAND:
                    return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/offhand.png");
                case EquipmentSlotName::RING1:
                case EquipmentSlotName::RING2:
                    return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/ring.png");
                case EquipmentSlotName::AMULET:
                    return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/amulet.png");
                case EquipmentSlotName::COUNT:
                    break;
            }

            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/empty-inv_slot.png");
        }

        bool validateEquipmentDrop(const EquipmentSlotName itemType, const ItemComponent& item)
        {
            if (item.HasFlag(ItemFlags::WEAPON))
            {
                if (itemType == EquipmentSlotName::LEFTHAND)
                {
                    return true;
                }

                const ItemFlags rightHandRestrictedFlags =
                    ItemFlags::MAIN_HAND_ONLY | ItemFlags::TWO_HANDED | ItemFlags::BOW | ItemFlags::CROSSBOW;
                return itemType == EquipmentSlotName::RIGHTHAND && !item.HasAnyFlag(rightHandRestrictedFlags);
            }

            if (!item.HasFlag(ItemFlags::ARMOR))
            {
                return false;
            }

            switch (itemType)
            {
                case EquipmentSlotName::HELM:
                    return item.HasFlag(ItemFlags::HELMET);
                case EquipmentSlotName::AMULET:
                    return item.HasFlag(ItemFlags::AMULET);
                case EquipmentSlotName::CHEST:
                    return item.HasFlag(ItemFlags::CHEST);
                case EquipmentSlotName::BELT:
                    return item.HasFlag(ItemFlags::BELT);
                case EquipmentSlotName::ARMS:
                    return item.HasFlag(ItemFlags::ARMS);
                case EquipmentSlotName::LEGS:
                    return item.HasFlag(ItemFlags::LEGS);
                case EquipmentSlotName::BOOTS:
                    return item.HasFlag(ItemFlags::BOOTS);
                case EquipmentSlotName::RING1:
                case EquipmentSlotName::RING2:
                    return item.HasFlag(ItemFlags::RING);
                case EquipmentSlotName::LEFTHAND:
                case EquipmentSlotName::RIGHTHAND:
                case EquipmentSlotName::COUNT:
                    return false;
            }

            return false;
        }

        void bindItemPresentation(ItemSlot& slot, LeverUIEngine* engine)
        {
            auto* slotPtr = &slot;
            auto* registry = engine->registry;

            slot.iconProvider = [registry, slotPtr]() {
                const auto itemId = slotPtr->GetItemId();
                if (itemId == entt::null) return Texture{};
                return loadItemIcon(registry, itemId);
            };

            slot.tooltipFactory = [engine, registry, slotPtr](Vector2 pos, sage::Window* parentWindow) {
                const auto itemId = slotPtr->GetItemId();
                if (itemId == entt::null) return static_cast<sage::TooltipWindow*>(nullptr);

                auto& item = registry->get<ItemComponent>(itemId);
                return GameUiFactory::CreateItemTooltip(engine, item, parentWindow, pos);
            };
        }

        bool trySpawnItemInWorld(LeverUIEngine* engine, const entt::entity itemId)
        {
            if (itemId == entt::null) return false;

            const auto cursorPos = engine->cursor->getFirstNaviCollision();
            const auto selectedActor = engine->cursor->GetSelectedActor();
            const auto playerPos = engine->registry->get<sage::sgTransform>(selectedActor).GetWorldPos();
            const auto dist = Vector3Distance(playerPos, cursorPos.point);

            if (const bool outOfRange = dist > ItemComponent::MAX_ITEM_DROP_RANGE; !cursorPos.hit || outOfRange)
            {
                engine->CreateErrorMessage("Out of range.");
                return false;
            }

            if (!GameObjectFactory::spawnItemInWorld(engine->registry, engine->sys, itemId, cursorPos.point))
            {
                engine->CreateErrorMessage("Item cannot be dropped.");
                return false;
            }

            return true;
        }

        void refreshDroppedSlots(ItemSlot* source, ItemSlot* destination, LeverUIEngine* engine)
        {
            source->RetrieveInfo();
            destination->RetrieveInfo();
            engine->BringClickedWindowToFront(destination->parent->GetWindow());
        }
    } // namespace

    void BindInventorySlot(LeverUIEngine* engine, InventorySlot& slot, const bool followSelectedActor)
    {
        auto* slotPtr = &slot;
        auto* ui = engine;
        auto* registry = engine->registry;
        auto* equipmentSystem = engine->sys->equipmentSystem.get();

        bindItemPresentation(slot, engine);

        slot.emptyTextureProvider = []() {
            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/empty-inv_slot.png");
        };

        slot.itemProvider = [registry, slotPtr]() -> entt::entity {
            const auto owner = slotPtr->GetOwner();
            if (owner == entt::null || !registry->valid(owner) || !registry->all_of<InventoryComponent>(owner))
            {
                return entt::null;
            }

            const auto& inventory = registry->get<InventoryComponent>(owner);
            return inventory.GetItem(slotPtr->row, slotPtr->col);
        };

        slot.onDroppedToWorld.Subscribe([ui, registry, slotPtr](ItemSlot*) {
            const auto itemId = slotPtr->GetItemId();
            if (!trySpawnItemInWorld(ui, itemId)) return;

            auto& inventory = registry->get<InventoryComponent>(slotPtr->GetOwner());
            inventory.RemoveItem(slotPtr->row, slotPtr->col);
        });

        slot.onDroppedOnSlot.Subscribe([ui, registry, equipmentSystem](ItemSlot* source, ItemSlot* destination) {
            auto* destinationSlot = dynamic_cast<InventorySlot*>(destination);
            if (!destinationSlot) return;

            auto& destinationInventory = registry->get<InventoryComponent>(destinationSlot->GetOwner());

            if (auto* sourceSlot = dynamic_cast<InventorySlot*>(source))
            {
                const auto itemId = sourceSlot->GetItemId();
                if (itemId == entt::null) return;

                if (sourceSlot->GetOwner() == destinationSlot->GetOwner())
                {
                    destinationInventory.SwapItems(
                        destinationSlot->row, destinationSlot->col, sourceSlot->row, sourceSlot->col);
                }
                else
                {
                    auto& sourceInventory = registry->get<InventoryComponent>(sourceSlot->GetOwner());
                    if (!destinationInventory.AddItem(itemId, destinationSlot->row, destinationSlot->col))
                    {
                        ui->CreateErrorMessage("Inventory Full.");
                        return;
                    }
                    sourceInventory.RemoveItem(itemId);
                }

                refreshDroppedSlots(source, destination, ui);
            }
            else if (auto* sourceSlot = dynamic_cast<EquipmentSlot*>(source))
            {
                const auto owner = destinationSlot->GetOwner();
                if (!registry->valid(owner) || !registry->all_of<EquipmentComponent>(owner)) return;

                const auto droppedItemId = equipmentSystem->GetItem(owner, sourceSlot->itemType);
                if (droppedItemId == entt::null) return;

                equipmentSystem->DestroyItem(owner, sourceSlot->itemType);

                if (const auto inventoryItemId =
                        destinationInventory.GetItem(destinationSlot->row, destinationSlot->col);
                    inventoryItemId != entt::null)
                {
                    destinationInventory.RemoveItem(destinationSlot->row, destinationSlot->col);
                    equipmentSystem->EquipItem(owner, inventoryItemId, sourceSlot->itemType);
                }

                if (!destinationInventory.AddItem(droppedItemId, destinationSlot->row, destinationSlot->col))
                {
                    ui->CreateErrorMessage("Inventory Full.");
                }

                refreshDroppedSlots(source, destination, ui);
            }
        });

        if (followSelectedActor)
        {
            engine->sys->engine.cursor->onSelectedActorChange.Subscribe(
                [slotPtr](entt::entity, entt::entity current) { slotPtr->SetOwner(current); });
        }

        engine->sys->inventorySystem->onInventoryUpdated.Subscribe([slotPtr]() { slotPtr->RetrieveInfo(); });
        slot.RetrieveInfo();
    }

    void BindEquipmentSlot(LeverUIEngine* engine, EquipmentSlot& slot)
    {
        auto* slotPtr = &slot;
        auto* ui = engine;
        auto* registry = engine->registry;
        auto* equipmentSystem = engine->sys->equipmentSystem.get();
        auto* cursor = engine->sys->engine.cursor.get();

        bindItemPresentation(slot, engine);

        slot.emptyTextureProvider = [itemType = slot.itemType]() { return loadEquipmentEmptyTexture(itemType); };

        slot.itemProvider = [registry, equipmentSystem, cursor, itemType = slot.itemType]() -> entt::entity {
            const auto actor = cursor->GetSelectedActor();
            if (actor == entt::null || !registry->valid(actor) || !registry->all_of<EquipmentComponent>(actor))
            {
                return entt::null;
            }
            return equipmentSystem->GetItem(actor, itemType);
        };

        slot.onDroppedToWorld.Subscribe([ui, equipmentSystem, cursor, slotPtr](ItemSlot*) {
            const auto itemId = slotPtr->GetItemId();
            if (!trySpawnItemInWorld(ui, itemId)) return;

            equipmentSystem->DestroyItem(cursor->GetSelectedActor(), slotPtr->itemType);
        });

        slot.onDroppedOnSlot.Subscribe([ui, registry, equipmentSystem, cursor](ItemSlot* source, ItemSlot* destination) {
            auto* destinationSlot = dynamic_cast<EquipmentSlot*>(destination);
            if (!destinationSlot) return;

            if (auto* sourceSlot = dynamic_cast<InventorySlot*>(source))
            {
                const auto actor = cursor->GetSelectedActor();
                auto& inventory = registry->get<InventoryComponent>(sourceSlot->GetOwner());
                const auto itemId = inventory.GetItem(sourceSlot->row, sourceSlot->col);
                if (itemId == entt::null) return;

                const auto& item = registry->get<ItemComponent>(itemId);
                if (!validateEquipmentDrop(destinationSlot->itemType, item)) return;

                inventory.RemoveItem(sourceSlot->row, sourceSlot->col);
                equipmentSystem->MoveItemToInventory(actor, destinationSlot->itemType);
                equipmentSystem->EquipItem(actor, itemId, destinationSlot->itemType);
                refreshDroppedSlots(source, destination, ui);
            }
            else if (const auto* sourceSlot = dynamic_cast<EquipmentSlot*>(source))
            {
                // TODO: This still relies on EquipmentSystem validation for legal slot swaps.
                if (equipmentSystem->SwapItems(cursor->GetSelectedActor(), destinationSlot->itemType, sourceSlot->itemType))
                {
                    refreshDroppedSlots(source, destination, ui);
                }
            }
        });

        engine->sys->engine.cursor->onSelectedActorChange.Subscribe(
            [slotPtr](entt::entity, entt::entity) { slotPtr->RetrieveInfo(); });

        engine->sys->equipmentSystem->onEquipmentUpdated.Subscribe(
            [slotPtr](entt::entity) { slotPtr->RetrieveInfo(); });
        slot.RetrieveInfo();
    }
} // namespace lq
namespace lq
{
    namespace
    {
        entt::entity getSelectedActor(LeverUIEngine* engine)
        {
            return engine->sys->engine.cursor->GetSelectedActor();
        }

        bool hasComponent(entt::registry* registry, const entt::entity entity)
        {
            return entity != entt::null && registry->valid(entity);
        }

        std::string getStatText(
            entt::registry* registry,
            const entt::entity actor,
            const CharacterStatText::StatisticType statisticType)
        {
            if (!hasComponent(registry, actor)) return "";

            if (statisticType == CharacterStatText::StatisticType::NAME)
            {
                if (!registry->all_of<sage::Renderable>(actor)) return "";
                const auto& renderable = registry->get<sage::Renderable>(actor);
                return std::format("{}", renderable.GetVanityName());
            }

            if (!registry->all_of<CombatableActor>(actor)) return "";
            const auto& combatable = registry->get<CombatableActor>(actor);

            switch (statisticType)
            {
                case CharacterStatText::StatisticType::STRENGTH:
                    return std::format("Strength: {}", combatable.baseStatistics.strength);
                case CharacterStatText::StatisticType::AGILITY:
                    return std::format("Agility: {}", combatable.baseStatistics.agility);
                case CharacterStatText::StatisticType::INTELLIGENCE:
                    return std::format("Intelligence: {}", combatable.baseStatistics.intelligence);
                case CharacterStatText::StatisticType::CONSTITUTION:
                    return std::format("Constitution: {}", combatable.baseStatistics.constitution);
                case CharacterStatText::StatisticType::WITS:
                    return std::format("Wits: {}", combatable.baseStatistics.wits);
                case CharacterStatText::StatisticType::MEMORY:
                    return std::format("Memory: {}", combatable.baseStatistics.memory);
                case CharacterStatText::StatisticType::NAME:
                case CharacterStatText::StatisticType::COUNT:
                    break;
            }

            return "";
        }

        RenderTexture* getSelectedRenderTexture(LeverUIEngine* engine)
        {
            auto* registry = engine->registry;
            const auto actor = getSelectedActor(engine);
            if (!hasComponent(registry, actor) || !registry->all_of<EquipmentComponent>(actor)) return nullptr;

            return &registry->get<EquipmentComponent>(actor).renderTexture;
        }
    } // namespace

    void BindCharacterStatText(LeverUIEngine* engine, CharacterStatText& statText)
    {
        auto* statTextPtr = &statText;
        auto* ui = engine;
        auto* registry = engine->registry;

        statText.contentProvider = [ui, registry, statisticType = statText.statisticType]() {
            return getStatText(registry, getSelectedActor(ui), statisticType);
        };

        engine->sys->engine.cursor->onSelectedActorChange.Subscribe(
            [statTextPtr](entt::entity, entt::entity) { statTextPtr->RetrieveInfo(); });

        engine->sys->equipmentSystem->onEquipmentUpdated.Subscribe(
            [statTextPtr](entt::entity) { statTextPtr->RetrieveInfo(); });

        statText.RetrieveInfo();
    }

    void BindEquipmentCharacterPreview(LeverUIEngine* engine, EquipmentCharacterPreview& preview)
    {
        auto* previewPtr = &preview;
        auto* ui = engine;
        auto* equipmentSystem = engine->sys->equipmentSystem.get();

        preview.previewGenerator = [ui, equipmentSystem](const float width, const float height) {
            const auto actor = getSelectedActor(ui);
            if (!hasComponent(ui->registry, actor) || !ui->registry->all_of<EquipmentComponent>(actor)) return;

            equipmentSystem->GenerateRenderTexture(actor, width, height);
        };

        preview.renderTextureProvider = [ui]() { return getSelectedRenderTexture(ui); };

        engine->sys->engine.cursor->onSelectedActorChange.Subscribe(
            [previewPtr](entt::entity, entt::entity) { previewPtr->RetrieveInfo(); });

        engine->sys->equipmentSystem->onEquipmentUpdated.Subscribe(
            [previewPtr](entt::entity) { previewPtr->RetrieveInfo(); });

        preview.RetrieveInfo();
    }

    void BindPartyMemberPortrait(LeverUIEngine* engine, PartyMemberPortrait& portrait)
    {
        auto* portraitPtr = &portrait;
        auto* ui = engine;
        auto* registry = engine->registry;
        auto* cursor = engine->sys->engine.cursor.get();
        auto* partySystem = engine->sys->partySystem.get();
        auto* equipmentSystem = engine->sys->equipmentSystem.get();

        portrait.memberProvider = [partySystem, memberNumber = portrait.GetMemberNumber()]() {
            return partySystem->GetMember(memberNumber);
        };

        portrait.onEmptyHovered = [cursor]() {
            cursor->EnableContextSwitching();
            cursor->Enable();
        };

        portrait.isSelectedProvider = [cursor, portraitPtr]() {
            const auto member = portraitPtr->GetMember();
            return member != entt::null && cursor->GetSelectedActor() == member;
        };

        portrait.portraitProvider = [registry, equipmentSystem, portraitPtr](Texture& target) {
            const auto member = portraitPtr->GetMember();
            if (!hasComponent(registry, member) || !registry->all_of<PartyMemberComponent>(member)) return;

            equipmentSystem->GeneratePortraitRenderTexture(member, target.width * 4, target.height * 4);
            const auto& info = registry->get<PartyMemberComponent>(member);
            target.id = info.portraitImg.texture.id;
        };

        portrait.onPortraitClicked.Subscribe([cursor](PartyMemberPortrait* clickedPortrait) {
            const auto member = clickedPortrait->GetMember();
            if (member != entt::null) cursor->SetSelectedActor(member);
        });

        portrait.onDroppedOnPortrait.Subscribe(
            [ui, registry, cursor, equipmentSystem](PartyMemberPortrait* destination, sage::CellElement* droppedElement) {
                const auto receiver = destination->GetMember();
                if (!hasComponent(registry, receiver) || !registry->all_of<InventoryComponent>(receiver)) return;

                if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
                {
                    const auto sender = dropped->GetOwner();
                    if (receiver == sender) return;
                    if (!hasComponent(registry, sender) || !registry->all_of<InventoryComponent>(sender)) return;

                    auto& receiverInventory = registry->get<InventoryComponent>(receiver);
                    auto& senderInventory = registry->get<InventoryComponent>(sender);
                    const auto itemId = senderInventory.GetItem(dropped->row, dropped->col);
                    if (itemId == entt::null) return;

                    if (receiverInventory.AddItem(itemId))
                    {
                        senderInventory.RemoveItem(dropped->row, dropped->col);
                        dropped->RetrieveInfo();
                        destination->RetrieveInfo();
                    }
                    else
                    {
                        ui->CreateErrorMessage("Inventory full.");
                    }
                }
                else if (auto* dropped = dynamic_cast<EquipmentSlot*>(droppedElement))
                {
                    const auto sender = cursor->GetSelectedActor();
                    if (!hasComponent(registry, sender) || !registry->all_of<EquipmentComponent>(sender)) return;

                    auto& receiverInventory = registry->get<InventoryComponent>(receiver);
                    const auto droppedItemId = equipmentSystem->GetItem(sender, dropped->itemType);
                    if (droppedItemId == entt::null) return;

                    if (receiverInventory.AddItem(droppedItemId))
                    {
                        equipmentSystem->DestroyItem(sender, dropped->itemType);
                        dropped->RetrieveInfo();
                        destination->RetrieveInfo();
                    }
                    else
                    {
                        ui->CreateErrorMessage("Inventory full.");
                    }
                }
            });

        engine->sys->engine.cursor->onSelectedActorChange.Subscribe(
            [portraitPtr](entt::entity, entt::entity) { portraitPtr->RetrieveInfo(); });

        engine->sys->partySystem->onPartyChange.Subscribe([portraitPtr]() { portraitPtr->RetrieveInfo(); });

        engine->sys->equipmentSystem->onEquipmentUpdated.Subscribe(
            [portraitPtr](entt::entity) { portraitPtr->RetrieveInfo(); });

        portrait.RetrieveInfo();
    }
} // namespace lq
