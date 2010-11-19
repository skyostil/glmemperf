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
#ifndef CPUINTERLEAVINGTEST_H
#define CPUINTERLEAVINGTEST_H

#include "blittest.h"
#include "util.h"
#include "ext.h"

#include <map>

#include <X11/extensions/XShm.h>

enum CPUInterleavingMethod
{
    CPUI_TEXTURE_UPLOAD,
    CPUI_XSHM_IMAGE,
    /*CPUI_XSHM_PIXMAP, xshm pixmaps are generally not supported anymore */
    CPUI_IMG_TEXTURE_STREAMING,
    CPUI_PIXEL_BUFFER_OBJECT,
    CPUI_EGL_LOCK_SURFACE,
};

const int CPUI_MAX_BUFFERS = 2;

class CPUInterleavingTest: public BlitTest
{
    CPUInterleavingMethod m_method;
    int m_buffers;
    GLuint m_textures[CPUI_MAX_BUFFERS];

    char* m_textureData[CPUI_MAX_BUFFERS];
    int m_textureDataSize[CPUI_MAX_BUFFERS];
    int m_dataStride;
    int m_dataBitsPerPixel;

    int m_readBuffer;
    int m_writeBuffer;

    Pixmap m_pixmaps[CPUI_MAX_BUFFERS];
    EGLSurface m_surfaces[CPUI_MAX_BUFFERS];
    EGLImageKHR m_images[CPUI_MAX_BUFFERS];
    EGLConfig m_config;

    XShmSegmentInfo m_shminfo[CPUI_MAX_BUFFERS];
    XImage* m_ximage[CPUI_MAX_BUFFERS];
    GC m_gc[CPUI_MAX_BUFFERS];
    int m_completionEvent;
    std::map<Drawable, int> m_drawableIndex;
    bool m_writeCompleted[CPUI_MAX_BUFFERS];

    // Lock surface functions
    PFNEGLLOCKSURFACEKHRPROC m_eglLockSurfaceKHR;
    PFNEGLUNLOCKSURFACEKHRPROC m_eglUnlockSurfaceKHR;

    // EGLImage functions
    PFNEGLCREATEIMAGEKHRPROC m_eglCreateImageKHR;
    PFNEGLDESTROYIMAGEKHRPROC m_eglDestroyImageKHR;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC m_glEGLImageTargetTexture2DOES;

public:
    CPUInterleavingTest(CPUInterleavingMethod method, int buffers,
                        int bitsPerPixel,
                        int width, int height,
                        float texW = 1.0f, float texH = 1.0f);

    void prepare();
    void operator()(int frame);
    void teardown();

    std::string name() const;
};

#endif // CPUINTERLEAVINGTEST_H
