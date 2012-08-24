/**
 * OpenGL ES 2.0 memory performance estimator
 * Copyright (C) 2010 Nokia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * \author Sami Kyöstilä <sami.kyostila@nokia.com>
 *
 * Shader blit test
 */
#ifndef SHADERBLITTEST_H
#define SHADERBLITTEST_H

#include "test.h"
#include "util.h"
#include <GLES2/gl2.h>

class ShaderBlitTest: public Test
{
protected:
    GLint m_program, m_secondaryProgram;
    GLint m_positionAttr, m_texcoordAttr;
    GLuint m_texture;
    int m_width, m_height;
    float m_texW, m_texH;
    float m_quadW, m_quadH;
    std::string m_effect;
    GLuint m_framebuffers[2];
    GLuint m_fboTextures[2];
    int m_downSample;
    GLint m_savedViewport[4];

public:
    ShaderBlitTest(const std::string& effect, int width, int height, 
                   float texW = 1.0f, float texH = 1.0f,
                   float quadW = 1.0f, float quadH = 1.0f);

    void operator()(int frame);

    void prepare();
    std::string name() const;
    void teardown();
};

#endif // SHADERBLITTEST_H
