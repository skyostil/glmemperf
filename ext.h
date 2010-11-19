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
 */
#ifndef EXT_H
#define EXT_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2ext.h>

#ifndef EGL_KHR_lock_surface
#define EGL_KHR_lock_surface 1
#define EGL_READ_SURFACE_BIT_KHR                0x0001  /* EGL_LOCK_USAGE_HINT_KHR bitfield */
#define EGL_WRITE_SURFACE_BIT_KHR               0x0002  /* EGL_LOCK_USAGE_HINT_KHR bitfield */
#define EGL_LOCK_SURFACE_BIT_KHR                0x0080  /* EGL_SURFACE_TYPE bitfield */
#define EGL_OPTIMAL_FORMAT_BIT_KHR              0x0100  /* EGL_SURFACE_TYPE bitfield */
#define EGL_MATCH_FORMAT_KHR                    0x3043  /* EGLConfig attribute */
#define EGL_FORMAT_RGB_565_EXACT_KHR            0x30C0  /* EGL_MATCH_FORMAT_KHR value */
#define EGL_FORMAT_RGB_565_KHR                  0x30C1  /* EGL_MATCH_FORMAT_KHR value */
#define EGL_FORMAT_RGBA_8888_EXACT_KHR          0x30C2  /* EGL_MATCH_FORMAT_KHR value */
#define EGL_FORMAT_RGBA_8888_KHR                0x30C3  /* EGL_MATCH_FORMAT_KHR value */
#define EGL_MAP_PRESERVE_PIXELS_KHR             0x30C4  /* eglLockSurfaceKHR attribute */
#define EGL_LOCK_USAGE_HINT_KHR                 0x30C5  /* eglLockSurfaceKHR attribute */
#define EGL_BITMAP_POINTER_KHR                  0x30C6  /* eglQuerySurface attribute */
#define EGL_BITMAP_PITCH_KHR                    0x30C7  /* eglQuerySurface attribute */
#define EGL_BITMAP_ORIGIN_KHR                   0x30C8  /* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_RED_OFFSET_KHR         0x30C9  /* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR       0x30CA  /* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR        0x30CB  /* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR       0x30CC  /* eglQuerySurface attribute */
#define EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR   0x30CD  /* eglQuerySurface attribute */
#define EGL_LOWER_LEFT_KHR                      0x30CE  /* EGL_BITMAP_ORIGIN_KHR value */
#define EGL_UPPER_LEFT_KHR                      0x30CF  /* EGL_BITMAP_ORIGIN_KHR value */

typedef EGLBoolean (EGLAPIENTRYP PFNEGLLOCKSURFACEKHRPROC) (EGLDisplay display, EGLSurface surface, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLUNLOCKSURFACEKHRPROC) (EGLDisplay display, EGLSurface surface);
#endif

#ifndef EGL_KHR_lock_surface2
#define EGL_KHR_lock_surface2 1
#define EGL_BITMAP_PIXEL_SIZE_KHR               0x3110
#endif

/* GL_IMG_texture_compression_pvrtc */
#ifndef GL_IMG_texture_compression_pvrtc
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03
#endif

#endif // EXT_H
