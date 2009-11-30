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
 * EGL and OpenGL ES utility functions
 */
#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <assert.h>

/**
 *  Verify that GL commands up to this point have not produced any errors.
 */
#define ASSERT_GL() \
    do \
    { \
        GLint err = glGetError(); \
        if (err) \
        { \
            printf("GL error 0x%x (%d) at %s:%d\n", err, err, __FILE__, __LINE__); \
            assert(!err); \
        } \
    } while (0)

/**
 *  Verify that EGL commands up to this point have not produced any errors.
 */
#define ASSERT_EGL() \
    do \
    { \
        EGLint err = eglGetError(); \
        if (err != EGL_SUCCESS) \
        { \
            printf("EGL error 0x%x (%d) at %s:%d\n", err, err, __FILE__, __LINE__); \
            assert(!err); \
        } \
    } while (0)

/**
 *  EGL context objects available to all tests
 */
struct Context
{
    EGLNativeDisplayType nativeDisplay;
    EGLConfig config;
    EGLNativeWindowType win;
    EGLDisplay dpy;
    EGLContext context;
    EGLSurface surface;
};

extern struct Context ctx;

/**
 *  Indicate that a frame is complete
 */
void swapBuffers();

/**
 *  Load a texture from a binary file
 *
 *  @param target               Texture target (usually GL_TEXTURE_2D)
 *  @param level                Mipmap level
 *  @param internalFormat       Internal texture format
 *  @param width                Texture width in pixels
 *  @param height               Texture height in pixels
 *  @param type                 Data type
 *  @param fileName             File containing the texture data
 *
 *  @returns true on success, false on failure
 */
bool loadRawTexture(GLenum target, int level, GLenum internalFormat, int width,
                    int height, GLenum format, GLenum type, const std::string& fileName);

/**
 *  Load a compressed texture from a binary file
 *
 *  @param target               Texture target (usually GL_TEXTURE_2D)
 *  @param internalFormat       Internal texture format
 *  @param width                Texture width in pixels
 *  @param height               Texture height in pixels
 *  @param fileName             File containing the texture data
 *
 *  @returns true on success, false on failure
 */
bool loadCompressedTexture(GLenum target, int level, GLenum internalFormat, int width,
                           int height, const std::string& fileName);

/**
 *  Check whether an EGL extension is supported
 *
 *  @param name                 Extension name
 *
 *  @returns true if extension is supported
 */
bool isEGLExtensionSupported(const std::string& name);

/**
 *  Check whether an OpenGL ES extension is supported
 *
 *  @param name                 Extension name
 *
 *  @returns true if extension is supported
 */
bool isGLExtensionSupported(const std::string& name);

/**
 *  Compile a vertex and fragment shader and create a new program from the
 *  result
 *
 *  @param vertSrc              Vertex program source
 *  @param fragSrc              Fragment program source
 *
 *  @returns new program handle
 */
GLint createProgram(const std::string& vertSrc, const std::string& fragSrc);

/**
 *  Describe a texture format and type combination
 *
 *  @param format               Texture format
 *  @param type                 Texture type
 */
std::string textureFormatName(GLenum format, GLenum type);

/**
 *  Print EGL config attributes on the terminal
 *
 *  @param dpy                  EGL display
 *  @param config               EGL config
 */
void dumpConfig(EGLDisplay dpy, EGLConfig config);

#endif // UTIL_H
