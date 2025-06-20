#pragma once
#include "mygl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "lib/stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "shader.hpp"
#include "model.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

void drawModel(unsigned int shader, Model *model, unsigned int instanceCount);
void uploadInstanceData(Model *model, std::vector<glm::mat4> instances);
void setupModel(Model *model, int maxInstances);
void uploadData(Model*model, glm::mat4 transform);
