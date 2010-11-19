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
 * CPU texture streaming test
 */
#include "cpuinterleavingtest.h"
#include "util.h"
#include "native.h"

#include <sstream>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include <sys/ipc.h>
#include <sys/shm.h>

template <typename TYPE>
void fillTexture(TYPE* pixels, int width, int height, int stride, int frame)
{
    TYPE color = (TYPE)0xffffffffu;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if ((x + y + frame) & 0x10)
            {
                pixels[x] = color;
            }
            else
            {
                pixels[x] = 0;
            }
        }
        pixels += stride / sizeof(TYPE);
    }
}

CPUInterleavingTest::CPUInterleavingTest(CPUInterleavingMethod method,
                                         int buffers, int bitsPerPixel,
                                         int width, int height,
                                         float texW, float texH):
    BlitTest(width, height, false, texW, texH),
    m_method(method),
    m_buffers(buffers),
    m_dataBitsPerPixel(bitsPerPixel),
    m_readBuffer(buffers - 1),
    m_writeBuffer(0)
{
}


void CPUInterleavingTest::prepare()
{
    int i;
    bool success;

    BlitTest::prepare();

    glGenTextures(m_buffers, m_textures);
    for (int i = 0; i < m_buffers; i++)
    {
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    ASSERT_GL();

    switch (m_method)
    {
    case CPUI_TEXTURE_UPLOAD:
        {
            m_dataStride = m_width * m_dataBitsPerPixel / 8;
            for (i = 0; i < m_buffers; i++)
            {
                m_textureData[i] = new char[m_height * m_dataStride];
            }
        }
        break;
    case CPUI_XSHM_IMAGE:
        {
            Status shmSupported = XShmQueryExtension(ctx.nativeDisplay);
            if (!shmSupported)
            {
                fail("X11 shared memory extension not supported");
            }

            m_completionEvent = XShmGetEventBase(ctx.nativeDisplay) + ShmCompletion;

            const EGLint pixmapConfigAttrs[] =
            {
                EGL_BUFFER_SIZE, m_dataBitsPerPixel,
                EGL_NONE
            };
            EGLint configCount = 0;

            eglChooseConfig(ctx.dpy, pixmapConfigAttrs, &m_config, 1, &configCount);
            ASSERT(configCount);

            for (i = 0; i < m_buffers; i++)
            {
                success = nativeCreatePixmap(ctx.nativeDisplay, ctx.dpy,
                                             m_config, m_width, m_height, &m_pixmaps[i]);
                ASSERT(success);

                XGCValues gcValues;
                m_gc[i] = XCreateGC(ctx.nativeDisplay, m_pixmaps[i], 0, &gcValues);

                const EGLint surfAttrs[] =
                {
                    EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGB,
                    EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
                    EGL_MIPMAP_TEXTURE, EGL_FALSE,
                    EGL_NONE
                };

                m_surfaces[i] = eglCreatePixmapSurface(ctx.dpy, m_config, m_pixmaps[i], surfAttrs);
                ASSERT(m_surfaces[i] != EGL_NO_SURFACE);

                glBindTexture(GL_TEXTURE_2D, m_textures[i]);
                success = eglBindTexImage(ctx.dpy, m_surfaces[i], EGL_BACK_BUFFER);
                ASSERT(success);

                XVisualInfo visualInfo;
                XVisualInfo* visual;
                int visualCount = 0;
                visualInfo.depth = m_dataBitsPerPixel;
                visualInfo.screen = DefaultScreen(ctx.nativeDisplay);
                visual = XGetVisualInfo(ctx.nativeDisplay, VisualDepthMask | VisualScreenMask, 
                                        &visualInfo, &visualCount);

                ASSERT(visualCount > 0);

                m_ximage[i] = XShmCreateImage(ctx.nativeDisplay, visual->visual, m_dataBitsPerPixel,
                                              ZPixmap, NULL,
                                              &m_shminfo[i], m_width, m_height);
                m_shminfo[i].shmid = shmget(IPC_PRIVATE, 
                                            m_ximage[i]->bytes_per_line *
                                            m_ximage[i]->height, IPC_CREAT | 0777);
                m_shminfo[i].shmaddr = m_ximage[i]->data = (char*)shmat(m_shminfo[i].shmid, 0, 0);
                ASSERT(m_shminfo[i].shmaddr);
                m_shminfo[i].readOnly = False;
                Status status = XShmAttach(ctx.nativeDisplay, &m_shminfo[i]);
                ASSERT(status);

                m_textureData[i] = m_ximage[i]->data;
                m_dataStride = m_ximage[i]->bytes_per_line;
                m_writeCompleted[i] = true;
                m_drawableIndex[m_pixmaps[i]] = i;
            }
        }
        break;
    case CPUI_EGL_LOCK_SURFACE:
        {
            if (!isEGLExtensionSupported("EGL_KHR_lock_surface2"))
            {
                fail("EGL_KHR_lock_surface2 not supported");
            }

            // Get function pointers
            m_eglLockSurfaceKHR =
                (PFNEGLLOCKSURFACEKHRPROC)eglGetProcAddress("eglLockSurfaceKHR");
            m_eglUnlockSurfaceKHR =
                (PFNEGLUNLOCKSURFACEKHRPROC)eglGetProcAddress("eglUnlockSurfaceKHR");

            ASSERT(m_eglLockSurfaceKHR);
            ASSERT(m_eglUnlockSurfaceKHR);

            if (!isEGLExtensionSupported("EGL_KHR_image_base"))
            {
                fail("EGL_KHR_image_base not supported");
            }

            if (!isEGLExtensionSupported("EGL_KHR_image_pixmap"))
            {
                fail("EGL_KHR_image_pixmap not supported");
            }

            m_eglCreateImageKHR =
                (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
            m_eglDestroyImageKHR =
                (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
            m_glEGLImageTargetTexture2DOES =
                (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");

            ASSERT(m_eglCreateImageKHR);
            ASSERT(m_eglDestroyImageKHR);
            ASSERT(m_glEGLImageTargetTexture2DOES);

            const EGLint pixmapConfigAttrs[] =
            {
                EGL_SURFACE_TYPE, EGL_PIXMAP_BIT | EGL_LOCK_SURFACE_BIT_KHR,
                EGL_MATCH_FORMAT_KHR,
                    (m_dataBitsPerPixel == 16) ? EGL_FORMAT_RGB_565_EXACT_KHR :
                                                 EGL_FORMAT_RGBA_8888_EXACT_KHR,
                EGL_BUFFER_SIZE, m_dataBitsPerPixel,
                EGL_NONE
            };
            EGLint configCount = 0;

            eglChooseConfig(ctx.dpy, pixmapConfigAttrs, &m_config, 1, &configCount);
            ASSERT(configCount);

            for (i = 0; i < m_buffers; i++)
            {
                success = nativeCreatePixmap(ctx.nativeDisplay, ctx.dpy,
                                             m_config, m_width, m_height, &m_pixmaps[i]);
                ASSERT(success);

                m_surfaces[i] = eglCreatePixmapSurface(ctx.dpy, m_config, m_pixmaps[i], NULL);
                ASSERT(m_surfaces[i] != EGL_NO_SURFACE);

                // Create an EGL image from the pixmap
                m_images[i] = m_eglCreateImageKHR(ctx.dpy, EGL_NO_CONTEXT,
                                                  EGL_NATIVE_PIXMAP_KHR,
                                                  (EGLClientBuffer)m_pixmaps[i],
                                                  NULL);
                ASSERT(m_images[i]);

                // Bind the image to a texture
                glBindTexture(GL_TEXTURE_2D, m_textures[i]);
                m_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_images[i]);
                ASSERT_GL();
                m_writeCompleted[i] = true;
            }
        }
        break;
    default:
        ASSERT(0);
        return;
    }
}

void CPUInterleavingTest::teardown()
{
    int i;
    glDeleteTextures(m_buffers, m_textures);

    switch (m_method)
    {
    case CPUI_TEXTURE_UPLOAD:
        {
            for (i = 0; i < m_buffers; i++)
            {
                delete[] m_textureData[i];
            }
        }
        break;
    case CPUI_XSHM_IMAGE:
        {
	    for (i = 0; i < m_buffers; i++)
	    {
                XShmDetach(ctx.nativeDisplay, &m_shminfo[i]);
                XDestroyImage(m_ximage[i]);
                shmdt(m_shminfo[i].shmaddr);
                shmctl(m_shminfo[i].shmid, IPC_RMID, 0);

                eglReleaseTexImage(ctx.dpy, m_surfaces[i], EGL_BACK_BUFFER);
                eglDestroySurface(ctx.dpy, m_surfaces[i]);
                nativeDestroyPixmap(ctx.nativeDisplay, m_pixmaps[i]);
                XFreeGC(ctx.nativeDisplay, m_gc[i]);
            }
        }
        break;
    case CPUI_EGL_LOCK_SURFACE:
        {
            for (i = 0; i < m_buffers; i++)
            {
                m_eglDestroyImageKHR(ctx.dpy, m_images[i]);
                eglDestroySurface(ctx.dpy, m_surfaces[i]);
                nativeDestroyPixmap(ctx.nativeDisplay, m_pixmaps[i]);
            }
        }
        break;
    default:
        ASSERT(0);
        return;
    }

    BlitTest::teardown();
}

std::string CPUInterleavingTest::name() const
{
    std::stringstream s;

    s << "blit_cpu_";

    switch (m_method)
    {
    case CPUI_TEXTURE_UPLOAD:
        s << "texupload";
        break;
    case CPUI_XSHM_IMAGE:
        s << "shmimage";
        break;
    case CPUI_IMG_TEXTURE_STREAMING:
        s << "texstream";
        break;
    case CPUI_PIXEL_BUFFER_OBJECT:
        s << "pbo";
        break;
    case CPUI_EGL_LOCK_SURFACE:
        s << "locksurf";
        break;
    }

    switch (m_dataBitsPerPixel)
    {
    case 16:
        s << "_16bpp";
        break;
    case 32:
        s << "_32bpp";
        break;
    }

    s << "_" << m_buffers << "x" << m_width << "x" << m_height;

    return s.str();
}

void CPUInterleavingTest::operator()(int frame)
{
    switch (m_method)
    {
    case CPUI_EGL_LOCK_SURFACE:
        {
            EGLint lockAttrs[] =
            {
                EGL_LOCK_USAGE_HINT_KHR, EGL_WRITE_SURFACE_BIT_KHR,
                EGL_NONE
            };
            m_eglLockSurfaceKHR(ctx.dpy, m_surfaces[m_writeBuffer], lockAttrs);
            eglQuerySurface(ctx.dpy, m_surfaces[m_writeBuffer], EGL_BITMAP_POINTER_KHR,
                            reinterpret_cast<EGLint*>(&m_textureData[m_writeBuffer]));
            eglQuerySurface(ctx.dpy, m_surfaces[m_writeBuffer], EGL_BITMAP_PITCH_KHR,
                            reinterpret_cast<EGLint*>(&m_dataStride));
        }
        break;
    default:
        break;
    }

    switch (m_dataBitsPerPixel)
    {
    case 16:
        fillTexture(reinterpret_cast<uint16_t*>(m_textureData[m_writeBuffer]),
                    m_width, m_height, m_dataStride, frame);
        break;
    case 32:
        fillTexture(reinterpret_cast<uint32_t*>(m_textureData[m_writeBuffer]),
                    m_width, m_height, m_dataStride, frame);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, m_textures[m_writeBuffer]);

    switch (m_method)
    {
    case CPUI_TEXTURE_UPLOAD:
        if (m_dataBitsPerPixel == 32)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, m_textureData[m_writeBuffer]);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0,
                         GL_RGB, GL_UNSIGNED_SHORT_5_6_5, m_textureData[m_writeBuffer]);
        }
        break;
    case CPUI_XSHM_IMAGE:
        {
            // Wait for the completion event for this buffer
            while (XEventsQueued(ctx.nativeDisplay, QueuedAfterReading) > 0 ||
                   !m_writeCompleted[m_writeBuffer])
            {
                XEvent event;
                XNextEvent(ctx.nativeDisplay, &event);
                if (event.type == m_completionEvent)
                {
                    XShmCompletionEvent* e = reinterpret_cast<XShmCompletionEvent*>(&event);
                    int i = m_drawableIndex[e->drawable];
                    m_writeCompleted[i] = true;
                }
            }
            XShmPutImage(ctx.nativeDisplay, m_pixmaps[m_writeBuffer], m_gc[m_writeBuffer],
                         m_ximage[m_writeBuffer], 0, 0, 0, 0, m_width, m_height, True);
            m_writeCompleted[m_writeBuffer] = false;
        }
        break;
    case CPUI_EGL_LOCK_SURFACE:
        {
            m_eglUnlockSurfaceKHR(ctx.dpy, m_surfaces[m_readBuffer]);
        }
        break;
    default:
        ASSERT(0);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, m_textures[m_readBuffer]);
    m_writeBuffer = (m_writeBuffer + 1) % m_buffers;
    m_readBuffer  = (m_readBuffer  + 1) % m_buffers;

    BlitTest::operator()(frame);
}
