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
 * Generic blitter
 */
#include "blittest.h"
#include "util.h"

#include <sstream>
#include <stdio.h>

const char *defaultVertSource = 
    "precision mediump float;\n"
    "attribute vec2 in_position;\n"
    "attribute vec2 in_texcoord;\n"
    "varying vec2 texcoord;\n"
    "\n"
    "void main()\n"
    "{\n"
    "	gl_Position = vec4(in_position, 0.0, 1.0);\n"
    "	texcoord = in_texcoord;\n"
    "}\n";

const char *defaultFragSource = 
    "precision mediump float;\n"
    "varying vec2 texcoord;\n"
    "uniform sampler2D texture;\n"
    "\n"
    "void main()\n"
    "{\n"
    "	gl_FragColor = texture2D(texture, texcoord);\n"
    "}\n";

BlitTest::BlitTest(int width, int height, 
                   bool rotate, float texW, float texH,
                   float quadW, float quadH):
    m_width(width),
    m_height(height),
    m_texW(texW),
    m_texH(texH),
    m_quadW(quadW),
    m_quadH(quadH),
    m_rotate(rotate),
    m_vertSource(defaultVertSource),
    m_fragSource(defaultFragSource)
{
}

BlitTest::BlitTest(GLenum format, GLenum type, int width, int height, const std::string& fileName,
                   bool rotate, float texW, float texH,
                   float quadW, float quadH):
    m_format(format),
    m_type(type),
    m_width(width),
    m_height(height),
    m_texW(texW),
    m_texH(texH),
    m_quadW(quadW),
    m_quadH(quadH),
    m_rotate(rotate),
    m_fileName(fileName),
    m_vertSource(defaultVertSource),
    m_fragSource(defaultFragSource)
{
    if (m_format >= 0x8c00)
    {
        m_type = m_format;
    }
}

void BlitTest::teardown()
{
    glUseProgram(0);
    glDisableVertexAttribArray(m_positionAttr);
    glDisableVertexAttribArray(m_texcoordAttr);
    glDeleteProgram(m_program);
    // Disabled until driver segfault is fixed
    //glDeleteTextures(1, &m_texture);
}

void BlitTest::prepare()
{
    initializeBlitter();
    if (m_fileName.size())
    {
        if (m_format >= 0x8c00)
        {
            loadCompressedTexture(GL_TEXTURE_2D, 0, m_format, m_width, m_height, m_fileName);
        }
        else
        {
            loadRawTexture(GL_TEXTURE_2D, 0, m_format, m_width, m_height, m_format, m_type, m_fileName);
        }
    }
    ASSERT_GL();
}

void BlitTest::operator()(int frame)
{
    float t = frame / 400.0f;

    // Disable animation for now, since it interferes with the results due
    // to lack of wrapping for NPOT textures
    t = 0;

    const GLfloat texcoords[] =
    {
         0 + t,       m_texH + t,
         0 + t,       0 + t,
         m_texW + t,  m_texH + t,
         m_texW + t,  0 + t
    };

    const GLfloat texcoordsRotated[] =
    {
         m_texW + t,  0 + t,
         0 + t,       0 + t,
         m_texW + t,  m_texH + t,
         0 + t,       m_texH + t
    };

    const GLfloat vertices[] =
    {
        -m_quadW, -m_quadH,
        -m_quadW,  m_quadH,
         m_quadW, -m_quadH,
         m_quadW,  m_quadH
    };

    glVertexAttribPointer(m_positionAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(m_texcoordAttr, 2, GL_FLOAT, GL_FALSE, 0, m_rotate ? texcoordsRotated : texcoords);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

std::string BlitTest::name() const
{
    std::stringstream s;

    if (m_rotate)
    {
        s << "blit_tex_rot90_";
    }
    else
    {
        s << "blit_tex_";
    }

    s << textureFormatName(m_format, m_type);
    s << "_" << m_width << "x" << m_height;

    return s.str();
}

void BlitTest::initializeBlitter()
{
    m_program = createProgram(m_vertSource, m_fragSource);
    glUseProgram(m_program);

    glClearColor(.2, .4, .6, 1.0);

    m_positionAttr = glGetAttribLocation(m_program, "in_position");
    m_texcoordAttr = glGetAttribLocation(m_program, "in_texcoord");
    assert(m_positionAttr >= 0);
    assert(m_texcoordAttr >= 0);

    glEnableVertexAttribArray(m_positionAttr);
    glEnableVertexAttribArray(m_texcoordAttr);

    m_textureUnif = glGetUniformLocation(m_program, "texture");
    assert(m_textureUnif >= 0);
    glUniform1i(m_textureUnif, 0);

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL();

    GLint read;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &read);
    assert(read == GL_NEAREST);
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &read);
    assert(read == GL_NEAREST);
}
