/*THIS FILE IS AUTO GENERATED*/
#ifndef RESOURCE_IDS_HPP
#define RESOURCE_IDS_HPP

#include <cstdint>

namespace resources {

enum AssetId : unsigned int{
    None = 0,
    Models_ford = 1,
    Models_pigeon = 2,
    Models_mustang = 3,
    Shaders_fragment = 4,
    Shaders_particle_vertex = 5,
    Shaders_vertex = 6,
    Shaders_particle_fragment = 7,
};

const char* path(AssetId id);

} // namespace resources

#endif // RESOURCE_IDS_HPP
