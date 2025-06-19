#pragma once
#include "mygl.h"
#include "camera.hpp"
#include "model.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <glm/glm.hpp>

// Hash function for 3D grid coordinates
struct GridCoordHash {
    std::size_t operator()(const glm::ivec3& coord) const {
        // Simple hash combining x, y, z coordinates
        return std::hash<int>()(coord.x) ^
               (std::hash<int>()(coord.y) << 1) ^
               (std::hash<int>()(coord.z) << 2);
    }
};

// Object handle - could be an entity ID in your ECS
using ObjectHandle = uint32_t;

struct SpatialObject {
    ObjectHandle handle;
    AABB aabb;
    void* userData; // Pointer to your actual object (Model*, etc.)
};

class SpatialHashGrid {
private:
float distanceToPlane(const glm::vec4& plane, const glm::vec3& point) {
    return glm::dot(glm::vec3(plane), point) + plane.w;
}

bool isAABBOutsidePlane(const AABB& aabb, const glm::vec4& plane) {
    // Get the vertex closest to the plane (negative vertex)
    glm::vec3 negativeVertex;
    negativeVertex.x = (plane.x < 0.0f) ? aabb.max.x : aabb.min.x;
    negativeVertex.y = (plane.y < 0.0f) ? aabb.max.y : aabb.min.y;
    negativeVertex.z = (plane.z < 0.0f) ? aabb.max.z : aabb.min.z;

    // If the closest vertex is outside (negative distance), the entire AABB is outside
    return distanceToPlane(plane, negativeVertex) < 0.0f;
}

// Test if AABB is within or intersects the frustum
bool isAABBInFrustum(const AABB& aabb, const Frustum& frustum) {
    // Test against all 6 frustum planes
    for (int i = 0; i < 6; i++) {
        if (isAABBOutsidePlane(aabb, frustum.planes[i])) {
            return false; // AABB is completely outside this plane
        }
    }
    return true; // AABB is inside or intersects the frustum
}




    float cellSize;
    std::unordered_map<glm::ivec3, std::vector<ObjectHandle>, GridCoordHash> grid;
    std::unordered_map<ObjectHandle, SpatialObject> objects;
    std::unordered_map<ObjectHandle, std::unordered_set<glm::ivec3, GridCoordHash>> objectCells;

    // Convert world position to grid coordinate
    glm::ivec3 worldToGrid(const glm::vec3& worldPos) const {
        return glm::ivec3(
            static_cast<int>(std::floor(worldPos.x / cellSize)),
            static_cast<int>(std::floor(worldPos.y / cellSize)),
            static_cast<int>(std::floor(worldPos.z / cellSize))
        );
    }

    // Get all grid cells that an AABB spans
    std::vector<glm::ivec3> getAABBCells(const AABB& aabb) const {
        glm::ivec3 minCell = worldToGrid(aabb.min);
        glm::ivec3 maxCell = worldToGrid(aabb.max);

        std::vector<glm::ivec3> cells;
        for (int x = minCell.x; x <= maxCell.x; x++) {
            for (int y = minCell.y; y <= maxCell.y; y++) {
                for (int z = minCell.z; z <= maxCell.z; z++) {
                    cells.emplace_back(x, y, z);
                }
            }
        }
        return cells;
    }

    // Calculate AABB of camera frustum (approximation)
    AABB calculateFrustumAABB(const Camera& camera, float nearPlane, float farPlane) const {
        // Get frustum corners at near and far planes
        float halfFovY = glm::radians(camera.Zoom) * 0.5f;
        float aspect = 1200.0f / 800.0f; // You might want to pass this in

        float nearHeight = 2.0f * std::tan(halfFovY) * nearPlane;
        float nearWidth = nearHeight * aspect;
        float farHeight = 2.0f * std::tan(halfFovY) * farPlane;
        float farWidth = farHeight * aspect;

        // Calculate frustum corners in world space
        glm::vec3 nearCenter = camera.Position + camera.Front * nearPlane;
        glm::vec3 farCenter = camera.Position + camera.Front * farPlane;

        AABB frustumAABB;

        // Near plane corners
        std::vector<glm::vec3> corners = {
            nearCenter + camera.Up * (nearHeight * 0.5f) - camera.Right * (nearWidth * 0.5f),
            nearCenter + camera.Up * (nearHeight * 0.5f) + camera.Right * (nearWidth * 0.5f),
            nearCenter - camera.Up * (nearHeight * 0.5f) - camera.Right * (nearWidth * 0.5f),
            nearCenter - camera.Up * (nearHeight * 0.5f) + camera.Right * (nearWidth * 0.5f),
            // Far plane corners
            farCenter + camera.Up * (farHeight * 0.5f) - camera.Right * (farWidth * 0.5f),
            farCenter + camera.Up * (farHeight * 0.5f) + camera.Right * (farWidth * 0.5f),
            farCenter - camera.Up * (farHeight * 0.5f) - camera.Right * (farWidth * 0.5f),
            farCenter - camera.Up * (farHeight * 0.5f) + camera.Right * (farWidth * 0.5f)
        };

        for (const auto& corner : corners) {
            frustumAABB.expand(corner);
        }

        return frustumAABB;
    }

public:
    explicit SpatialHashGrid(float cellSize = 10.0f) : cellSize(cellSize) {}

    // Add an object to the spatial hash
    void addObject(ObjectHandle handle, const AABB& aabb, void* userData = nullptr) {
        // Store object data
        objects[handle] = {handle, aabb, userData};

        // Get cells this object spans
        auto cells = getAABBCells(aabb);
        objectCells[handle] = std::unordered_set<glm::ivec3, GridCoordHash>(cells.begin(), cells.end());

        // Add to grid cells
        for (const auto& cell : cells) {
            grid[cell].push_back(handle);
        }
    }

    // Remove object from spatial hash
    void removeObject(ObjectHandle handle) {
        auto objectIt = objects.find(handle);
        if (objectIt == objects.end()) return;

        // Remove from all cells
        auto cellsIt = objectCells.find(handle);
        if (cellsIt != objectCells.end()) {
            for (const auto& cell : cellsIt->second) {
                auto& cellObjects = grid[cell];
                cellObjects.erase(
                    std::remove(cellObjects.begin(), cellObjects.end(), handle),
                    cellObjects.end()
                );

                // Clean up empty cells
                if (cellObjects.empty()) {
                    grid.erase(cell);
                }
            }
            objectCells.erase(cellsIt);
        }

        objects.erase(objectIt);
    }

    // Update object position (removes and re-adds)
    void updateObject(ObjectHandle handle, const AABB& newAABB) {
        auto objectIt = objects.find(handle);
        if (objectIt == objects.end()) return;

        void* userData = objectIt->second.userData;
        removeObject(handle);
        addObject(handle, newAABB, userData);
    }

    // Query objects potentially visible to camera (coarse culling)
    std::vector<ObjectHandle> queryFrustum(const Camera& camera, float nearPlane = 0.1f, float farPlane = 100.0f) const {
        // Get frustum AABB
        AABB frustumAABB = calculateFrustumAABB(camera, nearPlane, farPlane);

        // Get all cells the frustum intersects
        auto frustumCells = getAABBCells(frustumAABB);

        // Collect unique objects from intersected cells
        std::unordered_set<ObjectHandle> candidateObjects;
        for (const auto& cell : frustumCells) {
            auto cellIt = grid.find(cell);
            if (cellIt != grid.end()) {
                for (ObjectHandle handle : cellIt->second) {
                    candidateObjects.insert(handle);
                }
            }
        }

        return std::vector<ObjectHandle>(candidateObjects.begin(), candidateObjects.end());
    }

    // Get object data
    const SpatialObject* getObject(ObjectHandle handle) const {
        auto it = objects.find(handle);
        return (it != objects.end()) ? &it->second : nullptr;
    }

    // Debug info
    size_t getObjectCount() const { return objects.size(); }
    size_t getCellCount() const { return grid.size(); }
    float getCellSize() const { return cellSize; }

    // Clear all objects
    void clear() {
        grid.clear();
        objects.clear();
        objectCells.clear();
    }



    // Helper function to perform complete culling pipeline
    std::vector<ObjectHandle> cullObjects(
        const Camera& camera,
        float nearPlane = 0.1f,
        float farPlane = 100.0f) {

        // Step 1: Coarse culling with spatial hash
        auto candidates = queryFrustum(camera, nearPlane, farPlane);

        // Step 2: Precise frustum culling
        std::vector<ObjectHandle> visibleObjects;
        for (ObjectHandle handle : candidates) {
            const SpatialObject* obj = getObject(handle);
            if (obj && isAABBInFrustum(obj->aabb, camera.ViewFrustum)) {
                visibleObjects.push_back(handle);
            }
        }


        if (visibleObjects.size() == 0 ) {
            // exit(0);
            std::cout << "No visible objects found." << std::endl;
        }

        return visibleObjects;
    }
};
