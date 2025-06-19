#pragma once
#include "model.hpp"
#include <vector>




void collision_system_init();
void collision_system_update(std::vector<AABB*> &boxes);


//
