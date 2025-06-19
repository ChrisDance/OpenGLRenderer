#include "render_system.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "model_setup.hpp"

void render_system_init() {

  for (auto [entity, model] : ecs.view<Model>().each()) {
    setupModel(&model, 1);
  }
}
void render_system_update() {
  Camera &camera = entt::locator<Camera>::value();
  Meta &meta = entt::locator<Meta>::value();
  Shaders &shaders = entt::locator<Shaders>::value();

  glm::mat4 projection = glm::perspective(
      glm::radians(camera.Zoom),
      meta.WindowDimensions.x / meta.WindowDimensions.y, 0.1f, 100.0f);
  glm::mat4 view = camera.GetViewMatrix();

  Shader::Use(shaders.MAIN);
  Shader::SetMat4("projection", shaders.MAIN, projection);
  Shader::SetMat4("view", shaders.MAIN, view);
  Shader::SetVec3("viewPos", shaders.MAIN, camera.Position);

  for (auto [entity, model] : ecs.view<Model>().each()){
    drawModel(shaders.MAIN, &model, 1);
  }
}
