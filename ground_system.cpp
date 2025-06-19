#include "ground_system.hpp"
#include "camera.hpp"
#include <cmath>

struct Ground {
  float y;
};

Ground ground;

void ground_system_init() { ground.y = 0.0f; }


void ground_system_update(float dt) {
  Camera &camera = entt::locator<Camera>::value();
  camera.Position.y = 1;
}
