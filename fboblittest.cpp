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
#include "fboblittest.h"
#include "util.h"
#include <sstream>

FBOBlitTest::FBOBlitTest(GLenum format, GLenum type, int width, int height,
                         bool rotate, float texW, float texH, bool useDepth):
    BlitTest(width, height, rotate, texW, texH),
    m_type(type),
    m_useDepth(useDepth)
{
    m_format = format;
    m_type = type;
}

void FBOBlitTest::prepare()
{
    BlitTest::prepare();

    glTexImage2D(GL_TEXTURE_2D, 0, m_format, m_width, m_height, 0, m_format, m_type, NULL);
    ASSERT_GL();

    glGenRenderbuffers(1, &m_depthbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height);
    ASSERT_GL();

    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
    if (m_useDepth)
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, m_depthbuffer);
    }
    ASSERT_GL();

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(status == GL_FRAMEBUFFER_COMPLETE);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, m_width, m_height);
    fillTexture();
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    ASSERT_GL();
}

void FBOBlitTest::fillTexture()
{
    int i;
    glEnable(GL_SCISSOR_TEST);
    for (i = 0; i < m_width; i++)
    {
        glScissor(i, 0, 1, m_height);
        glClearColor(i / (float)m_width, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    glDisable(GL_SCISSOR_TEST);
}

void FBOBlitTest::teardown()
{
    glDeleteFramebuffers(1, &m_framebuffer);
    glDeleteRenderbuffers(1, &m_depthbuffer);
    BlitTest::teardown();
}

std::string FBOBlitTest::name() const
{
    std::stringstream s;

    if (m_rotate)
    {
        s << "blit_fbo_rot90_";
    }
    else
    {
        s << "blit_fbo_";
    }

    s << textureFormatName(m_format, m_type);
    s << "_" << m_width << "x" << m_height;

    return s.str();
}
