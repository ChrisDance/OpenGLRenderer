#pragma once
#include "mygl.h"
#include <glm/glm.hpp>
#include <string>

#define ShaderID unsigned int

namespace Shader
{

    unsigned int Create(const char *vPath, const char *fPath);
    void Use(unsigned int ID);
    void SetBool(const std::string &name, unsigned int ID, bool value);
    void SetInt(const std::string &name, unsigned int ID, int value);
    void SetFloat(const std::string &name, unsigned int ID, float value);
    void SetVec3(const std::string &name, unsigned int ID, const glm::vec3 &value);
    void SetMat4(const std::string &name, unsigned int ID, const glm::mat4 &mat);
    GLint GetUniformLocation(unsigned int shader, const char *pUniformName);
};
