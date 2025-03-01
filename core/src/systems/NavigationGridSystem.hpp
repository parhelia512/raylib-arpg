#pragma once

#include "BaseSystem.hpp"

#include "slib.hpp"

#include "components/NavigationGridSquare.hpp"

#include "entt/entt.hpp"
#include "raylib.h"
#include <memory>

namespace sage
{
    class CollisionSystem;
    struct GridSquare;

    enum class AStarHeuristic
    {
        DEFAULT,
        FAVOUR_RIGHT
    };

    class NavigationGridSystem : public BaseSystem
    {
        std::vector<std::pair<int, int>> directions = {
            {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {-1, 1}, {-1, -1}, {1, -1}};
        CollisionSystem* collisionSystem;
        std::vector<std::vector<std::unique_ptr<NavigationGridSquare>>> gridSquares;

        //---------------------------------------------------------
        [[nodiscard]] std::vector<Vector3> tracebackPath(
            const std::vector<std::vector<GridSquare>>& came_from,
            const GridSquare& start,
            const GridSquare& finish) const;
        //---------------------------------------------------------
        bool getExtents(entt::entity entity, GridSquare& extents) const;
        //---------------------------------------------------------
        [[nodiscard]] bool checkExtents(GridSquare square, GridSquare extents) const;
        //---------------------------------------------------------
        bool getExtents(Vector3 worldPos, GridSquare& extents) const;
        //---------------------------------------------------------
        void calculateTerrainHeightAndNormals(const entt::entity& entity);
        //---------------------------------------------------------
        std::pair<float, float> getHeightBounds(float slices);
        //---------------------------------------------------------
        void loadTerrainNormalMap(const ImageSafe& normalMap);
        //---------------------------------------------------------
        void loadTerrainHeightMap(const ImageSafe& heightMap);
        //---------------------------------------------------------

      public:
        float spacing{};
        int slices{};

        //---------------------------------------------------------
        void Init(int _slices, float _spacing);
        //---------------------------------------------------------
        void InitGridHeightAndNormals();
        //---------------------------------------------------------
        void GenerateNormalMap(ImageSafe& image);
        //---------------------------------------------------------
        void GenerateHeightMap(ImageSafe& image);
        //---------------------------------------------------------
        void PopulateGrid(const ImageSafe& heightMap, const ImageSafe& normalMap);
        //---------------------------------------------------------
        bool GetPathfindRange(
            const entt::entity& actorId, int bounds, GridSquare& minRange, GridSquare& maxRange) const;
        //---------------------------------------------------------
        bool GetGridRange(BoundingBox bb, int bounds, GridSquare& minRange, GridSquare& maxRange) const;
        //---------------------------------------------------------
        bool GetGridRange(Vector3 center, int bounds, GridSquare& minRange, GridSquare& maxRange) const;
        //---------------------------------------------------------
        bool GridToWorldSpace(GridSquare gridPos, Vector3& out) const;
        //---------------------------------------------------------
        bool WorldToGridSpace(Vector3 worldPos, GridSquare& out) const;
        //---------------------------------------------------------
        bool WorldToGridSpace(
            Vector3 worldPos, GridSquare& out, const GridSquare& _minRange, const GridSquare& _maxRange) const;
        //---------------------------------------------------------
        [[nodiscard]] GridSquare FindNextBestLocation(entt::entity entity, GridSquare target) const;
        //---------------------------------------------------------
        [[nodiscard]] GridSquare FindNextBestLocation(
            GridSquare currentPos,
            GridSquare target,
            GridSquare minRange,
            GridSquare maxRange,
            GridSquare extents) const;
        //---------------------------------------------------------
        [[nodiscard]] NavigationGridSquare* CastRay(
            int currentRow,
            int currentCol,
            Vector2 direction,
            float distance,
            std::vector<GridSquare>& debugLines) const;
        //---------------------------------------------------------
        [[nodiscard]] std::vector<Vector3> AStarPathfind(
            const entt::entity& entity,
            const Vector3& startPos,
            const Vector3& finishPos,
            AStarHeuristic heuristicType = AStarHeuristic::DEFAULT) const;
        //---------------------------------------------------------
        [[nodiscard]] std::vector<Vector3> AStarPathfind(
            const entt::entity& entity,
            const Vector3& startPos,
            const Vector3& finishPos,
            const GridSquare& minRange,
            const GridSquare& maxRange,
            AStarHeuristic heuristicType = AStarHeuristic::DEFAULT) const;
        //---------------------------------------------------------
        [[nodiscard]] std::vector<Vector3> BFSPathfind(
            const entt::entity& entity, const Vector3& startPos, const Vector3& finishPos) const;
        //---------------------------------------------------------
        [[nodiscard]] std::vector<Vector3> BFSPathfind(
            const entt::entity& entity,
            const Vector3& startPos,
            const Vector3& finishPos,
            const GridSquare& minRange,
            const GridSquare& maxRange) const;
        //---------------------------------------------------------
        const std::vector<std::vector<std::unique_ptr<NavigationGridSquare>>>& GetGridSquares();
        //---------------------------------------------------------
        [[nodiscard]] const NavigationGridSquare* GetGridSquare(int row, int col) const;
        //---------------------------------------------------------
        void DrawDebugPathfinding(const GridSquare& minRange, const GridSquare& maxRange) const;
        //---------------------------------------------------------
        void MarkSquareAreaOccupiedIfSteep(const BoundingBox& occupant, bool occupied);
        //---------------------------------------------------------
        void MarkSquareAreaOccupied(
            const BoundingBox& occupant, bool occupied, entt::entity occupantEntity = entt::null) const;
        //---------------------------------------------------------
        void MarkSquaresOccupied(const std::vector<GridSquare>& squares, bool occupied = true) const;
        //---------------------------------------------------------
        void MarkSquaresDebug(const std::vector<GridSquare>& squares, Color color, bool occupied = true) const;
        //---------------------------------------------------------
        [[nodiscard]] bool CheckWithinGridBounds(Vector3 worldPos) const;
        //---------------------------------------------------------
        [[nodiscard]] bool CheckWithinGridBounds(GridSquare square) const;
        //---------------------------------------------------------
        [[nodiscard]] bool CheckWithinBounds(Vector3 worldPos, GridSquare minRange, GridSquare maxRange) const;
        //---------------------------------------------------------
        [[nodiscard]] static bool CheckWithinBounds(GridSquare square, GridSquare minRange, GridSquare maxRange);
        //---------------------------------------------------------
        [[nodiscard]] bool CheckSingleSquareOccupied(Vector3 worldPos) const;
        //---------------------------------------------------------
        [[nodiscard]] bool CheckSingleSquareOccupied(GridSquare position) const;
        //---------------------------------------------------------
        [[nodiscard]] bool CheckBoundingBoxAreaUnoccupied(Vector3 worldPos, const BoundingBox& bb) const;
        //---------------------------------------------------------
        [[nodiscard]] bool CheckBoundingBoxAreaUnoccupied(GridSquare square, const BoundingBox& bb) const;
        //---------------------------------------------------------
        [[nodiscard]] entt::entity CheckSingleSquareOccupant(Vector3 worldPos) const;
        //---------------------------------------------------------
        [[nodiscard]] entt::entity CheckSingleSquareOccupant(GridSquare position) const;
        //---------------------------------------------------------
        [[nodiscard]] entt::entity CheckSquareAreaOccupant(Vector3 worldPos, const BoundingBox& bb) const;
        //---------------------------------------------------------
        [[nodiscard]] entt::entity CheckSquareAreaOccupant(GridSquare square, const BoundingBox& bb) const;
        //---------------------------------------------------------
        [[nodiscard]] bool CompareSquareAreaOccupant(entt::entity entity, const BoundingBox& bb) const;
        //---------------------------------------------------------
        [[nodiscard]] bool CompareSingleSquareOccupant(entt::entity entity, const BoundingBox& bb) const;
        //---------------------------------------------------------
        void DrawDebug() const;
        //---------------------------------------------------------
        explicit NavigationGridSystem(entt::registry* _registry, CollisionSystem* _collisionSystem);
    };
} // namespace sage
