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
 *  Native windowing implementation for X11
 */
#include <EGL/egl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "native.h"

EGLBoolean nativeCreateDisplay(EGLNativeDisplayType *pNativeDisplay)
{
    *pNativeDisplay = XOpenDisplay(NULL);

    if (!*pNativeDisplay)
    {
        fprintf(stderr, "XOpenDisplay failed\n");
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

void nativeDestroyDisplay(EGLNativeDisplayType nativeDisplay)
{
    XCloseDisplay(nativeDisplay);
}

static int runningOnFremantle(void)
{
    /* Somewhat hacky way of detecting fremantle */
    FILE* f = popen("pgrep hildon-desktop", "r");
    int found = 0;

    while (!feof(f))
    {
        int pid;
        if (fscanf(f, "%d", &pid) == 1)
        {
            found = 1;
        }
    }
    pclose(f);
    return found;
}

EGLBoolean nativeCreateWindow(EGLNativeDisplayType nativeDisplay, EGLDisplay dpy, EGLConfig config, 
                              const char *title, int width, int height, EGLNativeWindowType *nativeWindow)
{
    Window rootWindow = DefaultRootWindow(nativeDisplay);
    EGLint visualId;
    Window window;
    XVisualInfo* visual;
    XTextProperty windowTitle;
    XSetWindowAttributes winAttrs;
    XWindowAttributes rootAttrs;
    XVisualInfo visualInfo;
    int visualCount = 0;

    if (eglGetConfigAttrib(dpy, config, EGL_NATIVE_VISUAL_ID, &visualId) != EGL_TRUE)
    {
        fprintf(stderr, "eglGetConfigAttrib failed: %x\n", eglGetError());
        return EGL_FALSE;
    }

    visualInfo.visualid = visualId;
    visualInfo.screen = DefaultScreen(nativeDisplay);
    visual = XGetVisualInfo(nativeDisplay, VisualIDMask | VisualScreenMask, &visualInfo, &visualCount);

    if (!visualCount)
    {
        fprintf(stderr, "XGetVisualInfo failed\n");
        return EGL_FALSE;
    }

    XGetWindowAttributes(nativeDisplay, rootWindow, &rootAttrs);

    winAttrs.background_pixmap = None;
    winAttrs.border_pixel = 0;
    winAttrs.colormap = XCreateColormap(nativeDisplay, rootWindow, visual->visual, AllocNone);

    window = XCreateWindow(nativeDisplay, rootWindow, 0, 0,
                           width, height, 0, visual->depth,
                           InputOutput, visual->visual,
                           CWBackPixmap | CWBorderPixel | CWColormap,
                           &winAttrs);

    if (!window)
    {
        fprintf(stderr, "XCreateSimpleWindow failed\n");
        return EGL_FALSE;
    }

    /* Enable non-composited mode if possible */
    if (XVisualIDFromVisual(rootAttrs.visual) == visualId)
    {
        Atom nonCompWindow = XInternAtom(nativeDisplay, "_HILDON_NON_COMPOSITED_WINDOW", False);
        Atom windowType = XInternAtom(nativeDisplay, "_NET_WM_WINDOW_TYPE", False);
        Atom windowTypeOverride = XInternAtom(nativeDisplay, "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE", False);
        int one = 1;

        XChangeProperty(nativeDisplay, window, nonCompWindow, XA_INTEGER, 32, PropModeReplace, (unsigned char*)&one, 1);

        if (!runningOnFremantle())
        {
            XChangeProperty(nativeDisplay, window, windowType, XA_ATOM, 32, PropModeReplace, (unsigned char*)&windowTypeOverride, 1);
        }
    }
    else
    {
        printf("Warning: using a composited window\n");
    }

    windowTitle.value    = (unsigned char*)title;
    windowTitle.encoding = XA_STRING;
    windowTitle.format   = 8;
    windowTitle.nitems   = strlen(title);
    XSetWMName(nativeDisplay, window, &windowTitle);

    XMapWindow(nativeDisplay, window);
    XFlush(nativeDisplay);

    /* Set window to fullscreen mode if it matches the screen size */
    if (rootAttrs.width == width && rootAttrs.height == height)
    {
        XEvent xev;
        Atom wmState = XInternAtom(nativeDisplay, "_NET_WM_STATE", False);
        Atom wmStateFullscreen = XInternAtom(nativeDisplay, "_NET_WM_STATE_FULLSCREEN", False);

        memset(&xev, 0, sizeof(XEvent));
        xev.type = ClientMessage;
        xev.xclient.window = window;
        xev.xclient.message_type = wmState;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = 1;
        xev.xclient.data.l[1] = wmStateFullscreen;
        xev.xclient.data.l[2] = 0;
        XSendEvent(nativeDisplay, rootWindow, False, SubstructureNotifyMask, &xev);
    }

    *nativeWindow = window;
    
    return EGL_TRUE;
}

void nativeDestroyWindow(EGLNativeDisplayType nativeDisplay, EGLNativeWindowType nativeWindow)
{
    XDestroyWindow(nativeDisplay, nativeWindow);
}

EGLBoolean nativeCreatePixmap(EGLNativeDisplayType nativeDisplay, EGLDisplay dpy, EGLConfig config, 
                              int width, int height, EGLNativePixmapType *nativePixmap)
{
    Window rootWindow = DefaultRootWindow(nativeDisplay);
    Pixmap pixmap;
    int depth;

    if (eglGetConfigAttrib(dpy, config, EGL_BUFFER_SIZE, &depth) != EGL_TRUE)
    {
        fprintf(stderr, "eglGetConfigAttrib failed: %x\n", eglGetError());
        return EGL_FALSE;
    }

    pixmap = XCreatePixmap(nativeDisplay, rootWindow, width, height, depth);

    if (!pixmap)
    {
        fprintf(stderr, "XCreatePixmap failed\n");
        return EGL_FALSE;
    }

    XFlush(nativeDisplay);

    *nativePixmap = pixmap;

    return EGL_TRUE;
}

void nativeDestroyPixmap(EGLNativeDisplayType nativeDisplay, EGLNativePixmapType nativePixmap)
{
    XFreePixmap(nativeDisplay, nativePixmap);
}
