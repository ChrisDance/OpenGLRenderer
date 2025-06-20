#pragma once
#include <map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "mygl.h"

#include <ft2build.h>
#include FT_FREETYPE_H

/// Holds all state information relevant to a character as loaded using FreeType
struct Character
{
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2 Size;        // Size of glyph
    glm::ivec2 Bearing;     // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

extern std::map<GLchar, Character> Characters;
extern unsigned int _VAO, _VBO;

void RenderText(unsigned int shader, std::string text, float x, float y, float scale, glm::vec3 color);
void text_init(unsigned int shader, int width, int height);
