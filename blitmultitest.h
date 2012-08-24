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
#ifndef PIXMAPBLITMULTITEST_H
#define PIXMAPBLITMULTITEST_H

#include "pixmapblittest.h"
#include "blittest.h"
#include <vector>
#include <EGL/egl.h>

template <class parent>
class BlitMultiTest: public parent
{
protected:
    std::vector<NativePixmapType> m_pixmaps;
    std::vector<GLuint> m_textures;
    int m_nr_textures;
    bool m_blend;
private:
    void pixmap_prepare();
    void pixmap_teardown();
public:

    BlitMultiTest(int width, int height, EGLConfig config,
                  bool rotate = false, int nr_textures = 2,
                  float texW = 1.0f, float texH = 1.0f);

    BlitMultiTest(GLenum format, GLenum type, int width, int height, const std::string& fileName,
                  bool rotate = false, int nr_textures = 2);
    void prepare();
    void operator()(int frame);
    void teardown();
    std::string name() const;
};

#endif
