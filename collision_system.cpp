#include "collision_system.hpp"
#include "camera.hpp"
#include "model.hpp"

glm::vec3 lastOkayPosition;
void collision_system_init() {
  Camera &cam = entt::locator<Camera>::value();
  lastOkayPosition = cam.Position;
}

void collision_system_update(float dt) {

  Camera &cam = entt::locator<Camera>::value();
  auto colliders = entt::locator<tCollidables>::value();
  auto base = AABB(glm::vec3(-1, -1, -1), glm::vec3(1,1,1));

  for (auto &box : colliders.aabbs) {
    auto test = base.transform(box);
    if (test.contains(cam.Position)) {
      cam.Position = lastOkayPosition;
      return;
    }
  }

  lastOkayPosition = cam.Position;
}
