/*THIS FILE IS AUTO GENERATED*/
#include "resource_ids.hpp"

const char* resources::path(AssetId id) {
    switch (id) {
        case AssetId::Models_ford:
            return "resources/models/ford/scene.gltf";
        case AssetId::Models_pigeon:
            return "resources/models/pigeon/scene.gltf";
        case AssetId::Models_mustang:
            return "resources/models/mustang/scene.gltf";
        case AssetId::Shaders_fragment:
            return "resources/shaders/fragment.glsl";
        case AssetId::Shaders_particle_vertex:
            return "resources/shaders/particle_vertex.glsl";
        case AssetId::Shaders_vertex:
            return "resources/shaders/vertex.glsl";
        case AssetId::Shaders_particle_fragment:
            return "resources/shaders/particle_fragment.glsl";
        default:
            return nullptr;
    }
}
