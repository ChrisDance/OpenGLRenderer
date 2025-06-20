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
std::vector<glm::mat4> instances;
Model model;

// model = load_model(resources::path(resources::Models_gun));
// auto t = model.aabb.getScaleToLengthTransform(1);
// // model.aabb = model.aabb.transform(t);

// instances.push_back(t);

// auto ent1 = ecs.create();
// ecs.emplace<Model>(ent1, model);
// setupModel(&model, 1);
// // render_system_init();

// uploadInstanceData(&model, instances);
//



void game_init(GLFWwindow *window) {


    Shaders shaders;
  shaders.MAIN = Shader::Create(resources::path(resources::Shaders_vertex),
                                resources::path(resources::Shaders_fragment));

  shaders.TEXT= Shader::Create(resources::path(resources::Shaders_text_vertex),
                                  resources::path(resources::Shaders_text_fragment));


  entt::locator<Shaders>::emplace(shaders);
  auto win = entt::locator<Meta>::value().WindowDimensions;

  text_init(shaders.TEXT, win.x, win.y);
  static_system_init();
  light_system_init();

  // ground_system_init();
  collision_system_init();
}

void game_update(float dt) {

  // ground_system_update(dt);

  collision_system_update(dt);
  render_system_update();
  static_system_update(dt);



}

// Camera&camera = entt::locator<Camera>::value();
// instances[0] = camera.GetGunTransform(0.3, -0.3, 0.6);

// instances[0] = glm::rotate(instances[0], (float)M_PI * 1.5f, glm::vec3(0, 1, 0));
// instances[0] = glm::rotate(instances[0], (float)M_PI / 2, glm::vec3(-1, 0, 0));


// uploadInstanceData(&model, instances);
