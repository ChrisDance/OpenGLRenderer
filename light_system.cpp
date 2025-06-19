#include "light_system.hpp"
#include "model.hpp"



uint _ShaderId;

// TODO: will load from script
static void setup_lights() {
    uint shaderID = _ShaderId;
    // Directional light
    Shader::SetVec3("directLight.Direction", shaderID, glm::vec3(-1.0f, 0.0f, -0.0f));
    Shader::SetVec3("directLight.Intensity", shaderID, glm::vec3(0.5f, 0.5f, 0.5f));
    Shader::SetVec3("directLight.Color", shaderID, glm::vec3(1.0f, 1.0f, 1.0f));

    // Number of point lights
    Shader::SetInt("numPointLights", shaderID, 2);

    // Point light 0
    Shader::SetVec3("pointLights[0].position", shaderID, glm::vec3(0.0f, 1.0f, 0.0f));
    Shader::SetVec3("pointLights[0].color", shaderID, glm::vec3(1.0f, 0.5f, 0.3f));
    Shader::SetFloat("pointLights[0].constant", shaderID, 1.0f);
    Shader::SetFloat("pointLights[0].linear", shaderID, 0.09f);
    Shader::SetFloat("pointLights[0].quadratic", shaderID, 0.032f);

    // Point light 1
    Shader::SetVec3("pointLights[1].position", shaderID, glm::vec3(-2.0f, 2.0f, -1.0f));
    Shader::SetVec3("pointLights[1].color", shaderID, glm::vec3(0.3f, 0.5f, 1.0f));
    Shader::SetFloat("pointLights[1].constant", shaderID, 1.0f);
    Shader::SetFloat("pointLights[1].linear", shaderID, 0.09f);
    Shader::SetFloat("pointLights[1].quadratic", shaderID, 0.032f);
}

void light_system_init(uint shaderID) {
    _ShaderId = entt::locator<Shaders>::value().MAIN;
    setup_lights();
}

void light_system_update(float dt) {
   bool dirty = false;
   if(dirty) {
       // Maybe
   }


}
