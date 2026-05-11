//
// Created by steve on 02/10/2024.
//

#pragma once

#include "components/DialogComponent.hpp"

#include "engine/GameUiEngine.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

#include <functional>
#include <string>

namespace sage
{
    class Window;
}

namespace lq
{
    class LeverUIEngine;
    class Systems;
    enum class EquipmentSlotName;
    class QuestManager;

    // Also displays description
    class JournalEntryManager : public sage::TextBox
    {
        sage::TableCell* journalEntryRoot;
        QuestManager* questManager;

        void updateQuests();

      public:
        JournalEntryManager(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            sage::TableCell* _journalEntryRoot,
            const FontInfo& _fontInfo,
            sage::VertAlignment _vertAlignment = sage::VertAlignment::TOP,
            sage::HoriAlignment _horiAlignment = sage::HoriAlignment::LEFT);
    };

    class JournalEntry : public sage::TextBox
    {
        entt::entity questId{};
        Quest* quest;
        sage::TableCell* descriptionCell;
        bool drawHighlight = false;

      public:
        void OnHoverStart() override;
        void OnHoverStop() override;
        void Draw2D() override;
        void OnClick() override;
        JournalEntry(
            sage::GameUIEngine* _engine,
            sage::TableCell* _parent,
            sage::TableCell* _descriptionCell,
            Quest* _quest,
            const FontInfo& _fontInfo,
            sage::VertAlignment _vertAlignment = sage::VertAlignment::TOP,
            sage::HoriAlignment _horiAlignment = sage::HoriAlignment::LEFT);
    };

    class DialogOption : public sage::TextBox
    {
        dialog::Option* option;
        unsigned int index{};
        bool drawHighlight = false;

      public:
        void OnHoverStart() override;
        void OnHoverStop() override;
        void Draw2D() override;
        void OnClick() override;
        DialogOption(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            dialog::Option* _option,
            unsigned int _index,
            const FontInfo& _fontInfo,
            sage::VertAlignment _vertAlignment = sage::VertAlignment::TOP,
            sage::HoriAlignment _horiAlignment = sage::HoriAlignment::LEFT);
    };

    class CharacterStatText final : public sage::TextBox
    {
      public:
        enum class StatisticType
        {
            NAME,
            STRENGTH,
            AGILITY,
            INTELLIGENCE,
            CONSTITUTION,
            WITS,
            MEMORY,
            COUNT // must be last
        };
        StatisticType statisticType;
        std::function<std::string()> contentProvider;

        void RetrieveInfo() override;
        ~CharacterStatText() override = default;
        CharacterStatText(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            const FontInfo& _fontInfo,
            StatisticType _statisticType);
    };

    class ResourceOrb : public sage::ImageBox // Health, mana, etc.
    {

      public:
        void RetrieveInfo() override;
        void Draw2D() override;
        ResourceOrb(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            sage::VertAlignment _vertAlignment = sage::VertAlignment::TOP,
            sage::HoriAlignment _horiAlignment = sage::HoriAlignment::LEFT);
    };

    class EquipmentCharacterPreview : public sage::ImageBox
    {
      public:
        std::function<void(float width, float height)> previewGenerator;
        std::function<RenderTexture*()> renderTextureProvider;

        void UpdateDimensions() override;
        void RetrieveInfo() override;
        void Draw2D() override;
        EquipmentCharacterPreview(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            sage::VertAlignment _vertAlignment = sage::VertAlignment::TOP,
            sage::HoriAlignment _horiAlignment = sage::HoriAlignment::LEFT);
    };

    class PartyMemberPortrait : public sage::ImageBox
    {
        unsigned int memberNumber{};
        Texture portraitBgTex{};
        int width;
        int height;

      public:
        std::function<entt::entity()> memberProvider;
        std::function<void(Texture& target)> portraitProvider;
        std::function<bool()> isSelectedProvider;
        std::function<void()> onEmptyHovered;

        sage::Event<PartyMemberPortrait*> onPortraitClicked;
        sage::Event<PartyMemberPortrait*, sage::CellElement*> onDroppedOnPortrait;

        [[nodiscard]] unsigned int GetMemberNumber() const;
        [[nodiscard]] entt::entity GetMember() const;
        void HoverUpdate() override;
        void UpdateDimensions() override;
        void RetrieveInfo() override;
        void ReceiveDrop(CellElement* droppedElement) override;
        void OnClick() override;
        void Draw2D() override;
        PartyMemberPortrait(
            LeverUIEngine* _engine, sage::TableCell* _parent, unsigned int _memberNumber, int _width, int _height);
        friend class sage::TableCell;
    };

    class DialogPortrait : public sage::ImageBox
    {
      public:
        void Draw2D() override;
        DialogPortrait(LeverUIEngine* _engine, sage::TableCell* _parent, const Texture& _tex);
        friend class sage::TableCell;
    };

    class AbilitySlot : public sage::ImageBox
    {
      public:
        // Identity tag used by the factory to wire callbacks, and by drag/drop to
        // distinguish source slots in a swap. Pure data — no system coupling.
        unsigned int slotNumber{};

        // Providers — caller (factory) supplies these to bridge to whichever game
        // system owns the underlying data. Slot calls them on demand.
        std::function<Texture()> iconProvider;
        std::function<bool()> isInteractiveProvider;
        std::function<bool()> cooldownReadyProvider;
        std::function<sage::TooltipWindow*(Vector2 pos)> tooltipFactory;

        // Events — factory subscribes to these to perform game-side effects.
        sage::Event<> onClicked;
        sage::Event<AbilitySlot*> onSwapRequested;

        void RetrieveInfo() override;
        void ReceiveDrop(CellElement* droppedElement) override;
        void HoverUpdate() override;
        void Draw2D() override;
        void OnClick() override;
        AbilitySlot(LeverUIEngine* _engine, sage::TableCell* _parent, unsigned int _slotNumber);
        friend class sage::TableCell;
    };

    class ItemSlot : public sage::ImageBox
    {
      protected:
        Texture backgroundTex{};
        void dropItemInWorld();
        void updateRectangle(
            const sage::Dimensions& dimensions, const Vector2& offset, const sage::Dimensions& space) override;

        [[nodiscard]] Texture getEmptyTex() const;

      public:
        std::function<entt::entity()> itemProvider;
        std::function<Texture()> iconProvider;
        std::function<Texture()> emptyTextureProvider;
        std::function<sage::TooltipWindow*(Vector2 pos, sage::Window* parentWindow)> tooltipFactory;

        sage::Event<ItemSlot*> onDroppedToWorld;
        sage::Event<ItemSlot*, ItemSlot*> onDroppedOnSlot;

        [[nodiscard]] entt::entity GetItemId() const;
        void Draw2D() override;
        void RetrieveInfo() override;
        void OnDrop(CellElement* receiver) override;
        void ReceiveDrop(CellElement* droppedElement) override;
        void HoverUpdate() override;
        ItemSlot(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            sage::VertAlignment _vertAlignment = sage::VertAlignment::TOP,
            sage::HoriAlignment _horiAlignment = sage::HoriAlignment::LEFT);
        friend class sage::TableCell;
    };

    class EquipmentSlot : public ItemSlot
    {
      public:
        EquipmentSlotName itemType;
        EquipmentSlot(LeverUIEngine* _engine, sage::TableCell* _parent, EquipmentSlotName _itemType);
        friend class sage::TableCell;
    };

    class InventorySlot : public ItemSlot
    {
      protected:
        entt::entity owner{};

      public:
        unsigned int row{};
        unsigned int col{};
        [[nodiscard]] entt::entity GetOwner() const;
        void SetOwner(entt::entity _owner);
        InventorySlot(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            entt::entity _owner,
            unsigned int _row,
            unsigned int _col);
        friend class sage::TableCell;
    };

    class LeverUIEngine : public sage::GameUIEngine
    {
        void onWorldItemHover(entt::entity entity);
        void onNPCHover(entt::entity entity);
        void onStopWorldHover() const;

      public:
        Systems* sys{};
        LeverUIEngine(entt::registry* _registry, Systems* _sys);
    };

} // namespace lq
