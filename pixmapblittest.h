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
 * X11 pixmap surface test using shared memory and eglBindTexImage
 */
#ifndef PIXMAPBLITTEST_H
#define PIXMAPBLITTEST_H

#include "blittest.h"
#include "util.h"
#include <EGL/egl.h>

class PixmapBlitTest: public BlitTest
{
    Pixmap m_pixmap;
    EGLSurface m_surface;
    EGLContext m_context;
    EGLConfig m_config;
    int m_depth;
public:
    PixmapBlitTest(int width, int height, EGLConfig config,
                   bool rotate = false, float texW = 1.0f, float texH = 1.0f);

    void prepare();
    std::string name() const;
    void teardown();
protected:
    bool fillPixmap();
};

#endif // PIXMAPBLITTEST_H
