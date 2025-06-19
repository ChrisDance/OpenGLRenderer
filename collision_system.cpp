#include "collision_system.hpp"
#include "camera.hpp"

glm::vec3 lastOkayPosition;
void collision_system_init() {
  Camera &cam = entt::locator<Camera>::value();
  lastOkayPosition = cam.Position;
}

void collision_system_update(std::vector<AABB*> &boxes) {
  Camera &cam = entt::locator<Camera>::value();
  // glm::mat4 expander = glm::mat4(1);
  // expander = glm::t
  for (const auto &box : boxes) {
    if (box->contains(cam.Position)) {
      cam.Position = lastOkayPosition;
      return;
    }
  }

  lastOkayPosition = cam.Position;
}
