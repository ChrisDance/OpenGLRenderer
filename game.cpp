#include "game.hpp"
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
#include <entt/entt.hpp>
#include <iostream>


Model model;
void load_models() {

  std::vector<glm::mat4> instances;
  model = load_model(resources::path(resources::Models_ford));

  auto t = model.aabb.getScaleToLengthTransform(5);
  model.aabb = model.aabb.transform(t);

  instances.push_back(t);

  auto ent1 = ecs.create();
  ecs.emplace<Model>(ent1, model);
  uploadInstanceData(&model, instances);
}






void game_init(GLFWwindow *window) {

  Shaders shaders;
  shaders.MAIN = Shader::Create(resources::path(resources::Shaders_vertex),
                           resources::path(resources::Shaders_fragment));

  entt::locator<Shaders>::emplace(shaders);



  load_models();


  // ground_system_init();
  collision_system_init();
  render_system_init();
}

void game_update(float dt) {
  // ground_system_update(dt);
  // collision_system_update(boxes);
  render_system_update();

}
