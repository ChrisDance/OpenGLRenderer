#include "game.hpp"
#include "text_renderer.hpp"
#include "camera.hpp"
#include "collision_system.hpp"
#include "ground_system.hpp"
#include "light_system.hpp"
#include "model.hpp"
#include "model_loader.hpp"
#include "model_setup.hpp"
#include "render_system.hpp"
#include "resource_ids.hpp"
#include "shader.hpp"
#include "static_system.hpp"
#include <cmath>
#include <entt/entt.hpp>
#include <iostream>
#include "skydome.hpp"
#include "animation_test.hpp"


entt::entity e;
AnimatedModel aModel;
void game_init(GLFWwindow *window) {
    // Model model = load_model("resources/models/buster_drone/scene.gltf");
    // // model.aabb =
    // setupModel(&model, 1);
    // e = ecs.create() ;
    // ecs.emplace<Model>(e, model);
    // uploadData(&model, glm::mat4(1.0f));//model.aabb.getFromUnitCubeTransform());
    Shaders shaders;
  shaders.MAIN = Shader::Create(resources::path(resources::Shaders_vertex),
                                resources::path(resources::Shaders_fragment));

  // shaders.TEXT= Shader::Create(resources::path(resources::Shaders_text_vertex),
  //                                 resources::path(resources::Shaders_text_fragment));



  entt::locator<Shaders>::emplace(shaders);
  auto win = entt::locator<Meta>::value().WindowDimensions;



  light_system_init();



}

void game_update(float dt) {

  // ground_system_update(dt);
  aModel.update(dt);


  // collision_system_update(dt);
  render_system_update();
  auto camera = entt::locator<Camera>::value();
  Meta& meta = entt::locator<Meta>::value();

  glm::mat4 projection = glm::perspective(
      glm::radians(camera.Zoom),
      meta.WindowDimensions.x / meta.WindowDimensions.y,
      0.1f, 100.0f
  );
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 vp = projection * view;

  aModel.render(view, projection);
  // static_system_update(dt);




}
