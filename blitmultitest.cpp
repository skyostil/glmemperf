/**
 * OpenGL ES 2.0 memory performance estimator
 * Copyright (C) 2011 Nokia
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
 * \author Pauli Nieminen <ext-pauli.nieminen@nokia.com>
 *
 * Composite render performance tests
 */

#include "blitmultitest.h"
#include <typeinfo>

#include <sstream>
template <>
BlitMultiTest<PixmapBlitTest>::BlitMultiTest(int width, int height,
                                         EGLConfig config,
                                         bool rotate, int nr_textures,
                                         float texW, float texH) :
    PixmapBlitTest(width, height, config, rotate, texW, texH),
    m_nr_textures(nr_textures)
{
}

template <>
BlitMultiTest<BlitTest>::BlitMultiTest(GLenum format, GLenum type, int width, int height,
                   const std::string& fileName,
                   bool rotate, int nr_textures) :
    BlitTest(format, type, width, height, fileName, rotate),
    m_nr_textures(nr_textures)
{
}

template <class PARENT>
void BlitMultiTest<PARENT>::pixmap_prepare()
{
}

template <>
void BlitMultiTest<PixmapBlitTest>::pixmap_prepare()
{
    m_pixmaps.push_back(PixmapBlitTest::m_pixmap);
}

template <class PARENT>
void BlitMultiTest<PARENT>::pixmap_teardown()
{
}

template <>
void BlitMultiTest<PixmapBlitTest>::pixmap_teardown()
{
    PixmapBlitTest::m_pixmap = m_pixmaps.back();
    m_pixmaps.pop_back();
}

template <class PARENT>
void BlitMultiTest<PARENT>::prepare()
{
    int i;
    for (i = 0; i < m_nr_textures; ++i) {
        PARENT::prepare();
        pixmap_prepare();
        m_textures.push_back(PARENT::m_texture);
        if (m_nr_textures != i + 1) {
            glUseProgram(0);
            glDeleteProgram(PARENT::m_program);
        }
    }
}

template <class PARENT>
void BlitMultiTest<PARENT>::operator()(int frame)
{
    int i = 0;
    glBindTexture(GL_TEXTURE_2D, m_textures[i]);
    PARENT::operator()(frame);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (i = 1; i < m_nr_textures; ++i) {
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        PARENT::render(frame);
    }

    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
}

template <class PARENT>
void BlitMultiTest<PARENT>::teardown()
{
    int i;
    for (i = 0; i < m_nr_textures; ++i) {
        PARENT::m_texture = m_textures.back();
        m_textures.pop_back();
        pixmap_teardown();
        PARENT::teardown();
    }
}

template <class PARENT>
std::string BlitMultiTest<PARENT>::name() const
{
    std::stringstream ss;

    ss << PARENT::name();
    ss << "_blend" << m_nr_textures;

    return ss.str();
}
