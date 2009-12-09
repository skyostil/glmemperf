/**
 * OpenGL ES 2.0 memory performance estimator
 * Copyright (C) 2009 Nokia
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
#include "shaderblittest.h"
#include <sstream>
#include <GLES2/gl2ext.h>
#include <stdlib.h>

ShaderBlitTest::ShaderBlitTest(const std::string& effect, int width, int height, 
                               float texW, float texH,
                               float quadW, float quadH):
    m_width(width),
    m_height(height),
    m_texW(texW),
    m_texH(texH),
    m_quadW(quadW),
    m_quadH(quadH),
    m_effect(effect)
{
}

void ShaderBlitTest::teardown()
{
    glUseProgram(0);
    glDisableVertexAttribArray(m_positionAttr);
    glDisableVertexAttribArray(m_texcoordAttr);
    glDeleteProgram(m_program);
    // Disabled until driver segfault is fixed
    //glDeleteTextures(1, &m_texture);
}

void ShaderBlitTest::operator()(int frame)
{
    float t = frame * 0;

    const GLfloat texcoords[] =
    {
         0 + t,       m_texH + t,
         0 + t,       0 + t,
         m_texW + t,  m_texH + t,
         m_texW + t,  0 + t
    };

    const GLfloat vertices[] =
    {
        -m_quadW, -m_quadH,
        -m_quadW,  m_quadH,
         m_quadW, -m_quadH,
         m_quadW,  m_quadH
    };

    glVertexAttribPointer(m_positionAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(m_texcoordAttr, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

std::string ShaderBlitTest::name() const
{
    std::stringstream s;

    s << "blit_shader_" << m_effect;
    s << "_" << m_width << "x" << m_height;

    return s.str();
}

void ShaderBlitTest::prepare()
{
    const char* vertSource = 
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

    const char* fragSourceConst = 
        "precision lowp float;\n"
        "\n"
        "void main()\n"
        "{\n"
        "	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    const char* fragSourceLinGrad = 
        "precision lowp float;\n"
        "varying vec2 texcoord;\n"
        "\n"
        "void main()\n"
        "{\n"
        "	gl_FragColor = vec4(texcoord.x, 0.0, 0.0, 1.0);\n"
        "}\n";

    const char* fragSourceRadGrad = 
        "precision mediump float;\n"
        "varying vec2 texcoord;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   float t = 1.0 - distance(texcoord, vec2(0.5, 0.5)) * 2.0;\n"
        "	gl_FragColor = vec4(0.0, t, 0.0, 1.0);\n"
        "}\n";

    const char* fragSourcePalette = 
        "precision mediump float;\n"
        "varying vec2 texcoord;\n"
        "uniform sampler2D texture;\n"
        "uniform sampler2D paletteTexture;\n"
        "\n"
        "void main()\n"
        "{\n"
        "	float index = texture2D(texture, texcoord).a;\n"
        "	gl_FragColor = texture2D(paletteTexture, vec2(index, 0.0));\n"
        "}\n";

    const char* fragSourceMask = 
        "precision mediump float;\n"
        "varying vec2 texcoord;\n"
        "uniform sampler2D texture;\n"
        "\n"
        "void main()\n"
        "{\n"
        "	float mask   = texture2D(texture, texcoord).g;\n"
        "	vec4 color   = texture2D(texture, texcoord + vec2(0, 64.0 / 128.0));\n"
        "	gl_FragColor = vec4(color.rgb, mask);\n"
        "}\n";

    const char* fragSource = 0;
    if (m_effect == "const")
    {
        fragSource = fragSourceConst;
    }
    else if (m_effect == "lingrad")
    {
        fragSource = fragSourceLinGrad;
    }
    else if (m_effect == "radgrad")
    {
        fragSource = fragSourceRadGrad;
    }
    else if (m_effect == "palette")
    {
        fragSource = fragSourcePalette;
    }
    else if (m_effect == "mask")
    {
        fragSource = fragSourceMask;
    }
    assert(fragSource);

    m_program = createProgram(vertSource, fragSource);
    glUseProgram(m_program);

    glClearColor(.2, .4, .6, 1.0);

    m_positionAttr = glGetAttribLocation(m_program, "in_position");
    m_texcoordAttr = glGetAttribLocation(m_program, "in_texcoord");

    glGenTextures(1, &m_texture);
    glEnableVertexAttribArray(m_positionAttr);
    glEnableVertexAttribArray(m_texcoordAttr);

    if (m_effect == "palette")
    {
        char* texture = new char[800 * 480];
        char* palette = new char[256 * 4];
        for (int y = 0; y < 480; y++)
        {
            for (int x = 0; x < 800; x++)
            {
                texture[y * 800 + x] = x ^ y;
            }
        }
        for (int i = 0; i < 256; i++)
        {
            palette[i * 4 + 0] = i;
            palette[i * 4 + 1] = abs(i - 0x7f);
            palette[i * 4 + 2] = 0xff - i;
            palette[i * 4 + 3] = 0xff;
        }
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 3);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, palette);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        ASSERT_GL();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 800, 480, 0, GL_ALPHA, GL_UNSIGNED_BYTE, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        ASSERT_GL();
        delete[] texture;
        delete[] palette;
        glUniform1i(glGetUniformLocation(m_program, "texture"), 0);
        glUniform1i(glGetUniformLocation(m_program, "paletteTexture"), 1);
        ASSERT_GL();
    }

    if (m_effect == "mask")
    {
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        loadCompressedTexture(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, 128, 256, "data/xorg-colormask_128x256_etc1.raw");
        glUniform1i(glGetUniformLocation(m_program, "texture"), 0);
        ASSERT_GL();
    }

    ASSERT_GL();
}
