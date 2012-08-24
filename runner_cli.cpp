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
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <time.h>
#include <memory>
#include <list>
#include <iostream>
#include <algorithm>

#include "native.h"
#include "util.h"
#include "test.h"
#include "blittest.h"
#include "cleartest.h"
#include "fboblittest.h"
#include "shaderblittest.h"
#include "cpuinterleavingtest.h"
#include "ext.h"

#if defined(SUPPORT_X11)
#include <X11/Xutil.h>
#include "pixmapblittest.h"
#include "blitmultitest.h"
#endif

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(HAVE_LIBOSSO)
#include <libosso.h>
#endif

/**
 *  Command line options
 */
static struct
{
    int                    bitsPerPixel;
    bool                   verbose;
    int                    minTime;
    bool                   listTests;
    std::list<std::string> includedTests;
    std::list<std::string> excludedTests;
} options;

/** Shared EGL objects */
struct Context ctx;

#if defined(HAVE_LIBOSSO)
osso_context_t* ossoContext;
#endif

bool initializeEgl(int width, int height, const EGLint* configAttrs, const EGLint* contextAttrs)
{
    EGLint configCount = 0;

#if defined(HAVE_LIBOSSO)
    ossoContext = osso_initialize("com.nokia.memperf", "1.0", FALSE, NULL);
    if (!ossoContext)
    {
        printf("Warning: osso_initialize failed\n");
    }
#endif

    ctx.dpy = eglGetDisplay(ctx.nativeDisplay);
    ASSERT_EGL();

    eglInitialize(ctx.dpy, NULL, NULL);
    eglChooseConfig(ctx.dpy, configAttrs, &ctx.config, 1, &configCount);
    ASSERT_EGL();

    if (!configCount)
    {
        printf("Config not found\n");
        goto out_error;
    }

    if (options.verbose)
    {
        printf("Config attributes:\n");
        dumpConfig(ctx.dpy, ctx.config);
    }

    if (!nativeCreateWindow(ctx.nativeDisplay, ctx.dpy, ctx.config, __FILE__,
                            width, height, &ctx.win))
    {
        printf("Unable to create a window\n");
        goto out_error;
    }

    ctx.context = eglCreateContext(ctx.dpy, ctx.config, EGL_NO_CONTEXT, contextAttrs);
    ASSERT_EGL();
    if (!ctx.context)
    {
        printf("Unable to create a context\n");
        goto out_error;
    }

    ctx.surface = eglCreateWindowSurface(ctx.dpy, ctx.config, ctx.win, NULL);
    ASSERT_EGL();
    if (!ctx.surface)
    {
        printf("Unable to create a surface\n");
        goto out_error;
    }

    eglMakeCurrent(ctx.dpy, ctx.surface, ctx.surface, ctx.context);
    ASSERT_EGL();

    eglSwapInterval(ctx.dpy, 0);
    return true;

out_error:
    eglMakeCurrent(ctx.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(ctx.dpy, ctx.surface);
    eglDestroyContext(ctx.dpy, ctx.context);
    eglTerminate(ctx.dpy);
    nativeDestroyWindow(ctx.nativeDisplay, ctx.win);
    nativeDestroyDisplay(ctx.nativeDisplay);
    return false;
}

void terminateEgl()
{
    eglMakeCurrent(ctx.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(ctx.dpy, ctx.surface);
    eglDestroyContext(ctx.dpy, ctx.context);
    eglTerminate(ctx.dpy);
    nativeDestroyWindow(ctx.nativeDisplay, ctx.win);
    nativeDestroyDisplay(ctx.nativeDisplay);

#if defined(HAVE_LIBOSSO)
    if (ossoContext)
    {
        osso_deinitialize(ossoContext);
        ossoContext = 0;
    }
#endif
}

bool shouldRunTest(const std::string& testName)
{
    std::list<std::string>::iterator i;
    bool result = true;

    if (options.includedTests.size())
    {
        result = false;
        for (i = options.includedTests.begin(); i != options.includedTests.end(); ++i)
        {
            if (testName.find(*i) != std::string::npos)
            {
                result = true;
                break;
            }
        }
    }

    for (i = options.excludedTests.begin(); i != options.excludedTests.end(); ++i)
    {
        if (testName.find(*i) != std::string::npos)
        {
            result = false;
            break;
        }
    }

    return result;
}

void runTest(Test& test)
{
    int frames = 0;
    int frameLimit = 100;
    int warmup = 20;
    int64_t minTime = options.minTime * 1000 * 1000 * 1000LL;
    struct timespec res, start, end;

    if (options.listTests)
    {
        printf("%s\n", test.name().c_str());
        return;
    }

    if (!shouldRunTest(test.name()))
    {
        return;
    }

    clock_getres(CLOCK_REALTIME, &res);
    //printf("Timer resolution: %d.%09d s\n", res.tv_sec, res.tv_nsec);
    printf("%-40s", (test.name() + ":").c_str());
    fflush(stdout);

    try
    {
        test.prepare();
        ASSERT_GL();
        ASSERT_EGL();
    } catch (const std::exception& e)
    {
        printf("%s\n", e.what());
        return;
    }

    nativeVerifyWindow(ctx.nativeDisplay, ctx.win);

    while (warmup--)
    {
        test(0);
        swapBuffers();
    }

    ASSERT_GL();
    ASSERT_EGL();

#if defined(HAVE_LIBOSSO)
    if (ossoContext)
    {
        osso_display_blanking_pause(ossoContext);
    }
#endif

    clock_gettime(CLOCK_REALTIME, &start);
    while (frames < frameLimit)
    {
        test(frames);
        swapBuffers();
        clock_gettime(CLOCK_REALTIME, &end);
        frames++;
        if (frames >= frameLimit && timeDiff(start, end) < minTime)
        {
            frameLimit *= 2;
        }
    }

    ASSERT_GL();
    ASSERT_EGL();

    test.teardown();
    ASSERT_GL();
    ASSERT_EGL();

    int64_t diff = timeDiff(start, end);
    int fps = static_cast<int>((1000 * 1000 * 1000LL * frames) / diff);
    //printf("%d frames in %6.2f ms (%3d fps) ", frames, diff / (1000.0f * 1000.0f), fps);
    printf("%3d fps ", fps);

    while (fps > 0)
    {
        fputc('#', stdout);
        fps -= 3;
    }
    fputc('\n', stdout);
}

void showIntro()
{
    std::cout <<
        "GLMemPerf v" PACKAGE_VERSION " - OpenGL ES 2.0 memory performance benchmark\n"
        "Copyright (C) 2010 Nokia Corporation. GLMemPerf comes with ABSOLUTELY\n"
        "NO WARRANTY; This is free software, and you are welcome to redistribute\n"
        "it under certain conditions.\n"
        "\n";
}

void showUsage()
{
    std::cout <<
        "Usage:\n"
        "       glmemperf [OPTIONS]\n"
        "Options:\n"
        "       -h             This text\n"
        "       -v             Verbose mode\n"
        "       -l             List all tests without running them\n"
        "       -i TEST        Include a specific test (full name or substring)\n"
        "       -e TEST        Exclude a specific test (full name or substring)\n"
        "       -t SECS        Minimum time to run each test\n"
        "       -b BPP         Bits per pixel\n";
}

void parseArguments(const std::list<std::string>& args)
{
    std::list<std::string>::const_iterator i;

    // Set up defaults
    options.verbose = false;
    options.minTime = 1;
    options.bitsPerPixel = 16;
    options.listTests = false;

    for (i = args.begin(), i++; i != args.end(); ++i)
    {
        if (*i == "-h" || *i == "--help")
        {
            showUsage();
            exit(0);
        }
        else if (*i == "-i" && ++i != args.end())
        {
            options.includedTests.push_back(*i);
        }
        else if (*i == "-e" && ++i != args.end())
        {
            options.excludedTests.push_back(*i);
        }
        else if (*i == "-t" && ++i != args.end())
        {
            options.minTime = atoi((*i).c_str());
        }
        else if (*i == "-b" && ++i != args.end())
        {
            options.bitsPerPixel = atoi((*i).c_str());
        }
        else if (*i == "-v")
        {
            options.verbose = true;
        }
        else if (*i == "-l")
        {
            options.listTests = true;
        }
        else
        {
            std::cerr << "Invalid option: " << *i << std::endl;
            showUsage();
            exit(1);
        }
    }
}

void findDataDirectory()
{
    struct stat st;
    if (stat("data", &st) == 0)
    {
        return;
    }
    chdir(PREFIX "/share/glmemperf/");
    ASSERT(stat("data", &st) == 0);
}

int main(int argc, char** argv)
{
    std::list<std::string> args(argv, argv + argc);

    showIntro();
    parseArguments(args);
    findDataDirectory();

    const EGLint configAttrs[] =
    {
        EGL_BUFFER_SIZE, options.bitsPerPixel,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    const EGLint configAttrs32[] =
    {
        EGL_BUFFER_SIZE, 32,
        EGL_NONE
    };

    const EGLint contextAttrs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    int winWidth = 800;
    int winHeight = 480;
    const float w = winWidth, h = winHeight;
    EGLConfig config32 = 0;
    EGLint configCount = 0;

    bool result = nativeCreateDisplay(&ctx.nativeDisplay);
    ASSERT(result);

    nativeGetScreenSize(ctx.nativeDisplay, &winWidth, &winHeight);
    result = initializeEgl(winWidth, winHeight, configAttrs, contextAttrs);
    ASSERT(result);

    eglChooseConfig(ctx.dpy, configAttrs32, &config32, 1, &configCount);

    if (!configCount)
    {
        printf("32bpp config not found\n");
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    ASSERT_GL();

#define ADD_TEST(TEST) runTest(*std::auto_ptr<Test>(new TEST));
#include "tests.inl"
#undef ADD_TEST

    terminateEgl();
}
