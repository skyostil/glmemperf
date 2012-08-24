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
#ifndef BLITTEST_H
#define BLITTEST_H

#include "test.h"
#include <GLES2/gl2.h>

class BlitTest: public Test
{
protected:
    GLint m_program;
    GLint m_positionAttr, m_texcoordAttr;
    GLint m_textureUnif;
    GLuint m_texture;
    GLenum m_format;
    GLenum m_type;
    int m_width, m_height;
    float m_texW, m_texH;
    float m_quadW, m_quadH;
    bool m_rotate;
    bool m_blend;
    std::string m_fileName;
    std::string m_vertSource;
    std::string m_fragSource;

public:
    BlitTest(int width, int height, 
             bool rotate = false, float texW = 1.0f, float texH = 1.0f,
             float quadW = 1.0f, float quadH = 1.0f, bool blend = false);

    BlitTest(GLenum format, GLenum type, int width, int height, const std::string& fileName,
             bool rotate = false, float texW = 1.0f, float texH = 1.0f,
             float quadW = 1.0f, float quadH = 1.0f, bool blend = false);

    void prepare();
    void operator()(int frame);
    void teardown();
    std::string name() const;

protected:
    void initializeBlitter();
    void render(int frame);
};

#endif // BLITTEST_H
