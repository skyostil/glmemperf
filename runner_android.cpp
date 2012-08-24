/**
 * OpenGL ES 2.0 memory performance estimator
 * Copyright (C) 2012 Google
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
 * \author Sami Kyöstilä <skyostil@google.com>
 */
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <time.h>
#include <list>

#include "native.h"
#include "util.h"
#include "test.h"
#include "blittest.h"
#include "cleartest.h"
#include "fboblittest.h"
#include "shaderblittest.h"
#include "cpuinterleavingtest.h"
#include "ext.h"

#include <android_native_app_glue.h>

typedef std::list<Test*> TestList;

class AppContext
{
public:
    AppContext(android_app* app):
        app(app),
        frameCount(0)
    {
    }

    android_app* app;
    TestList tests;
    TestList::iterator currentTest;
    int frameCount;
    struct timespec startTime;
};

/** Shared EGL objects */
struct Context ctx;

static bool render(AppContext* appContext)
{
    if (!appContext->tests.size() || appContext->currentTest == appContext->tests.end())
    {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        swapBuffers();
        return false;
    }
    Test* test = *appContext->currentTest;

    if (appContext->frameCount == 0)
    {
        try
        {
            test->prepare();
            ASSERT_GL();
            ASSERT_EGL();
        }
        catch (const std::exception& e)
        {
            LOGW("%s: %s", test->name().c_str(), e.what());
            ASSERT_GL();
            ASSERT_EGL();
            appContext->frameCount = 0;
            appContext->currentTest++;
            return false;
        }
    }

    const int warmUp = 20;
    if (appContext->frameCount++ < warmUp - 1)
    {
        (*test)(0);
        swapBuffers();
        return true;
    }

    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    int actualFrameCount = appContext->frameCount - warmUp;
    if (actualFrameCount == 0)
    {
        appContext->startTime = now;
    }

    const int64_t minTime = 2 * 1000 * 1000 * 1000LL;
    int64_t elapsed = timeDiff(appContext->startTime, now);
    if (elapsed > minTime)
    {
        int fps = static_cast<int>((1000 * 1000 * 1000LL * actualFrameCount) / elapsed);
        LOGI("%-40s %3d fps", (test->name() + ":").c_str(), fps);

        test->teardown();
        appContext->frameCount = 0;
        appContext->currentTest++;
    }
    else
    {
        (*test)(actualFrameCount);
        swapBuffers();
    }
    return true;
}

static void initializeTests(AppContext* appContext)
{
    EGLint winWidth = 0;
    EGLint winHeight = 0;
    EGLint w = 800;
    EGLint h = 480;

    eglQuerySurface(ctx.dpy, ctx.surface, EGL_WIDTH, &winWidth);
    eglQuerySurface(ctx.dpy, ctx.surface, EGL_HEIGHT, &winHeight);

    LOGI("Initializing tests");

#define ADD_TEST(TEST) appContext->tests.push_back(new TEST);
#include "tests.inl"
#undef ADD_TEST

    LOGI("Tests initialized at %dx%d resolution", winWidth, winHeight);
    appContext->currentTest = appContext->tests.begin();
}

static bool initializeEgl(AppContext* appContext)
{
    const EGLint configAttrs[] =
    {
        EGL_BUFFER_SIZE, 32,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    const EGLint contextAttrs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLint configCount = 0;

    LOGI("Initializing EGL");
    ctx.assetManager = appContext->app->activity->assetManager;
    ctx.dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    ASSERT_EGL();

    eglInitialize(ctx.dpy, NULL, NULL);
    eglChooseConfig(ctx.dpy, configAttrs, &ctx.config, 1, &configCount);
    ASSERT_EGL();

    if (!configCount)
    {
        LOGW("Config not found");
        goto out_error;
    }

    LOGI("Config attributes:");
    dumpConfig(ctx.dpy, ctx.config);

    EGLint format;
    eglGetConfigAttrib(ctx.dpy, ctx.config, EGL_NATIVE_VISUAL_ID, &format);
    ASSERT_EGL();

    ANativeWindow_setBuffersGeometry(appContext->app->window, 0, 0, format);

    ctx.context = eglCreateContext(ctx.dpy, ctx.config, EGL_NO_CONTEXT, contextAttrs);
    ASSERT_EGL();
    if (!ctx.context)
    {
        LOGW("Unable to create a context");
        goto out_error;
    }

    ctx.surface = eglCreateWindowSurface(ctx.dpy, ctx.config, appContext->app->window, NULL);
    ASSERT_EGL();
    if (!ctx.surface)
    {
        LOGW("Unable to create a surface");
        goto out_error;
    }

    eglMakeCurrent(ctx.dpy, ctx.surface, ctx.surface, ctx.context);
    ASSERT_EGL();

    eglSwapInterval(ctx.dpy, 0);
    ASSERT_EGL();
    initializeTests(appContext);

    return true;

out_error:
    eglMakeCurrent(ctx.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(ctx.dpy, ctx.surface);
    eglDestroyContext(ctx.dpy, ctx.context);
    eglTerminate(ctx.dpy);
    return false;
}

void terminateEgl(AppContext*)
{
    eglMakeCurrent(ctx.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(ctx.dpy, ctx.surface);
    eglDestroyContext(ctx.dpy, ctx.context);
    eglTerminate(ctx.dpy);
}

static void handleCommand(struct android_app* app, int32_t cmd)
{
    struct AppContext* appContext = (struct AppContext*)app->userData;

    switch (cmd)
    {
    case APP_CMD_INIT_WINDOW:
        if (appContext->app->window != NULL)
        {
            initializeEgl(appContext);
            render(appContext);
        }
        break;
    case APP_CMD_TERM_WINDOW:
        terminateEgl(appContext);
        break;
    }
}

extern "C" void android_main(struct android_app* state)
{
    struct AppContext appContext(state);

    // Make sure glue isn't stripped.
    app_dummy();

    state->userData = &appContext;
    state->onAppCmd = &handleCommand;

    appContext.app = state;

    while (true)
    {
        int ident;
        int events;
        struct android_poll_source* source;

        while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
        {
            if (source)
            {
                source->process(state, source);
            }
            if (state->destroyRequested != 0)
            {
                break;
            }
        }
        if (!render(&appContext) && appContext.tests.size())
        {
            break;
        }
    }

    while (appContext.tests.size())
    {
        delete appContext.tests.front();
        appContext.tests.pop_front();
    }

    LOGI("Tests done, exiting");
    exit(0);
}
