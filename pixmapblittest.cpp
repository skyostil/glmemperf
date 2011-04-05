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
 * X11 pixmap surface test using EGLImage
 */
#include "pixmapblittest.h"
#include "util.h"
#include "native.h"

#include <sstream>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

template <typename TYPE>
void fillImage(XImage& img)
{
    TYPE* d = reinterpret_cast<TYPE*>(img.data);

    for (int y = 0; y < img.height; y++)
    {
        for (int x = 0; x < img.width; x++)
        {
            d[x] = x ^ y;
        }
        d += img.bytes_per_line / sizeof(TYPE);
    }
}

PixmapBlitTest::PixmapBlitTest(int width, int height, EGLConfig config,
                               bool rotate, float texW, float texH):
    BlitTest(width, height, rotate, texW, texH),
    m_pixmap(0),
    m_config(config),
    m_depth(0)
{
    eglGetConfigAttrib(ctx.dpy, m_config, EGL_BUFFER_SIZE, &m_depth);
}

void PixmapBlitTest::prepare()
{
    if (!isEGLExtensionSupported("EGL_KHR_image_pixmap"))
    {
        fail("EGL_KHR_image_pixmap not supported");
    }

    if (!isGLExtensionSupported("GL_OES_EGL_image"))
    {
        fail("GL_OES_EGL_image not supported");
    }

    if (!m_config)
    {
        fail("Config not found");
    }

    eglCreateImageKHR =
        (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    eglDestroyImageKHR =
        (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
    glEGLImageTargetTexture2DOES =
        (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");

    ASSERT(eglCreateImageKHR);
    ASSERT(eglDestroyImageKHR);
    ASSERT(glEGLImageTargetTexture2DOES);

    BlitTest::prepare();

    EGLBoolean success;
    initializeBlitter();

    success = nativeCreatePixmap(ctx.nativeDisplay, ctx.dpy, m_config, m_width,
                                 m_height, &m_pixmap);
    ASSERT(success);

    success = fillPixmap();
    ASSERT(success);

    const EGLint imageAttributes[] =
    {
        EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
        EGL_NONE
    };

    EGLImageKHR image;
    image = eglCreateImageKHR(ctx.dpy, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR,
                              (EGLClientBuffer)m_pixmap, imageAttributes);
    ASSERT(image);
    ASSERT_EGL();

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
    ASSERT_GL();
    eglDestroyImageKHR(ctx.dpy, image);
    ASSERT_EGL();
}

bool PixmapBlitTest::fillPixmap()
{
    XImage* img = XGetImage(ctx.nativeDisplay, m_pixmap, 0, 0, m_width, m_height, ~0, ZPixmap);
    if (!img)
    {
        return false;
    }

    ASSERT(img->data);

    switch (img->depth)
    {
    case 16:
        fillImage<uint16_t>(*img);
        break;
    case 32:
        fillImage<uint32_t>(*img);
        break;
    default:
        ASSERT(!"Unknown pixmap depth");
        return false;
    }

    XGCValues gcValues;
    GC gc = XCreateGC(ctx.nativeDisplay, m_pixmap, 0, &gcValues);
    XPutImage(ctx.nativeDisplay, m_pixmap, gc, img, 0, 0, 0, 0, m_width, m_height);
    XFreeGC(ctx.nativeDisplay, gc);
    XDestroyImage(img);

    return true;
}

void PixmapBlitTest::teardown()
{
    nativeDestroyPixmap(ctx.nativeDisplay, m_pixmap);
    BlitTest::teardown();
}

std::string PixmapBlitTest::name() const
{
    std::stringstream s;

    if (m_rotate)
    {
        s << "blit_pixmap_rot90_";
    }
    else
    {
        s << "blit_pixmap_";
    }

    switch (m_depth)
    {
    case 16:
        s << "16bpp";
        break;
    case 32:
        s << "32bpp";
        break;
    }

    s << "_" << m_width << "x" << m_height;

    return s.str();
}
