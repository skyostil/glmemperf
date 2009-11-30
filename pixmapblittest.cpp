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
 * X11 pixmap surface test using shared memory and eglBindTexImage
 */
#include "pixmapblittest.h"
#include "util.h"
#include "native.h"
#include <sstream>
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
    m_config(config)
{
    eglGetConfigAttrib(ctx.dpy, m_config, EGL_BUFFER_SIZE, &m_depth);
}

void PixmapBlitTest::prepare()
{
    if (!isEGLExtensionSupported("EGL_NOKIA_texture_from_pixmap"))
    {
        fail("EGL_NOKIA_texture_from_pixmap not supported");
    }

    BlitTest::prepare();

    EGLBoolean success;
    initializeBlitter();

    success = nativeCreatePixmap(ctx.nativeDisplay, ctx.dpy, m_config, m_width,
	                         m_height, &m_pixmap);
    assert(success);

    success = fillPixmap();
    assert(success);

    const EGLint surfAttrs[] =
    {
        EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGB,
        EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
        EGL_MIPMAP_TEXTURE, EGL_FALSE,
        EGL_NONE
    };

    m_surface = eglCreatePixmapSurface(ctx.dpy, m_config, m_pixmap, surfAttrs);
    assert(m_surface != EGL_NO_SURFACE);

    success = eglBindTexImage(ctx.dpy, m_surface, EGL_BACK_BUFFER);
    assert(success);

    ASSERT_GL();
}

bool PixmapBlitTest::fillPixmap()
{
    XImage* img = XGetImage(ctx.nativeDisplay, m_pixmap, 0, 0, m_width, m_height, ~0, ZPixmap);
    if (!img)
    {
        return false;
    }

    assert(img->data);

    switch (img->depth)
    {
    case 16:
        fillImage<uint16_t>(*img);
        break;
    case 32:
        fillImage<uint32_t>(*img);
        break;
    default:
        assert(!"Unknown pixmap depth");
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
    eglReleaseTexImage(ctx.dpy, m_surface, EGL_BACK_BUFFER);
    eglDestroySurface(ctx.dpy, m_surface);
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
