/*THIS FILE IS AUTO GENERATED*/
#ifndef RESOURCE_IDS_HPP
#define RESOURCE_IDS_HPP

#include <cstdint>

namespace resources {

enum AssetId : unsigned int{
    None = 0,
    Models_ford = 1,
    Models_pigeon = 2,
    Models_gun = 3,
    Models_mustang = 4,
    Models_medieval_town = 5,
    Models_street_town = 6,
    Shaders_fragment = 7,
    Shaders_particle_vertex = 8,
    Shaders_text_vertex = 9,
    Shaders_vertex = 10,
    Shaders_particle_fragment = 11,
    Shaders_text_fragment = 12,
    Fonts_Antonio_Bold = 13,
};

const char* path(AssetId id);

} // namespace resources

#endif // RESOURCE_IDS_HPP
