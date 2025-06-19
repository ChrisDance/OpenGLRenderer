#include "shader.hpp"
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

#define INVALID_UNIFORM_LOCATION 0xffffffff


static bool checkCompileErrors(ShaderID shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                      << infoLog << std::endl;
            return false;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                      << infoLog << std::endl;

                      return false;
        }
    }

    return true ;
}

static std::string readFile(const char *filePath)
{
    std::ifstream file;
    std::stringstream stream;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        file.open(filePath);
        stream << file.rdbuf();
        file.close();
        return stream.str();
    }
    catch (std::ifstream::failure &e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        return "";
    }
}

static unsigned int compileShader(const char *source, GLenum type)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    checkCompileErrors(shader, type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
    return shader;
}
void Shader::Use(unsigned int ID) { glUseProgram(ID); }

void Shader::SetBool(const std::string &name, unsigned int ID, bool value)
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::SetInt(const std::string &name, unsigned int ID, int value)
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::SetFloat(const std::string &name, unsigned int ID, float value)
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::SetVec3(const std::string &name, unsigned int ID, const glm::vec3 &value)
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::SetMat4(const std::string &name, unsigned int ID, const glm::mat4 &mat)
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

GLint Shader::GetUniformLocation(unsigned int shader, const char *pUniformName)
{
    GLuint Location = glGetUniformLocation(shader, pUniformName);

    if (Location == INVALID_UNIFORM_LOCATION)
    {
        fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
    }

    return Location;
}

unsigned int Shader::Create(const char *vPath, const char *fPath)
{
    std::string vertexCode = readFile(vPath);
    std::string fragmentCode = readFile(fPath);

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    unsigned int vertex = compileShader(vShaderCode, GL_VERTEX_SHADER);
    unsigned int fragment = compileShader(fShaderCode, GL_FRAGMENT_SHADER);

    unsigned int ID;
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    bool success = checkCompileErrors(ID, "PROGRAM");


    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if(!success) {
        exit(1) ;
    }


    return ID;
}
