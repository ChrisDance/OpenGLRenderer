/*THIS FILE IS AUTO GENERATED*/
#ifndef RESOURCE_IDS_HPP
#define RESOURCE_IDS_HPP

#include <cstdint>

namespace resources {

enum AssetId : unsigned int{
    None = 0,
    Models_ford = 1,
    Models_pigeon = 2,
    Models_zombie = 3,
    Models_gun = 4,
    Models_mustang = 5,
    Models_medieval_town = 6,
    Models_pheonix = 7,
    Models_buster_drone = 8,
    Models_street_town = 9,
    Shaders_animation_fragment = 10,
    Shaders_fragment = 11,
    Shaders_animation_vertex = 12,
    Shaders_particle_vertex = 13,
    Shaders_text_vertex = 14,
    Shaders_vertex = 15,
    Shaders_particle_fragment = 16,
    Shaders_text_fragment = 17,
    Fonts_Antonio_Bold = 18,
};

const char* path(AssetId id);

} // namespace resources

#endif // RESOURCE_IDS_HPP
