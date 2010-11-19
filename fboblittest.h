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
 * Framebuffer object blit test
 */
#ifndef FBOBLITTEST_H
#define FBOBLITTEST_H

#include "blittest.h"
#include <GLES2/gl2.h>

class FBOBlitTest: public BlitTest
{
    GLuint m_framebuffer;
    GLuint m_depthbuffer;
    GLenum m_type;
    bool m_useDepth;
public:
    FBOBlitTest(GLenum format, GLenum type, int width, int height,
                bool rotate = false, float texW = 1.0f, float texH = 1.0f, bool useDepth = false);

    void prepare();
    std::string name() const;
    void teardown();
protected:
    void fillTexture();
};

#endif // FBOBLITTEST_H
