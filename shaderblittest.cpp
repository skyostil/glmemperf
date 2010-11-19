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
    m_secondaryProgram(0),
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
    if (m_secondaryProgram)
    {
        glDeleteProgram(m_secondaryProgram);
    }
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

    if (m_effect == "blur")
    {
        const GLfloat texcoordsFlipped[] =
        {
             0,       0,
             0,       m_texH,
             m_texW,  0,
             m_texW,  m_texH
        };

        int passes = 2;
        for (int pass = 0; pass < passes; pass++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffers[0]);
            glViewport(0, 0, m_width / m_downSample, m_height / m_downSample);
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            if (pass == passes - 1)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, m_width, m_height);
                glVertexAttribPointer(m_texcoordAttr, 2, GL_FLOAT, GL_FALSE, 0, texcoordsFlipped);
            }
            else
            {
                glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffers[1]);
                glViewport(0, 0, m_width / m_downSample, m_height / m_downSample);
            }

            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(m_secondaryProgram);
            glBindTexture(GL_TEXTURE_2D, m_fboTextures[0]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            glUseProgram(m_program);
            if (pass == passes - 1)
            {
                glBindTexture(GL_TEXTURE_2D, m_texture);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, m_fboTextures[1]);
            }
            glVertexAttribPointer(m_texcoordAttr, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
        }
    }
    else
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
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
    const char* vertSourceDefault = 
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

    const char* vertSourceBlur = 
        "precision mediump float;\n"
        "attribute vec2 in_position;\n"
        "attribute vec2 in_texcoord;\n"
        "uniform vec2 texoffsets[5];\n"
        "varying lowp vec2 texcoord0;\n"
        "varying lowp vec2 texcoord1;\n"
        "varying lowp vec2 texcoord2;\n"
        "varying lowp vec2 texcoord3;\n"
        "varying lowp vec2 texcoord4;\n"
        "\n"
        "void main()\n"
        "{\n"
        "	gl_Position = vec4(in_position, 0.0, 1.0);\n"
        "	texcoord0 = in_texcoord + texoffsets[0];\n"
        "	texcoord1 = in_texcoord + texoffsets[1];\n"
        "	texcoord2 = in_texcoord + texoffsets[2];\n"
        "	texcoord3 = in_texcoord + texoffsets[3];\n"
        "	texcoord4 = in_texcoord + texoffsets[4];\n"
        "}\n";

    const char* fragSourceBlur = 
        "precision mediump float;\n"
        "varying lowp vec2 texcoord0;\n"
        "varying lowp vec2 texcoord1;\n"
        "varying lowp vec2 texcoord2;\n"
        "varying lowp vec2 texcoord3;\n"
        "varying lowp vec2 texcoord4;\n"
        "uniform sampler2D texture;\n"
        "\n"
        "void main()\n"
        "{\n"
        "       lowp vec3 color = vec3(0.0, 0.0, 0.0);\n"
        "       color += texture2D(texture, texcoord0).rgb * 0.078184;\n"
        "       color += texture2D(texture, texcoord1).rgb * 0.225492;\n"
        "       color += texture2D(texture, texcoord2).rgb * 0.392649;\n"
        "       color += texture2D(texture, texcoord3).rgb * 0.225492;\n"
        "       color += texture2D(texture, texcoord4).rgb * 0.078184;\n"
        "       gl_FragColor = vec4(color, 1.0);\n"
        "}\n";

    const char* fragSource = 0;
    const char* vertSource = vertSourceDefault;
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
    else if (m_effect == "blur")
    {
        vertSource = vertSourceBlur;
        fragSource = fragSourceBlur;
    }
    assert(fragSource);
    assert(vertSource);

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

    if (m_effect == "blur")
    {
        m_downSample = 4;

        const float s = m_downSample;
        const float texoffsetsHoriz[] = 
        {
            -0.001953*s, 0.000000,
            -0.000977*s, 0.000000,
            0.000000*s, 0.000000,
            0.000977*s, 0.000000,
            0.001953*s, 0.000000,
        };

        const float texoffsetsVert[] = 
        {
            0.000000, -0.003906*s,
            0.000000, -0.001953*s,
            0.000000, 0.000000*s,
            0.000000, 0.001953*s,
            0.000000, 0.003906*s,
        };

        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        loadCompressedTexture(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, 1024, 512, "data/blur_1024x512_etc1.raw");
        //loadRawTexture(GL_TEXTURE_2D, 0, GL_RGB, 1024, 512, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, "data/digital_nature2_1024x512_rgb565.raw");
        glUniform1i(glGetUniformLocation(m_program, "texture"), 0);
        glUniform2fv(glGetUniformLocation(m_program, "texoffsets"), 5, texoffsetsHoriz);
        ASSERT_GL();

        m_texW = 800 / 1024.0f;
        m_texH = 480 / 512.0f;

        m_secondaryProgram = createProgram(vertSource, fragSourceBlur);
        glUseProgram(m_secondaryProgram);
        glBindAttribLocation(m_secondaryProgram, m_positionAttr, "in_position");
        glBindAttribLocation(m_secondaryProgram, m_texcoordAttr, "in_texcoord");
        glUniform2fv(glGetUniformLocation(m_program, "texoffsets"), 5, texoffsetsVert);
        glUseProgram(m_program);
        ASSERT_GL();

        glGenTextures(2, m_fboTextures);
        glBindTexture(GL_TEXTURE_2D, m_fboTextures[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024 / m_downSample, 512 / m_downSample, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
        glBindTexture(GL_TEXTURE_2D, m_fboTextures[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024 / m_downSample, 512 / m_downSample, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
        ASSERT_GL();

        glGenFramebuffers(2, m_framebuffers);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffers[0]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextures[0], 0);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffers[1]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextures[1], 0);
        ASSERT_GL();

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        assert(status == GL_FRAMEBUFFER_COMPLETE);
        ASSERT_GL();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ASSERT_GL();
}
