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
#include "util.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <GLES2/gl2ext.h>

void swapBuffers()
{
#if 1
    eglSwapBuffers(ctx.dpy, ctx.surface);
#else
    GLint pixel[4];
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
#endif
}

bool loadRawTexture(GLenum target, int level, GLenum internalFormat, int width,
                    int height, GLenum format, GLenum type, const std::string& fileName)
{
    int fd = open(fileName.c_str(), O_RDONLY); 

    if (fd == -1)
    {
        perror("open");
        return false;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        perror("stat");
        return false;
    }

    void* pixels = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    glTexImage2D(target, level, internalFormat, width, height, 0, format, type, pixels);

    munmap(pixels, sb.st_size);
    close(fd);

    ASSERT_GL();
    return true;
}

bool loadCompressedTexture(GLenum target, int level, GLenum internalFormat, int width,
                           int height, const std::string& fileName)
{
    int fd = open(fileName.c_str(), O_RDONLY); 

    if (fd == -1)
    {
        perror("open");
        return false;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        perror("stat");
        return false;
    }

    void* pixels = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    glCompressedTexImage2D(target, level, internalFormat, width, height, 0, sb.st_size, pixels);

    munmap(pixels, sb.st_size);
    close(fd);

    ASSERT_GL();
    return true;
}

GLint createProgram(const std::string& vertSrc, const std::string& fragSrc)
{
    GLint success = 0;
    GLint logLength = 0;
    char infoLog[1024];
    const char* vs = vertSrc.c_str();
    const char* fs = fragSrc.c_str();

    GLint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, 0);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), &logLength, infoLog);
        printf("Vertex shader compilation failed:\n%s\n", infoLog);
    }
    assert(success);

    GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, 0);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), &logLength, infoLog);
        printf("Fragment shader compilation failed:\n%s\n", infoLog);
    }
    assert(success);

    GLint program = glCreateProgram();
    glAttachShader(program, fragmentShader);
    glAttachShader(program, vertexShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, sizeof(infoLog), &logLength, infoLog);
        printf("Program linking failed:\n%s\n", infoLog);
    }
    assert(success);
    return program;
} 

std::string textureFormatName(GLenum format, GLenum type)
{
    switch (type)
    {
    case GL_UNSIGNED_BYTE:
        return ((format == GL_RGB) ? "rgb888" : "rgba8888");
    case GL_UNSIGNED_SHORT_5_6_5:
        return "rgb565";
    case GL_UNSIGNED_SHORT_4_4_4_4:
        return "rgba4444";
    case GL_UNSIGNED_SHORT_5_5_5_1:
        return "rgba5551";
    case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
        return "rgb_pvrtc4";
    case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
        return "rgb_pvrtc2";
    case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
        return "rgba_pvrtc4";
    case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
        return "rgba_pvrtc2";
    case GL_ETC1_RGB8_OES:
        return "rgb_etc1";
    default:
        return "unknown";
    }
}

#define DUMP_CFG_ATTRIB(attr, consts, bits) \
    do \
    { \
        EGLint value; \
        eglGetConfigAttrib(dpy, config, attr, &value); \
	ASSERT_EGL(); \
        printf("%-32s: %10d (0x%x)\n", #attr, value, value); \
        { \
            unsigned i; \
            for (i = 0; i < sizeof(consts) / sizeof(consts[0]); i++) \
            { \
                if (value == consts[i].value) \
                { \
                    printf("%-44s %s\n", "", consts[i].name); \
                } \
            } \
        } \
        { \
            unsigned i; \
            for (i = 0; i < sizeof(bits) / sizeof(bits[0]); i++) \
            { \
                if (value & bits[i].value) \
                { \
                    printf("%-44s %s\n", "", bits[i].name); \
                } \
            } \
        } \
    } while (0)

#define DECLARE_CONSTANTS(NAME) \
    static const struct { const char* name; EGLint value; } NAME[] =

#define C(constant) {#constant, constant}

#define DECLARE_BITFIELD(NAME) \
    static const struct { const char* name; EGLint value; } NAME[] =

#define B(bit) {#bit, bit}

DECLARE_CONSTANTS(defaultConsts)
{
};

DECLARE_BITFIELD(defaultBits)
{
};

DECLARE_CONSTANTS(configCaveatConsts)
{
    C(EGL_NONE),
    C(EGL_SLOW_CONFIG),
    C(EGL_NON_CONFORMANT_CONFIG),
};

DECLARE_CONSTANTS(transparentTypeConsts)
{
    C(EGL_NONE),
    C(EGL_TRANSPARENT_RGB),
};

DECLARE_CONSTANTS(colorBufferTypeConsts)
{
    C(EGL_NONE),
    C(EGL_RGB_BUFFER),
    C(EGL_LUMINANCE_BUFFER),
};

DECLARE_BITFIELD(surfaceTypeBits)
{
    B(EGL_PBUFFER_BIT),
    B(EGL_PIXMAP_BIT),
    B(EGL_WINDOW_BIT),
    B(EGL_VG_COLORSPACE_LINEAR_BIT),
    B(EGL_VG_ALPHA_FORMAT_PRE_BIT),
    B(EGL_MULTISAMPLE_RESOLVE_BOX_BIT),
    B(EGL_SWAP_BEHAVIOR_PRESERVED_BIT),
};

DECLARE_BITFIELD(renderableTypeBits)
{
    B(EGL_OPENGL_ES_BIT),
    B(EGL_OPENVG_BIT),
    B(EGL_OPENGL_ES2_BIT),
    B(EGL_OPENGL_BIT),
};

#undef C
#undef DECLARE_CONSTANTS
#undef B
#undef DECLARE_BITFIELD

void dumpConfig(EGLDisplay dpy, EGLConfig config)
{
    DUMP_CFG_ATTRIB(EGL_BUFFER_SIZE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_ALPHA_SIZE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_BLUE_SIZE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_GREEN_SIZE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_RED_SIZE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_DEPTH_SIZE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_STENCIL_SIZE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_CONFIG_CAVEAT, configCaveatConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_CONFIG_ID, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_LEVEL, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_MAX_PBUFFER_HEIGHT, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_MAX_PBUFFER_PIXELS, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_MAX_PBUFFER_WIDTH, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_NATIVE_RENDERABLE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_NATIVE_VISUAL_ID, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_NATIVE_VISUAL_TYPE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_PRESERVED_RESOURCES, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_SAMPLES, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_SAMPLE_BUFFERS, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_SURFACE_TYPE, defaultConsts, surfaceTypeBits);
    DUMP_CFG_ATTRIB(EGL_TRANSPARENT_TYPE, transparentTypeConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_TRANSPARENT_BLUE_VALUE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_TRANSPARENT_GREEN_VALUE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_TRANSPARENT_RED_VALUE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_BIND_TO_TEXTURE_RGB, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_BIND_TO_TEXTURE_RGBA, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_MIN_SWAP_INTERVAL, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_MAX_SWAP_INTERVAL, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_LUMINANCE_SIZE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_ALPHA_MASK_SIZE, defaultConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_COLOR_BUFFER_TYPE, colorBufferTypeConsts, defaultBits);
    DUMP_CFG_ATTRIB(EGL_RENDERABLE_TYPE, defaultConsts, renderableTypeBits);
    DUMP_CFG_ATTRIB(EGL_CONFORMANT, defaultConsts, renderableTypeBits);
}

#undef DUMP_CFG_ATTRIB

/* This is after http://www.opengl.org/resources/features/OGLextensions/ */
static bool isExtensionSupported(const std::string& extensions, const std::string& name)
{
    const char *start;
    const char *where, *terminator;

    /* Extension names should not have spaces. */
    where = strchr(name.c_str(), ' ');
    if (where || !name.size())
    {
        return 0;
    }

    /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
    start = extensions.c_str();
    for (;;)
    {
	where = strstr(start, name.c_str());
	if (!where)
	    break;
	terminator = where + strlen(name.c_str());
	if (where == start || *(where - 1) == ' ')
	    if (*terminator == ' ' || *terminator == '\0')
	        return 1;
	start = terminator;
    }
    return 0;
}

bool isEGLExtensionSupported(const std::string& name)
{
    return isExtensionSupported(eglQueryString(ctx.dpy, EGL_EXTENSIONS), name);
}

bool isGLExtensionSupported(const std::string& name)
{
    return isExtensionSupported((const char*)glGetString(GL_EXTENSIONS), name);
}
