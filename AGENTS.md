# AGENTS.md

This file provides guidance to Codex (Codex.ai/code) when working with code in this repository.

## Project Overview

'LeverQuest' — a BG3/Divinity: Original Sin 2-inspired CRPG vertical slice built in C++20 on top of raylib (used purely
for window management and rendering). The engine and game are built from scratch.

## Build

```bash
# Configure and build (CLion is the primary IDE, but standard CMake works)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Optional targets
cmake -S . -B build -DBUILD_EDITOR=ON -DBUILD_RESPACKER=ON
```

Build outputs:

- `cmake-build-debug/game/game` — the playable game
- `cmake-build-debug/respacker/respacker` — asset pipeline tool (Blender → binary)

There are **no automated tests** in this project.

## Architecture

Two namespaces with a strict dependency direction: `sage` (engine) → `lq` (game).

### `sage` namespace — `engine/`

Generic, reusable engine systems. `EngineSystems` is a plain struct of `std::unique_ptr` system instances plus a raw
`entt::registry*`. Systems are constructed with the registry and each other as needed — there is no DI container.

Key engine systems: `TransformSystem`, `RenderSystem`, `CollisionSystem`, `NavigationGridSystem`, `ActorMovementSystem`,
`AnimationSystem`, `UberShaderSystem`, `SpatialAudioSystem`, `DoorSystem`, `CursorClickIndicator`.

### `lq` namespace — `game/`

`lq::Systems` inherits from `sage::EngineSystems` and adds game-specific systems: `DialogSystem`, `CombatSystem`,
`PlayerAbilitySystem`, `InventorySystem`, `EquipmentSystem`, `PartySystem`, `QuestManager`, `ContextualDialogSystem`,
`LootSystem`, `NPCManager`, `HealthBarSystem`, `StateMachines`.

### ECS via EnTT

All game objects are `entt::entity` values in a single `entt::registry`. Components are plain structs added to entities;
systems query the registry with `registry->view<ComponentA, ComponentB>()`. Entities are created through factory
classes — `GameObjectFactory`, `ItemFactory`, `AbilityFactory`, `DialogFactory`.

### Scene management

`lq::Scene` (abstract base) owns a `lq::Systems*` and the entity population for a level. `ExampleScene` is the only
concrete scene. Scenes implement `Init()`, `Update()`, `Draw3D()`, `Draw2D()`.

### Transform hierarchy

`sgTransform` (in `engine/components/`) supports parent-child relationships. `TransformSystem` propagates dirty flags
down the hierarchy. Prefer `TransformSystem` methods over direct component mutation when moving entities.

### UI framework

Custom HTML-table-inspired layout in `engine/GameUiEngine.hpp`. Hierarchy:
`Window → Table/TableGrid → TableRow → TableCell → CellElement`. Supports drag-and-drop with a `UIState` enum (Idle →
Hover → DragDelay → Drag). The game-side `LeverUIEngine` extends this.

### Navigation / pathfinding

`NavigationGridSystem` maintains a height-mapped grid with occupancy tracking. Uses A* and BFS with configurable
heuristics (`DEFAULT`, `FAVOUR_RIGHT`). Multi-unit: extent checks prevent overlapping paths.

### Event system

`Event<T>` / `Subscription` in `engine/Event.hpp` — lightweight pub/sub used throughout for decoupled communication (
position updates, dialog triggers, combat events, etc.).

### Serialization

Cereal library (`vendor/cereal`). Components implement `save()`/`load()` templates. `engine/Serializer.hpp` provides
helpers for binary, JSON, and XML formats. `engine/raylib-cereal.hpp` bridges raylib math types to Cereal.

### Resource management

`ResourceManager` singleton caches models, textures, fonts, shaders, sounds, and music. Uses `ModelSafe`/`ImageSafe`
RAII wrappers (`engine/slib.hpp`) for deep-copy safety when raylib shares GPU handles.

### Dialog system

Custom markup language parsed at runtime from files in `resources/dialog/`. Supports branching, conditions, variable
tracking, and quest integration. `DialogFactory` creates dialog components on entities.

### Asset pipeline

`respacker/` tool compiles raw assets (models, textures) from `resources/` into packed binary files. Run respacker after
adding new assets from Blender.

## Key file locations

| Concern                    | Location                                                            |
|----------------------------|---------------------------------------------------------------------|
| Engine system declarations | `engine/EngineSystems.hpp`                                          |
| Game system declarations   | `game/src/Systems.hpp`                                              |
| Engine components          | `engine/components/`                                                |
| Game components            | `game/src/components/`                                              |
| Entity creation            | `game/src/GameObjectFactory.cpp`                                    |
| Scene setup                | `game/src/scenes/ExampleScene.cpp`                                  |
| Level loading              | `game/utils/MapLoader.cpp`                                          |
| Ability definitions        | `game/src/abilities/`                                               |
| Shared type utilities      | `game/utils/common_types.hpp`, `game/utils/enum_flag_operators.hpp` |
| Shader source              | `resources/shaders/`                                                |
