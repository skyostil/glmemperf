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
#include <X11/Xutil.h>
#include <EGL/egl.h>
#include <string>
#include <unistd.h>
#include <time.h>
#include <memory>
#include <list>
#include <iostream>
#include <algorithm>
#include <sys/stat.h>

#include "native.h"
#include "util.h"
#include "test.h"
#include "blittest.h"
#include "cleartest.h"
#include "pixmapblittest.h"
#include "fboblittest.h"
#include "shaderblittest.h"
#include "cpuinterleavingtest.h"
#include "config.h"

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
    osso_deinitialize(ossoContext);
    ossoContext = 0;
#endif
}

int64_t timeDiff(const struct timespec& start, const struct timespec& end)
{
    int64_t s = start.tv_sec * (1000 * 1000 * 1000LL) + start.tv_nsec;
    int64_t e =   end.tv_sec * (1000 * 1000 * 1000LL) +   end.tv_nsec;
    return e - s;
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

    while (warmup--)
    {
        test(0);
        swapBuffers();
    }

    ASSERT_GL();
    ASSERT_EGL();

#if defined(HAVE_LIBOSSO)
    osso_display_blanking_pause(ossoContext);
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

void getScreenSize(int& width, int& height)
{
    Window rootWindow = DefaultRootWindow(ctx.nativeDisplay);
    XWindowAttributes rootAttrs;

    XGetWindowAttributes(ctx.nativeDisplay, rootWindow, &rootAttrs);

    width = rootAttrs.width;
    height = rootAttrs.height;
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
    assert(stat("data", &st) == 0);
}

#define ADD_TEST(TEST) runTest(*std::auto_ptr<Test>(new TEST));

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
    assert(result);

    getScreenSize(winWidth, winHeight);
    result = initializeEgl(winWidth, winHeight, configAttrs, contextAttrs);
    assert(result);

    eglChooseConfig(ctx.dpy, configAttrs32, &config32, 1, &configCount);

    if (!configCount)
    {
        printf("32bpp config not found\n");
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    ASSERT_GL();

    // Clear test
    ADD_TEST(ClearTest());

    // Normal blits
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              800, 480, "data/water2_800x480_rgba8888.raw"));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_BYTE,              800, 480, "data/water2_800x480_rgb888.raw"));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              864, 480, "data/water2_864x480_rgba8888.raw"));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_BYTE,              864, 480, "data/water2_864x480_rgb888.raw"));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,             1024, 512, "data/digital_nature2_1024x512_rgba8888.raw", false, 800.0 / 1024, 480.0 / 512));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       800, 480, "data/water2_800x480_rgb565.raw"));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,      1024, 512, "data/digital_nature2_1024x512_rgb565.raw", false, 800.0 / 1024, 480.0 / 512));
    ADD_TEST(BlitTest(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 0, 1024, 512, "data/abstract3_1024x512_pvrtc4.raw", false, 800.0 / 1024, 480.0 / 512));
    ADD_TEST(BlitTest(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 0, 1024, 512, "data/abstract3_1024x512_pvrtc2.raw", false, 800.0 / 1024, 480.0 / 512));
    ADD_TEST(BlitTest(GL_ETC1_RGB8_OES,                   0, 1024, 512, "data/abstract3_1024x512_etc1.raw", false, 800.0 / 1024, 480.0 / 512));
    ADD_TEST(PixmapBlitTest(w, h, ctx.config));
    ADD_TEST(PixmapBlitTest(w, h, config32));
    ADD_TEST(FBOBlitTest(GL_RGBA, GL_UNSIGNED_BYTE,          w, h));
    ADD_TEST(FBOBlitTest(GL_RGBA, GL_UNSIGNED_BYTE,          1024, 512, false, w / 1024, h / 512));
    ADD_TEST(FBOBlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,   w, h));
    ADD_TEST(FBOBlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,   1024, 512, false, w / 1042, h / 512));

    // Rotated blits
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              480,  800, "data/water2_480x800_rgba8888.raw", true));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_BYTE,              480,  800, "data/water2_480x800_rgb888.raw", true));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              480,  864, "data/water2_480x864_rgba8888.raw", true));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_BYTE,              480,  864, "data/water2_480x864_rgb888.raw", true));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              512, 1024, "data/digital_nature2_512x1024_rgba8888.raw", true, 480.0 / 512, 800.0 / 1024));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       480,  800, "data/water2_480x800_rgb565.raw", true));
    ADD_TEST(BlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,       512, 1024, "data/digital_nature2_512x1024_rgb565.raw", true, 480.0 / 512, 800.0 / 1024));
    ADD_TEST(BlitTest(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 0,  512, 1024, "data/abstract3_512x1024_pvrtc4.raw", true, 480.0 / 512, 800.0 / 1024));
    ADD_TEST(BlitTest(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 0,  512, 1024, "data/abstract3_512x1024_pvrtc2.raw", true, 480.0 / 512, 800.0 / 1024));
    ADD_TEST(BlitTest(GL_ETC1_RGB8_OES,                   0,  512, 1024, "data/abstract3_512x1024_etc1.raw", true, 480.0 / 512, 800.0 / 1024));
    ADD_TEST(PixmapBlitTest(h, w, ctx.config, true));
    ADD_TEST(PixmapBlitTest(h, w, config32,   true));
    ADD_TEST(FBOBlitTest(GL_RGBA, GL_UNSIGNED_BYTE,        w, h, true, h / w, w / h));
    ADD_TEST(FBOBlitTest(GL_RGBA, GL_UNSIGNED_BYTE,        1024, 512, true, h / 512, w / 1024));
    ADD_TEST(FBOBlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5, w, h, true, h / w, w / h));
    ADD_TEST(FBOBlitTest(GL_RGB,  GL_UNSIGNED_SHORT_5_6_5, 1024, 512, true, h / 512, w / 1024));

    int gridW = 5;
    int gridH = 3;
    float w2 = winWidth  / gridW;
    float h2 = winHeight / gridH;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Small blended blits
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,     128, 128, "data/xorg_128x128_rgba4444.raw", false, gridW, gridH, 128.0 / w2, 128.0 / h2));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              127, 127, "data/xorg_127x127_rgba8888.raw", false, gridW, gridH, 127.0 / w2, 127.0 / h2));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              128, 128, "data/xorg_128x128_rgba8888.raw", false, gridW, gridH, 128.0 / w2, 128.0 / h2));
    ADD_TEST(BlitTest(GL_RGB, GL_UNSIGNED_SHORT_5_6_5,        127, 127, "data/xorg_127x127_rgb565.raw",   false, gridW, gridH, 127.0 / w2, 127.0 / h2));
    ADD_TEST(BlitTest(GL_RGB, GL_UNSIGNED_SHORT_5_6_5,        128, 128, "data/xorg_128x128_rgb565.raw",   false, gridW, gridH, 128.0 / w2, 128.0 / h2));
    ADD_TEST(BlitTest(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 0, 128, 128, "data/xorg_128x128_pvrtc4.raw",   false, gridW, gridH, 128.0 / w2, 128.0 / h2));
    ADD_TEST(BlitTest(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 0, 128, 128, "data/xorg_128x128_pvrtc2.raw",   false, gridW, gridH, 128.0 / w2, 128.0 / h2));
    ADD_TEST(BlitTest(GL_ETC1_RGB8_OES,                    0, 128, 128, "data/xorg_128x128_etc1.raw",     false, gridW, gridH, 128.0 / w2, 128.0 / h2));
    ADD_TEST(ShaderBlitTest("mask", 128, 128, gridW, gridH * 0.5f, 128.0 / w2, 128.0 / h2));

    // Rotated small blended blits
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              127, 127, "data/xorg_127x127_rgba8888.raw", true, gridH, gridW, 127.0 / w2, 127.0 / h2));
    ADD_TEST(BlitTest(GL_RGBA, GL_UNSIGNED_BYTE,              128, 128, "data/xorg_128x128_rgba8888.raw", true, gridH, gridW, 128.0 / w2, 128.0 / h2));
    ADD_TEST(BlitTest(GL_RGB, GL_UNSIGNED_SHORT_5_6_5,        127, 127, "data/xorg_127x127_rgb565.raw",   true, gridH, gridW, 127.0 / w2, 127.0 / h2));
    ADD_TEST(BlitTest(GL_RGB, GL_UNSIGNED_SHORT_5_6_5,        128, 128, "data/xorg_128x128_rgb565.raw",   true, gridH, gridW, 128.0 / w2, 128.0 / h2));
    ADD_TEST(BlitTest(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 0, 128, 128, "data/xorg_128x128_pvrtc4.raw",   true, gridH, gridW, 128.0 / w2, 128.0 / h2));
    ADD_TEST(BlitTest(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 0, 128, 128, "data/xorg_128x128_pvrtc2.raw",   true, gridH, gridW, 128.0 / w2, 128.0 / h2));
    ADD_TEST(BlitTest(GL_ETC1_RGB8_OES,                    0, 128, 128, "data/xorg_128x128_etc1.raw",     true, gridH, gridW, 128.0 / w2, 128.0 / h2));

    glDisable(GL_BLEND);

    // Shader tests
    ADD_TEST(ShaderBlitTest("const", w, h));
    ADD_TEST(ShaderBlitTest("lingrad", w, h));
    ADD_TEST(ShaderBlitTest("radgrad", w, h));
    ADD_TEST(ShaderBlitTest("palette", w, h));
    ADD_TEST(ShaderBlitTest("blur", w, h));
    
    // CPU interleaving
    ADD_TEST(CPUInterleavingTest(CPUI_XSHM_IMAGE, 2, 16, winWidth, winHeight));
    ADD_TEST(CPUInterleavingTest(CPUI_XSHM_IMAGE, 2, 32, winWidth, winHeight));
    ADD_TEST(CPUInterleavingTest(CPUI_TEXTURE_UPLOAD, 2, 16, winWidth, winHeight));
    ADD_TEST(CPUInterleavingTest(CPUI_TEXTURE_UPLOAD, 2, 32, winWidth, winHeight));

    ADD_TEST(CPUInterleavingTest(CPUI_EGL_LOCK_SURFACE, 2, 16, winWidth, winHeight));
    ADD_TEST(CPUInterleavingTest(CPUI_EGL_LOCK_SURFACE, 2, 32, winWidth, winHeight));

    terminateEgl();
}
