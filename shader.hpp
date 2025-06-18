#pragma once
#include "mygl.h"
#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>

namespace Shader
{

    unsigned int create(const char *vPath, const char *fPath);

    void use(unsigned int ID);

    void setBool(const std::string &name, unsigned int ID, bool value);
    void setInt(const std::string &name, unsigned int ID, int value);
    void setFloat(const std::string &name, unsigned int ID, float value);
    void setVec3(const std::string &name, unsigned int ID, const glm::vec3 &value);
    void setMat4(const std::string &name, unsigned int ID, const glm::mat4 &mat);
    void setUniformBuffer(const char* blockName, unsigned int ID, const void* data, ssize_t size,  int bindingPoint = 0);
    GLint GetUniformLocation(unsigned int shader, const char *pUniformName);

};
