/*THIS FILE IS AUTO GENERATED*/
#include "resource_ids.hpp"

const char* resources::path(AssetId id) {
    switch (id) {
        case AssetId::Models_ford:
            return "resources/models/ford/scene.gltf";
        case AssetId::Models_pigeon:
            return "resources/models/pigeon/scene.gltf";
        case AssetId::Models_zombie:
            return "resources/models/zombie/scene.gltf";
        case AssetId::Models_gun:
            return "resources/models/gun/scene.gltf";
        case AssetId::Models_mustang:
            return "resources/models/mustang/scene.gltf";
        case AssetId::Models_medieval_town:
            return "resources/models/medieval_town/scene.gltf";
        case AssetId::Models_pheonix:
            return "resources/models/pheonix/scene.gltf";
        case AssetId::Models_buster_drone:
            return "resources/models/buster_drone/scene.gltf";
        case AssetId::Models_street_town:
            return "resources/models/street_town/scene.gltf";
        case AssetId::Shaders_animation_fragment:
            return "resources/shaders/animation_fragment.glsl";
        case AssetId::Shaders_fragment:
            return "resources/shaders/fragment.glsl";
        case AssetId::Shaders_animation_vertex:
            return "resources/shaders/animation_vertex.glsl";
        case AssetId::Shaders_particle_vertex:
            return "resources/shaders/particle_vertex.glsl";
        case AssetId::Shaders_text_vertex:
            return "resources/shaders/text_vertex.glsl";
        case AssetId::Shaders_vertex:
            return "resources/shaders/vertex.glsl";
        case AssetId::Shaders_particle_fragment:
            return "resources/shaders/particle_fragment.glsl";
        case AssetId::Shaders_text_fragment:
            return "resources/shaders/text_fragment.glsl";
        case AssetId::Fonts_Antonio_Bold:
            return "resources/fonts/Antonio-Bold.ttf";
        default:
            return nullptr;
    }
}
